#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "serial.h"
#include "lib.h"
#include "consdrv.h"

#define CONS_BUFFER_SIZE 14
// コンソール管理用の構造体の定義
static struct consreg {
    kz_thread_id_t id;  // コンソールを利用するスレッド
    int index;          // 使用するシリアルの番号

    char *send_buf;     // 送信バッファ
    char *recv_buf;     // 受信バッファ
    int send_len;       // 送信バッファ中のデータサイズ
    int recv_len;       // 受信バッファ中のデータサイズ

    long dummy[3];
} consreg[CONSDRV_DEVICE_NUM];

// 
// 以下の2つの関数(send_char(), send_string())は割込み処理と
// スレッドから呼ばれるが送信バッファを操作しており再入不可のため、
// スレッドから呼び出す場合は排他のため割込み禁止状態で呼ぶこと。
// 

// 送信バッファの先頭一文字を送信する
static void send_char(struct consreg *cons) {
    int i;
    // 送信バッファの先頭1文字を出力し、その分バッファを詰める
    serial_send_byte(cons->index, cons->send_buf[0]);
    cons->send_len--;
    // 先頭文字を送信したので1文字分ずらす
    for (i = 0; i < cons->send_len; i++)
        cons->send_buf[i] = cons->send_buf[i + 1];
}

// 文字列を送信バッファに書き込み送信開始する
static void send_string(struct consreg *cons, char *str, int len) {
    int i;
    for (i = 0; i < len; i++) { // 文字列を送信バッファにコピー
        if (str[i] == '\n')
            cons->send_buf[cons->send_len++] = '\r';
        cons->send_buf[cons->send_len++] = str[i];
    }
    // 
    // 送信割込み無効ならば、送信開始されていないので送信開始する
    // 送信割込み有効ならば、送信開始されており、送信割込みの延長で
    // 送信バッファ内のデータが順次送信されるので、何もしなくてよい。
    // 
    if (cons->send_len && !serial_intr_is_send_enable(cons->index)) {
        serial_intr_send_enable(cons->index); // 送信割込み有効化
        send_char(cons);
    }
}

// 
// 以下は割込みハンドラから呼ばれる割込み処理であり、非同期で呼ばれるので、
// ライブラリ関数などを呼び出す場合には注意が必要。
// 基本として、以下のいずれかに当てはまる関数しか呼び出してはいけない。
// ・再入可能である
// ・スレッドから呼ばれることはない関数である
// また非コンテキスト状態で呼ばれるため、システム・コールは利用してはいけない。
// (サービス・コールを利用すること)
// 
static int consdrv_intrproc(struct consreg *cons) {
    unsigned char c;
    char *p;

    if (serial_is_recv_enable(cons->index)) { // 受信割込み
        c = serial_recv_byte(cons->index);
        if (c == '\r') // 改行コード変換 (\r→\n)
            c = '\n';
        
        send_string(cons, &c, 1); // エコーバック処理
        if (cons->id) {
            if (c != '\n') {
                // 改行でないなら、受信バッファにバッファリングする
                cons->recv_buf[cons->recv_len++] = c;
            } else {
                // 
                // Enter が押されたら、バッファの内容を
                // コマンド処理スレッドに通知する
                // (割込みハンドラなので、サービス・コールを利用する)
                // 
                p = kx_kmalloc(CONS_BUFFER_SIZE);
                memcpy(p, cons->recv_buf, cons->recv_len);
                kx_send(MSGBOX_ID_CONSINPUT, cons->recv_len, p);
                cons->recv_len = 0;
            }
        }
    }

    if (serial_is_send_enable(cons->index)) { // 送信割込み
        if (!cons->id || !cons->send_len) {
            // 送信データが無いならば送信処理終了
            serial_intr_send_disable(cons->index);
        } else {
            // 送信データがあるならば、引き続き送信する
            send_char(cons);
        }
    }

    return 0;
}

// 割込みハンドラ
static void consdrv_intr(void) {
    int i;
    struct consreg *cons;

    for (i = 0; i < CONSDRV_DEVICE_NUM; i++) {
        cons = &consreg[i];
        if (cons->id) {
            if (serial_is_send_enable(cons->index) ||
                serial_is_recv_enable(cons->index))
                // 割込みがあるならば、割込み処理を呼び出す
                consdrv_intrproc(cons);
        }
    }
}

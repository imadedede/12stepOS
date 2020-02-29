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

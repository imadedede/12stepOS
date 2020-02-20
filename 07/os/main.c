#include "defines.h"
#include "intr.h"
#include "interrupt.h"
#include "serial.h"
#include "lib.h"

// シリアル受信の割り込みハンドラ
static void intr(softvec_type_t type, unsigned long sp){
    int c;
    static char buf[32];    // 受信バッファ
    static int len;

    // 受信割込みが入ったのでコンソールから一文字受信する
    c = getc();

    if (c != '\n') {
        // 受信したのが改行でないなら受信バッファに保存する
        buf[len++] = c;
    } else {
        // 改行を受信した場合にはコマンド処理を行う
        buf[len++] = '\0';
        if (!strncmp(buf, "echo", 4)) {
            // echo コマンドの場合は後続の文字列を出力する
            puts(buf + 4);
            puts("\n");
        } else {
            // 不明なコマンドなら unknown と出力
            puts("unknown.\n");
        }
        puts("> ");
        len = 0;
    }
}

int main(void) {
    // 割込み無効の状態で初期化を行う
    INTR_DISABLE;    // 割込み無効にする

    // ブートメッセージを変更
    puts("kozos boot succeed!\n");

    // ソフトウェア割込みベクタにシリアル割込みのハンドラを設定
    softvec_setintr(SOFTVEC_TYPE_SERINTR, intr);
    // シリアル受信割込みを有効化
    serial_intr_recv_enable(SERIAL_DEFAULT_DEVICE);

    puts("> ");

    INTR_ENABLE;     // 割込み有効にする

    while (1) {
        asm volatile ("sleep"); // 省電力モードに移行 割込みベースで動作
    }

    return 0;
}

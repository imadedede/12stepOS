#include "defines.h"
#include "kozos.h"
#include "consdrv.h"
#include "lib.h"

// コンソール・ドライバの使用開始をコンソール・ドライバに依頼する
static void send_use(int index) {
    char *p;
    p = kz_kmalloc(3);
    p[0] = '0';             // コンソール番号 今回は1つだけなので0
    p[1] = CONSDRV_CMD_USE; // 初期化コマンドを設定
    p[2] = '0' + index;     // シリアルポート番号
    kz_send(MSGBOX_ID_CONSOUTPUT, 3, p); // コンソール・ドライバ・スレッドに送信
}

// コンソールへの文字列出力をコンソール・ドライバに依頼する
static void send_write(char *str) {
    char *p;
    int len;
    len = strlen(str);
    p = kz_kmalloc(len + 2);    // コマンド通知用の領域を獲得
    p[0] = '0';                 // コンソール番号 今回は1つだけなので0
    p[1] = CONSDRV_CMD_WRITE;   // 文字列出力コマンドを設定
    memcpy(&p[2], str, len);
    kz_send(MSGBOX_ID_CONSOUTPUT, len + 2, p); // コンソール・ドライバ・スレッドに送信
}

// コマンド・スレッドのメイン関数
int command_main(int argc, char *argv[]) {
    char *p;
    int size;

    // コンソール・ドライバ・スレッドにコンソールの初期化を依頼する
    send_use(SERIAL_DEFAULT_DEVICE);

    while (1) {
        send_write("command> "); // プロンプト表示

        // コンソールからの受信文字列を受け取る
        kz_recv(MSGBOX_ID_CONSINPUT, &size, &p);
        p[size] = '\0';

        if (!strncmp(p, "echo", 4)) { // echo コマンド
            send_write(p + 4); // echo に続く文字列を出力する
            send_write("\n");
        } else {
            send_write("unknown.\n");
        }
        // メッセージにより受信した領域を(送信元で獲得されたもの)開放する
        kz_kmfree(p);
    }

    return 0;
}

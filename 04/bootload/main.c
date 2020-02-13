#include "defines.h"
#include "serial.h"
#include "xmodem.h"
#include "lib.h"

static int init(void) {
    // 以下はリンカ・スクリプトで定義してあるシンボル
    // リンカ・スクリプトで定義されたシンボルを参照可能にする
    extern int data_start_load;
    extern int data_start;
    extern int edata;
    extern int bss_start;
    extern int ebss;

    // 静的領域(データ領域とBSS領域)を初期化する
    // この処理以降でないとグローバル変数が初期化されていないので注意
    memcpy(&data_start, &data_start_load, (long)&edata - (long)&data_start);
    memset(&bss_start, 0, (long)&ebss - (long)&bss_start);

    // シリアルの初期化
    serial_init(SERIAL_DEFAULT_DEVICE); // シリアルデバイスの初期化

    return 0;
}

// メモリの16進数ダンプ出力
static int dump(char *buf, long size) {
    long i;

    if (size < 0) {
        puts("no data.\n");
        return -1;
    }
    for (i = 0; i < size; i++) {
        putxval(buf[i], 2);
        if ((i & 0xf) == 15) {
            puts("\n");
        } else {
            if ((i & 0xf) == 7) puts(" ");
            puts(" ");
        }
    }
    puts("\n");

    return 0;
}

// ウェイト用のサービス関数を追加
static void wait(void) {
    volatile long i;
    for (i = 0; i < 300000; i++)
        ;
}

int main(void) {
    static char buf[16];
    static long size = -1;
    static unsigned char *loadbuf = NULL;
    extern int buffer_start;    // リンカスクリプトで定義されているバッファ

    init();

    puts("kzload (kozos boot loader) started.\n");

    // コンソールからのコマンドに応じて動作する
    while (1) {
        puts("kzload> ");   // プロンプト表示
        gets(buf);          // シリアルからのコマンド受信

        if (!strcmp(buf, "load")) { // XMODEM でのファイルのダウンロード
            loadbuf = (char *)(&buffer_start);
            size = xmodem_recv(loadbuf);
            wait();                 // 転送アプリが終了し端末アプリに制御が戻るまで待ち合わせる
            if (size < 0) {
                puts("\nXMODEM receive error!\n");
            } else {
                puts("\nXMODEM receive succeeded.!\n");
            }
        } else if (!strcmp(buf, "dump")) {  // メモリの16進数ダンプ表示
            puts("size: ");
            putxval(size, 0);
            puts("\n");
            dump(loadbuf, size);
        } else {
            puts("unknown.\n");
        }
    }

    return 0;
}

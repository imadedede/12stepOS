#include "defines.h"
#include "serial.h"
#include "lib.h"

static int init(void) {
    // 以下はリンカ・スクリプトで定義してあるシンボル
    // リンカ・スクリプトで定義されたシンボルを参照可能にする
    extern int erodata;
    extern int data_start;
    extern int edata;
    extern int bss_start;
    extern int ebss;

    // 静的領域(データ領域とBSS領域)を初期化する
    // この処理以降でないとグローバル変数が初期化されていないので注意
    memcpy(&data_start, &erodata, (long)&edata - (long)&data_start);
    memset(&bss_start, 0, (long)&ebss - (long)&bss_start);

    // シリアルの初期化
    serial_init(SERIAL_DEFAULT_DEVICE); // シリアルデバイスの初期化

    return 0;
}

// 静的変数操作のサンプル
int global_data = 0x10;         // .data セクションへ
int global_bss;                 // .bss セクションへ
static int static_data = 0x20;  // .data セクションへ
static int static_bss;          // .bss セクションへ

static void printval(void) {
    // 各変数の値を出力
    puts("global_data = "); putxval(global_data, 0); puts("\n");
    puts("global_bss  = "); putxval(global_bss,  0); puts("\n");
    puts("static_data = "); putxval(static_data, 0); puts("\n");
    puts("static_bss  = "); putxval(static_bss,  0); puts("\n");
}

int main(void) {
    init();     // 初期化関数の呼び出し
    puts("Hello World!\n");

    printval();
    puts("overwrite variables.\n");
    // 変数の値を書き換え、値を再度出力
    global_data = 0x20;
    global_bss  = 0x30;
    static_data = 0x40;
    static_bss  = 0x50;
    printval();

    while (1)
        ;
    
    return 0;
}

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

int main(void) {

    init();     // 初期化関数の呼び出し
    puts("Hello World!\n");

    while (1)
        ;
    
    return 0;
}

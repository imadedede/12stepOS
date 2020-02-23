#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "lib.h"

// システム・タスクとユーザ・スレッドの起動
static int start_threads(int argc, char *argv[]) {
    // コマンド処理スレッドを起動する
    kz_run(test08_1_main, "command", 0x100, 0, NULL);
    return 0;
}

int main(void) {
    // 割込み無効の状態で初期化を行う
    INTR_DISABLE;    // 割込み無効にする

    puts("kozos boot succeed!\n");

    // OS の動作開始
    // 初期スレッドとして start_threads() を起動し OS の動作を開始する
    kz_start(start_threads, "start", 0x100, 0, NULL);
    // ここには戻ってこない

    return 0;
}

#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "lib.h"

// システム・タスクとユーザ・タスクの起動
static int start_threads(int argc, char *argv[]) {
    kz_run(consdrv_main, "consdrv", 1, 0x200, 0, NULL); // コンソール・ドライバ・スレッドを起動
    kz_run(command_main, "command", 8, 0x200, 0, NULL); // コマンド・スレッドを起動

    kz_chpri(15); // 優先度を下げてアイドルスレッドに移行する
    INTR_ENABLE; // 割込み有効にする
    while(1) {
        asm volatile ("sleep"); // 省電力モードに移行
    }

    return 0;
}

int main(void) {
    // 割込み無効の状態で初期化を行う
    INTR_DISABLE;    // 割込み無効にする

    puts("kozos boot succeed!\n");

    // OS の動作開始
    // 初期スレッドを優先度ゼロで起動する
    kz_start(start_threads, "idle", 0, 0x100, 0, NULL);
    // ここには戻ってこない

    return 0;
}

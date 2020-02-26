#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "lib.h"

// サンプル・プログラムのスレッド ID
kz_thread_id_t test09_1_id;
kz_thread_id_t test09_2_id;
kz_thread_id_t test09_3_id;

// システム・タスクとユーザ・スレッドの起動
static int start_threads(int argc, char *argv[]) {
    test09_1_id = kz_run(test09_1_main, "test09_1", 1, 0x100, 0, NULL);
    test09_2_id = kz_run(test09_2_main, "test09_2", 2, 0x100, 0, NULL);
    test09_3_id = kz_run(test09_3_main, "test09_3", 3, 0x100, 0, NULL);

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

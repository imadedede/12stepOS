#include "defines.h"
#include "kozos.h"
#include "lib.h"

int test08_1_main(int argc, char *argv[]) {
    char buf[32];

    puts("test08_1 started.\n");

    // スレッドのメインループ
    while(1) {
        puts("> ");
        gets(buf);

        if (!strncmp(buf, "echo", 4)) {     // echo コマンドの場合
            puts(buf + 4);      // 後続の文字列を出力する
            puts("\n");
        } else if (!strcmp(buf, "exit")) {  // exit コマンドの場合
            break;              // メインループを終了する
        } else {
            puts("unknown.\n"); // 不明なコマンドなら unknown と出力
        }
    }
    
    puts("test08_1 exit.\n");

    return 0;
}

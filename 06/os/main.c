#include "defines.h"
#include "serial.h"
#include "lib.h"

int main(void) {
    static char buf[32];

    puts("Hello World!\n");

    // コンソールからのコマンドに応じて動作する
    while (1) {
        puts("> ");
        gets(buf);  // コンソールからの1行入力

        if (!strncmp(buf, "echo", 4)) {     // echo コマンドの処理
            puts(buf + 4);
            puts("\n");
        } else if (!strcmp(buf, "exit")) {  // exit コマンドの処理
            break;
        } else {
            puts("unknown.\n");
        }
    }

    return 0;
}

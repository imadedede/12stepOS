#include "defines.h"
#include "kozos.h"
#include "syscall.h"

// システム・コール
kz_thread_id_t kz_run(kz_func_t func, char *name, int stacksize,
                        int argc, char *argv[]) {
    // 引数として渡されたパラメータを設定する
    kz_syscall_param_t param;
    param.un.run.func = func;
    param.un.run.name = name;
    param.un.run.stacksize = stacksize;
    param.un.run.argc = argc;
    param.un.run.argv = argv;
    // システム・コールを呼び出す
    kz_syscall(KZ_SYSCALL_TYPE_RUN, &param);
    // システム・コールの応答が構造体に格納されて返るので
    // 構造体として返す
    return param.un.run.ret;
}

// kz_exit() の実体
void kz_exit(void) {
    kz_syscall(KZ_SYSCALL_TYPE_EXIT, NULL);
}

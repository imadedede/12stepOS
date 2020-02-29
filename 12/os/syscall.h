#ifndef _KOZOS_SYSCALL_H_INCLUDED_
#define _KOZOS_SYSCALL_H_INCLUDED_

#include "defines.h"
#include "interrupt.h"

// システム・コール番号の定義
typedef enum {
    KZ_SYSCALL_TYPE_RUN = 0,    // kz_run() のシステム・コール番号
    KZ_SYSCALL_TYPE_EXIT,       // kz_exit() のシステム・コール番号
    KZ_SYSCALL_TYPE_WAIT,       // kz_wait()
    KZ_SYSCALL_TYPE_SLEEP,      // kz_sleep()
    KZ_SYSCALL_TYPE_WAKEUP,     // kz_wakeup()
    KZ_SYSCALL_TYPE_GETID,      // kz_getid()
    KZ_SYSCALL_TYPE_CHPRI,      // kz_chpri()
    KZ_SYSCALL_TYPE_KMALLOC,    // kz_kmalloc()
    KZ_SYSCALL_TYPE_KMFREE,     // kz_kmfree()
    KZ_SYSCALL_TYPE_SEND,       // kz_send()
    KZ_SYSCALL_TYPE_RECV,       // kz_recv()
    KZ_SYSCALL_TYPE_SETINTR,    // kz_setintr()
} kz_syscall_type_t;

// システム・コール呼び出し時のパラメータ格納域の定義
typedef struct {
    union {
        struct {    // kz_run() のためのパラメータ
            kz_func_t func;     // メイン関数
            char *name;         // スレッド名
            int priority;       // 優先度
            int stacksize;      // スタックサイズ
            int argc;           // メイン関数に渡す引数の個数
            char **argv;        // メイン関数に渡す引数(argv 形式)
            kz_thread_id_t ret; // kz_run() の戻り値
        } run;
        struct {    // kz_exit() のためのパラメータ
            int dummy;          // パラメータ無しだが、空なのも良くないのでダミーのメンバ定義
        } exit;
        struct {
            int ret;
        } wait;
        struct {
            int ret;
        } sleep;
        struct {
            kz_thread_id_t id;
            int ret;
        } wakeup;
        struct {
            kz_thread_id_t ret;
        } getid;
        struct {
            int priority;
            int ret;
        } chpri;
        struct {
            int size;
            void *ret;
        } kmalloc;
        struct {
            char *p;
            int ret;
        } kmfree;
        struct {
            kz_msgbox_id_t id;
            int size;
            char *p;
            int ret;
        } send;
        struct {
            kz_msgbox_id_t id;
            int *sizep;
            char **pp;
            kz_thread_id_t ret;
        } recv;
        struct {
            softvec_type_t type;
            kz_handler_t handler;
            int ret;
        } setintr;
    } un;   // 複数のパラメータ領域を同時に利用することはないため、共用体で定義する
} kz_syscall_param_t;

#endif

#ifndef _KOZOS_H_INCLUDED_
#define _KOZOS_H_INCLUDED_

#include "defines.h"
#include "interrupt.h"
#include "syscall.h"

// システムコール
// スレッド起動のシステムコール
kz_thread_id_t kz_run(kz_func_t func, char *name, int priority, int stacksize,
                        int argc, char *argv[]);
void kz_exit(void);                 // スレッド終了のシステムコール
int kz_wait(void);                  // カレントスレッドの切り替え
int kz_sleep(void);                 // スレッドをレディキューから外す
int kz_wakeup(kz_thread_id_t id);   // スリープ状態のスレッドをレディキューにつなぐ
kz_thread_id_t kz_getid(void);      // 自身のスレッド ID を取得する
int kz_chpri(int priority);         // スレッドの優先度を変更する
void *kz_kmalloc(int size);         // 動的メモリ確保
int kz_kmfree(void *p);             // 動的メモリ開放
int kz_send(kz_msgbox_id_t id, int size, char *p);                  // タスク間メッセージ 送信
kz_thread_id_t kz_recv(kz_msgbox_id_t id, int *sizep, char **p);    // タスク間メッセージ 受信
int kz_setintr(softvec_type_t type, kz_handler_t handler);          // 割込みハンドラの登録

// サービスコール
int kx_wakeup(kz_thread_id_t id);   // スリープ状態のスレッドをレディキューにつなぐ
void *kx_kmalloc(int size);         // 動的メモリ確保
int kx_kmfree(void *p);             // 動的メモリ開放
int kx_send(kz_msgbox_id_t id, int size, char *p);                  // タスク間メッセージ 送信

// ライブラリ関数
// 初期スレッドを起動し OS の動作を開始する
void kz_start(kz_func_t func, char *name, int priority, int stacksize,
                int argc, char *argv[]);
void kz_sysdown(void);  // 致命的エラーのときに呼び出す
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param); // システムコールを実行する
void kz_srvcall(kz_syscall_type_t type, kz_syscall_param_t *param); // サービスコールを実行する

// システム・タスク
int consdrv_main(int argc, char *argv[]);   // コンソール・ドライバ・スレッド

// ユーザ・タスク
int command_main(int argc, char* argv[]);   // コマンド・スレッド

#endif

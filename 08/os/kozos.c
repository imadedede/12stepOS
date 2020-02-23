#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "syscall.h"
#include "lib.h"

#define THREAD_NUM 6        // TCB の個数
#define THREAD_NAME_SIZE 15 // スレッド名の最大長

// スレッドコンテキスト
// スレッドのコンテキスト保存用の構造体の定義
typedef struct _kz_context {
    uint32 sp;  // スタックポインタ
                // 汎用レジスタはスタックに保存されるので、
                // TCB にコンテキストとして保存するのはスタックポインタのみ
} kz_context;

// タスク・コントロール・ブロック (TCB) 定義
typedef struct _kz_thread {
    struct _kz_thread *next;    // レディ・キューへの接続に利用する next ポインタ
    char name[THREAD_NAME_SIZE + 1];    // スレッド名
    char *stack;                        // スレッドのスタック

    struct {    // スレッドのスタートアップ thread_init() に渡すパラメータ
        kz_func_t func; // スレッドのメイン関数
        int argc;       // スレッドのメイン関数に渡す argc
        char **argv;    // スレッドのメイン関数に渡す
    } init;

    // システムコールの発行時に利用するパラメータ領域
    struct {    // システムコール用バッファ
        kz_syscall_type_t type;
        kz_syscall_param_t *param;
    } syscall;

    kz_context context; // スレッドのコンテキスト情報の保存領域
} kz_thread;

// スレッドのレディーキュー
static struct {
    kz_thread *head;    // レディ・キューの先頭のエントリ
    kz_thread *tail;    // レディ・キューの末尾のエントリ
} readyque;

static kz_thread *current;                      // カレント・スレッド
static kz_thread threads[THREAD_NUM];           // タスク・コントロール・ブロック
static kz_handler_t handlers[SOFTVEC_TYPE_NUM]; // 割込みハンドラ

// スレッドのディスパッチ用関数 (実体は startup.s)
void dispatch(kz_context *context);

// カレント・スレッドをレディ・キューから抜き出す
static int getcurrent(void) {
    // レディ・キューからカレント・スレッドを抜き出す
    if (current == NULL) {
        return -1;
    }

    // カレントスレッドは必ず先頭にあるはずなので、先頭から抜き出す
    readyque.head = current->next;
    if (readyque.head == NULL) {
        readyque.tail = NULL;
    }
    // エントリをリンクリストから抜き出したので next ポインタをクリアしておく
    current->next = NULL;

    return 0;
}

// カレント・スレッドをレディ・キューにつなげる
static int putcurrent(void) {
    // レディ・キューの末尾にカレント・スレッドをつなげる
    if (current == NULL) {
        return -1;
    }

    // カレント・スレッドをレディ・キューの末尾に接続する
    if (readyque.tail) {
        readyque.tail->next = current;
    } else {
        readyque.head = current;
    }
    readyque.tail = current;

    return 0;
}

// スレッドの終了
static void thread_end(void) {
    kz_exit();
}

// スレッドのスタートアップ
static void thread_init(kz_thread *thp) {
    // スレッドのメイン関数を呼び出す
    thp->init.func(thp->init.argc, thp->init.argv);
    thread_end();   // メイン関数から戻ったらスレッドを終了する
}

// システムコールの処理 kz_run():スレッドの起動
static kz_thread_id_t thread_run(kz_func_t func, char *name,
                                    int stacksize, int argc, char* argv[]) {
    int i;
    kz_thread *thp;
    uint32 *sp;
    extern char userstack; // リンカスクリプトで定義されているスタック領域
    static char *thread_stack = &userstack; // ユーザスタックに利用される領域

    // 空いているタスク・コントロール・ブロックを検索
    for (i = 0; i < THREAD_NUM; i++) {
        thp = &threads[i];
        if (!thp->init.func)    // 見つかった
            break;
    }
    if (i == THREAD_NUM)    // 見つからなかった
        return -1;
    
    memset(thp, 0, sizeof(*thp));   // TCB をゼロクリアする

    // タスク・コントロール・ブロック (TBC) の設定　各種パラメータを設定
    strcpy(thp->name, name);
    thp->next = NULL;
    thp->init.func = func;
    thp->init.argc = argc;
    thp->init.argv = argv;

    // スタック領域を獲得して TCB に設定
    memset(thread_stack, 0, stacksize);
    thread_stack += stacksize;
    thp->stack = thread_stack;  // スタックを設定

    // スタックの初期化
    // スタックに thread_init() からの戻り先として thread_end() を設定する
    sp = (uint32 *)thp->stack;
    *(--sp) = (uint32)thread_end;

    //
    // プログラム・カウンタを設定する
    //
    // ディスパッチ時にプログラムカウンタに格納される値としてthread_init() を設定する。
    // よってスレッドは thread_init() から動作を開始する
    *(--sp) = (uint32)thread_init;

    *(--sp) = 0; // ER6
    *(--sp) = 0; // ER5
    *(--sp) = 0; // ER4
    *(--sp) = 0; // ER3
    *(--sp) = 0; // ER2
    *(--sp) = 0; // ER1

    // スレッドのスタートアップ thread_init() に渡す引数
    *(--sp) = (uint32)thp;  // ER0 第一引数

    // スレッドのコンテキストを設定
    thp->context.sp = (uint32)sp;   // コンテキストとしてスタックポインタを保存

    // システムコールを呼び出したスレッドをレディ・キューに戻す
    putcurrent();

    // 新規作成したスレッドをレディ・キューに接続する
    current = thp;
    putcurrent();

    // 新規作成したスレッドのスレッド ID を戻り地として返す
    return (kz_thread_id_t)current;
}

// システム・コールの処理 kz_exit():スレッドの終了
static int thread_exit(void) {
    //
    // 本来ならスタックも開放して再利用できるようにすべきだが省略
    // このため、スレッドを頻繁に生成・消去するようなことは現状でできない
    //
    // 終了時のメッセージとして <スレッド名> EXIT. を出力
    puts(current->name);
    puts(" EXIT.\n");
    memset(current, 0, sizeof(*current)); // TCB をクリアする
    return 0;
}

// 割込みハンドラの登録
static void thread_intr(softvec_type_t type, unsigned long sp);
static int setintr(softvec_type_t type, kz_handler_t handler) {

    //
    // 割込みを受け付けるために、ソフトウェア割込みベクタに
    // OSの割込み処理の入口となる関数を登録する
    //
    softvec_setintr(type, thread_intr);

    handlers[type] = handler;   // OS 側から呼び出す割込みハンドラを登録

    return 0;
}

// システム・コールの処理関数の呼び出し
static void call_functions(kz_syscall_type_t type, kz_syscall_param_t * p) {
    // システム・コールの実行中に current が書き換わるので注意
    switch(type) {
    case KZ_SYSCALL_TYPE_RUN:   // kz_run()
        p->un.run.ret = thread_run(p->un.run.func, p->un.run.name,
                                    p->un.run.stacksize,
                                    p->un.run.argc, p->un.run.argv);
        break;
    case KZ_SYSCALL_TYPE_EXIT:  // kz_exit()
        // TCB が削除されるので、戻り値を書き込んではいけない
        thread_exit();
        break;
    default:
        break;
    }
}

// システム・コールの処理
static void syscall_proc(kz_syscall_type_t type, kz_syscall_param_t * p) {
    //
    // システム・コールを呼び出したスレッドをレディ・キューから
    // 外した状態で処理関数を呼び出す。このためシステム・コールを
    // 呼び出したスレッドをそのまま動作継続させたい場合には、
    // 処理関数の内部で putcurrent() を行う必要がある
    //
    getcurrent();
    call_functions(type, p);
}

// スレッドのスケジューリング
static void schedule(void) {
    if (!readyque.head) // 見つからなかった
        kz_sysdown();
    current = readyque.head;    // カレントスレッドに設定する
}

// システム・コールの呼び出し
static void syscall_intr(void) {
    syscall_proc(current->syscall.type, current->syscall.param);
}

// ソフトウェア・エラーの発生
static void softerr_intr(void) {
    // ソフトウェア・エラーが発生した場合には、スレッドを強制終了する
    puts(current->name);
    puts(" DOWN.\n");
    getcurrent();
    thread_exit();
}

// 割込み処理の入口関数
static void thread_intr(softvec_type_t type, unsigned long sp) {
    // カレント・スレッドのコンテキストを保存する
    current->context.sp = sp;

    // 
    // 割込みごとの処理を実行する。
    // SOFTVEC_TYPE_SYSCALL, SOFTVEC_TYPE_SOFTERR の場合は
    // syscall_intr(), softerr_intr() がハンドラに登録されているので
    // それらが実行される
    // 
    if (handlers[type])
        handlers[type]();   // 割込みに対応した各ハンドラを実行する

    schedule(); // スレッドのスケジューリング

    // 
    // スレッドのディスパッチ
    // (dispatch() 関数の本体は startup.s にあり、アセンブラで記述されている)
    // 
    dispatch(&current->context);
    // ここには返ってこない
}

void kz_start(kz_func_t func, char *name, int stacksize,
                int argc, char *argv[]) {
    // 
    // 以降で呼び出すスレッド間のライブラリ関数の内部で current を
    // 見ている場合があるので、current を NULL に初期化しておく
    //
    current = NULL;

    // 各種データの初期化
    readyque.head = readyque.tail = NULL;
    memset(threads,  0, sizeof(threads));
    memset(handlers, 0, sizeof(handlers));

    // 割込みハンドラの登録
    setintr(SOFTVEC_TYPE_SYSCALL, syscall_intr);    // システム・コール
    setintr(SOFTVEC_TYPE_SOFTERR, softerr_intr);    // ダウン要因

    // システム・コール発行不可なので直接関数を呼び出してスレッド作成する
    current = (kz_thread *)thread_run(func, name, stacksize, argc, argv);

    // 最初のスレッドを起動
    dispatch(&current->context);

    // ここには返ってこない
}

// OS 内部で致命的エラーが発生した場合にはこの関数を呼ぶ
void kz_sysdown(void) {
    // エラーメッセージを出力
    puts("system error!\n");
    // 無限ループに入って停止する
    while(1)
        ;
}

// システム・コール呼び出し用ライブラリ関数
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param) {
    current->syscall.type  = type;  // システム・コール番号の設定
    current->syscall.param = param; // パラメータの設定
    asm volatile ("trapa #0");  // トラップ割込み発行
}

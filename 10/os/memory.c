#include "defines.h"
#include "kozos.h"
#include "lib.h"
#include "memory.h"

// 
// メモリ・ブロック構造体
// (獲得された各領域は、先頭に以下の構造体を持っている)
// 
typedef struct _kzmem_block {
    struct _kzmem_block *next;
    int size;
}kzmem_block;

// メモリ・プール ブロック・サイズごとに確保される
typedef struct _kzmem_pool {
    int size;
    int num;
    kzmem_block *free;
} kzmem_pool;

// メモリプールの定義(個々のサイズと個数)
static kzmem_pool pool[] = {
    { 16, 8, NULL }, { 32, 8, NULL }, { 64, 4, NULL },
};

// メモリプールの種類の個数
#define MEMORY_AREA_NUM (sizeof(pool) / sizeof(*pool))

// メモリ・プールの初期化
static int kzmem_init_pool(kzmem_pool *p) {
    int i;
    kzmem_block *mp;
    kzmem_block **mpp;
    extern char freearea; // リンカ・スクリプトで定義される空き領域
    static char *area = &freearea;

    mp = (kzmem_block *)area;

    // 個々の領域をすべて解放済みリンクリストにつなぐ
    // 動的メモリ用の領域から必要個数だけブロックを切り出し、解放済みリンクリストにつなぐ
    mpp = &p->free;
    for (i = 0; i < p->num; i++) {
        *mpp = mp;
        memset(mp, 0, sizeof(*mp));
        mp->size = p->size;
        mpp = &(mp->next);
        mp = (kzmem_block *)((char *)mp + p->size);
        area += p->size;
    }

    return 0;
}

// 動的メモリの初期化
int kzmem_init(void) {
    int i;
    for (i = 0; i < MEMORY_AREA_NUM; i++) {
        kzmem_init_pool(&pool[i]); // 各メモリ・プールを初期化する
    }
    return 0;
}

// 動的メモリの獲得
void *kzmem_alloc(int size) {
    int i;
    kzmem_block *mp;
    kzmem_pool *p;

    for (i = 0; i < MEMORY_AREA_NUM; i++) {
        p = &pool[i];
        if (size <= p->size - sizeof(kzmem_block)) { // 要求されたサイズが収まるか？
            if (p->free == NULL) { // 解放済み領域がない(メモリ・ブロック不足)
                kz_sysdown();
                return NULL;
            }
            // 解放済みリンクリストから領域を取得する
            mp = p->free;
            p->free = p->free->next;
            mp->next = NULL;

            //
            // 実際に利用可能な領域は、メモリ・ブロック構造体の
            // 直後の領域になるので、直後のアドレスを返す
            //
            return mp + 1;
        }
    }

    // 指定されたサイズの領域を格納できるメモリプールがない
    kz_sysdown();
    return NULL;
}

// メモリの開放
void kzmem_free(void *mem) {
    int i;
    kzmem_block *mp;
    kzmem_pool *p;

    // 領域の直前にある(はずの)メモリ・ブロック構造体を取得
    mp = ((kzmem_block *)mem - 1);

    for (i = 0; i < MEMORY_AREA_NUM; i++) {
        p = &pool[i];
        if (mp->size == p->size) { // 同一サイズのメモリ・プールを検索
            // 領域を解放済みリンクリストに戻す
            mp->next = p->free;
            p->free = mp;
            return;
        }
    }

    kz_sysdown();
}

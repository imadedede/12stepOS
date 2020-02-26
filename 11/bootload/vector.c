#include "defines.h"

extern void start(void);        // スタート・アップ
extern void intr_softerr(void); // ソフトウェアエラー
extern void intr_syscall(void); // システムコール
extern void intr_serintr(void); // シリアル割込み

/* 
 * 割り込みベクタの設定
 * リンカ・スクリプトの定義により、先頭番地に配置される
 */
void (*vectors[])(void) = {
    start, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    intr_syscall, intr_softerr, intr_softerr, intr_softerr,     // トラップ割込みのベクタ
                                                                // 先頭はシステムコールで利用する
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,     // SCI0 の割込みベクタ
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,     // SCI1 の割込みベクタ
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,     // SCI2 の割込みベクタ
};
#include "defines.h"
#include "intr.h"
#include "interrupt.h"

// ソフトウェア割込みベクタの初期化
int softvec_init(void) {
    int type;
    // すべてのソフトウェア割込みベクタを NULL に設定する
    for (type = 0; type < SOFTVEC_TYPE_NUM; type++)
        softvec_setintr(type, NULL);
    return 0;
}

// ソフトウェア割込みベクタの設定
int softvec_setintr(softvec_type_t type, softvec_handler_t handler) {
    SOFTVECS[type] = handler;
    return 0;
}

// 共通割込みハンドラ
// ソフトウェア割込みベクタを見て、各ハンドラに分岐する
void interrupt(softvec_type_t type, unsigned long sp) {
    softvec_handler_t handler = SOFTVECS[type];
    // ソフトウェア割込みベクタが設定されているならば、ハンドラを呼び出す
    if (handler)
        handler(type, sp);
}

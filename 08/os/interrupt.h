#ifndef _INTERRUPT_H_INCLUDED_
#define _INTERRUPT_H_INCLUDED_

// 以下はリンカスクリプトで定義してあるシンボル
extern char softvec;
#define SOFTVEC_ADDR (&softvec)

// ソフトウェア割込みベクタの種別を表す型の定義
typedef short softvec_type_t;

// 割り込みハンドラの型の定義
typedef void (*softvec_handler_t)(softvec_type_t type, unsigned long sp);

// ソフトウェア割込みベクタの位置
#define SOFTVECS ((softvec_handler_t *)SOFTVEC_ADDR)

#define INTR_ENABLE  asm volatile ("andc.b #0x3f,ccr")  // 割込み有効化
#define INTR_DISABLE asm volatile ("orc.b #0xc0,ccr")   // 割込み無効化(割込み禁止)

// ソフトウェア割込みベクタの初期化
int softvec_init(void);

// ソフトウェア割込みベクタの設定
int softvec_setintr(softvec_type_t type, softvec_handler_t handler);

#endif

#include "intr.h"
		.h8300h
		.section .text

		.global _intr_softerr
		.type	_intr_softerr,@function
# ソフトウェア・エラー
_intr_softerr:
		# 汎用レジスタの値をスタックに保存する
		mov.l	er6,@-er7
		mov.l	er5,@-er7
		mov.l	er4,@-er7
		mov.l	er3,@-er7
		mov.l	er2,@-er7
		mov.l	er1,@-er7
		mov.l	er0,@-er7
		# interrupt() の呼び出し
		mov.l	er7,er1						# 第二引数にスタックポインタを設定
		mov.w	#SOFTVEC_TYPE_SOFTERR,r0	# 第一引数に intr.h の SOFTVEC_TYPE_SOFTERR を設定
		jsr		@_interrupt					# 呼び出し
		# スタックから汎用レジスタの値を復旧する
		mov.l	@er7+,er0
		mov.l	@er7+,er1
		mov.l	@er7+,er2
		mov.l	@er7+,er3
		mov.l	@er7+,er4
		mov.l	@er7+,er5
		mov.l	@er7+,er6
		# 割込み復帰命令の実行
		rte

		.global _intr_syscall
		.type	_intr_syscall,@function
# システムコール
_intr_syscall:
		# 汎用レジスタの値をスタックに保存する
		mov.l	er6,@-er7
		mov.l	er5,@-er7
		mov.l	er4,@-er7
		mov.l	er3,@-er7
		mov.l	er2,@-er7
		mov.l	er1,@-er7
		mov.l	er0,@-er7
		# interrupt() の呼び出し
		mov.l	er7,er1						# 第二引数にスタックポインタを設定
		mov.w	#SOFTVEC_TYPE_SYSCALL,r0	# 第一引数に intr.h の SOFTVEC_TYPE_SYSCALL を設定
		jsr		@_interrupt					# 呼び出し
		# スタックから汎用レジスタの値を復旧する
		mov.l	@er7+,er0
		mov.l	@er7+,er1
		mov.l	@er7+,er2
		mov.l	@er7+,er3
		mov.l	@er7+,er4
		mov.l	@er7+,er5
		mov.l	@er7+,er6
		# 割込み復帰命令の実行
		rte

		.global _intr_serintr
		.type	_intr_serintr,@function
# シリアル割込み
_intr_serintr:
		# 汎用レジスタの値をスタックに保存する
		mov.l	er6,@-er7
		mov.l	er5,@-er7
		mov.l	er4,@-er7
		mov.l	er3,@-er7
		mov.l	er2,@-er7
		mov.l	er1,@-er7
		mov.l	er0,@-er7
		# interrupt() の呼び出し
		mov.l	er7,er1						# 第二引数にスタックポインタを設定
		mov.w	#SOFTVEC_TYPE_SERINTR,r0	# 第一引数に intr.h の SOFTVEC_TYPE_SERINTR を設定
		jsr		@_interrupt					# 呼び出し
		# スタックから汎用レジスタの値を復旧する
		mov.l	@er7+,er0
		mov.l	@er7+,er1
		mov.l	@er7+,er2
		mov.l	@er7+,er3
		mov.l	@er7+,er4
		mov.l	@er7+,er5
		mov.l	@er7+,er6
		# 割込み復帰命令の実行
		rte

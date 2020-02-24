	.h8300h
	.section	.text
	.global		_start
	.type		_start,@function
_start:
	mov.l		#_bootstack,sp
	jsr			@_main

1:
	bra			1b

# スレッドのディスパッチ処理
	.global 	_dispatch
	.type		_dispatch,@function
_dispatch:
	# 引数としてスレッドのスタックポインタが渡される
	mov.l		@er0,er7
	# スレッドのスタックから汎用レジスタの値を復旧する
	mov.l		@er7+,er0
	mov.l		@er7+,er1
	mov.l		@er7+,er2
	mov.l		@er7+,er3
	mov.l		@er7+,er4
	mov.l		@er7+,er5
	mov.l		@er7+,er6
	# 割込み復帰命令でコンテキスト切替えが行われる
	rte

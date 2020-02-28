PREFIX  = /usr/local
ARCH    = h8300-elf
BINDIR  = $(PREFIX)/bin
ADDNAME = $(ARCH)-

# 利用するツール類の指定
AR      = $(BINDIR)/$(ADDNAME)ar
AS      = $(BINDIR)/$(ADDNAME)as
CC      = $(BINDIR)/$(ADDNAME)gcc
LD      = $(BINDIR)/$(ADDNAME)ld
NM      = $(BINDIR)/$(ADDNAME)nm
OBJCOPY = $(BINDIR)/$(ADDNAME)objcopy
OBJDUMP = $(BINDIR)/$(ADDNAME)objdump
RANLIB  = $(BINDIR)/$(ADDNAME)ranlib
STRIP   = $(BINDIR)/$(ADDNAME)strip

# コンパイルするソースコード群
OBJS  = startup.o main.o interrupt.o
OBJS += lib.o serial.o

# OS のソースコードをコンパイル対象に追加
OBJS += kozos.o syscall.o memory.o test11_1.o test11_2.o

# 生成する実行形式のファイル名
TARGET = kozos

# コンパイルオプション
CFLAGS  = -Wall -mh -nostdinc -nostdlib -fno-builtin
#CFLAGS += -mint32 # int を32ビットにすると掛け算／割り算ができなくなる
CFLAGS += -I.
#CFLAGS += -g
CFLAGS += -Os
CFLAGS += -DKOZOS

# リンクオプション
LFLAGS = -static -T ld.scr -L.

.SUFFIXES: .c .o
.SUFFIXES: .s .o
.SUFFIXES: .S .o

all:			$(TARGET)
# 実行形式の生成ルール
$(TARGET):		$(OBJS)
				$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LFLAGS)
				cp $(TARGET) $(TARGET).elf
				$(STRIP) $(TARGET)

# *.c ファイルのコンパイルルール
.c.o:			$<
				$(CC) -c $(CFLAGS) $<

# アセンブラファイルのアセンブルルール
.s.o:			$<
				$(CC) -c $(CFLAGS) $<

.S.o:			$<
				$(CC) -c $(CFLAGS) $<

# フォルダの掃除
clean:
				rm -f $(OBJS) $(TARGET) $(TARGET).elf

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

# フラッシュ ROM 書き込みツール
H8WRITE = ../../tools/h8write/h8write

# シリアル接続先
# FreeBSD-4.x:/dev/cuaaX, FreeBSD-6.x:/dev/cuadX, FreeBSD(USB):/dev/cuaUx
# Linux:/dev/ttySx, Linux(USB):/dev/ttyUSBx, Windows:comX
H8WRITE_SERDEV = /dev/ttyUSB0

# コンパイルするソースコード群
OBJS  = vector.o startup.o intr.o main.o interrupt.o
OBJS += lib.o serial.o xmodem.o elf.o

# 生成する実行形式のファイル名
TARGET = kzload

# コンパイルオプション
CFLAGS = -Wall -mh -nostdinc -nostdlib -fno-builtin
#CFLAGS += -mint32 # int を32ビットにすると掛け算／割り算ができなくなる
CFLAGS += -I.
#CFLAGS += -g
CFLAGS += -Os
CFLAGS += -DKZLOAD

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

# モトローラSレコード・フォーマットへの変換
$(TARGET).mot:	$(TARGET)
				$(OBJCOPY) -O srec $(TARGET) $(TARGET).mot

image:			$(TARGET).mot

# フラッシュ ROM への転送
write:			$(TARGET).mot
				$(H8WRITE) -3069 -f20 $(TARGET).mot $(H8WRITE_SERDEV)

# フォルダの掃除
clean:
				rm -f $(OBJS) $(TARGET) $(TARGET).elf $(TARGET).mot
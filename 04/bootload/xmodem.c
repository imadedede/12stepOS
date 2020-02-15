#include "defines.h"
#include "serial.h"
#include "lib.h"
#include "xmodem.h"

// 制御コードの定義
#define XMODEM_SOH 0x01
#define XMODEM_STX 0x02
#define XMODEM_EOT 0x04
#define XMODEM_ACK 0x06
#define XMODEM_NAK 0x15
#define XMODEM_CAN 0x18
#define XMODEM_EOF 0x1a // Ctrl-Z

#define XMODEM_BLOCK_SIZE 128

// 受信開始されるまで送信要求を出す
static int xmodem_wait(void) {
    long cnt = 0;

    // 受信開始するまで NAK を定期的に送信する
    while (!serial_is_recv_enable(SERIAL_DEFAULT_DEVICE)) {
        if (++cnt >= 2000000) { // だいたい8秒間隔
            cnt = 0;
            serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_NAK);
        }
    }
    return 0;
}

// ブロック単位での受信
static int xmodem_read_block(unsigned char block_number, char *buf) {
    unsigned char c;
    unsigned char block_num;
    unsigned char check_sum;
    int i;

    // ブロック番号の受信
    block_num = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
    if (block_num != block_number)
        return -1;
    
    // 反転したブロック番号の受信
    block_num ^= serial_recv_byte(SERIAL_DEFAULT_DEVICE);
    if (block_num != 0xff)
        return -1;

    // 128バイトのデータを受信する
    check_sum = 0;
    for (i = 0; i < XMODEM_BLOCK_SIZE; i++) {
        c = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
        *(buf++) = c;
        check_sum += c;
    }

    // チェックサムの受信
    check_sum ^= serial_recv_byte(SERIAL_DEFAULT_DEVICE);
    if (check_sum)
        return -1;
    
    return i;
}

// XMODEM によるファイルの受信
long xmodem_recv(char *buf) {
    int r;
    int receiving = 0;
    long size = 0;
    unsigned char c;
    unsigned char block_number = 1;

    while (1) {
        if (!receiving)
            // 定期的に NAK を送信する
            xmodem_wait();  // 受信開始されるまで送信要求を出す

        c = serial_recv_byte(SERIAL_DEFAULT_DEVICE); // 1文字の受信

        if (c == XMODEM_EOT) {          // 受信終了
            // EOT を受信したら終了する
            serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_ACK);
            break;
        } else if (c == XMODEM_CAN) {   // 受信中断
            // CAN を受信したら中断する
            return -1;
        } else if (c == XMODEM_SOH) {   // 受信開始
            // SOH を受信したらデータ受信を開始する
            receiving++;
            r = xmodem_read_block(block_number, buf);   // ブロック単位での受信
            if (r < 0) {    // 受信エラー
                // エラー時には NAC を返す
                serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_NAK);
            } else {        // 正常受信
                // 正常時にはバッファのポインタを進め ACK を返す
                block_number++;
                size += r;
                buf  += r;
                serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_ACK);
            }
        } else {
            if (receiving)
                return -1;
        }
    }

    return size;
}


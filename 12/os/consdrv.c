#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "serial.h"
#include "lib.h"
#include "consdrv.h"

#define CONS_BUFFER_SIZE 14
// コンソール管理用の構造体の定義
static struct consreg {
    kz_thread_id_t id;  // コンソールを利用するスレッド
    int index;          // 使用するシリアルの番号

    char *send_buf;     // 送信バッファ
    char *recv_buf;     // 受信バッファ
    int send_len;       // 送信バッファ中のデータサイズ
    int recv_len;       // 受信バッファ中のデータサイズ

    long dummy[3];
} consreg[CONSDRV_DEVICE_NUM];

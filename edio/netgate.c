
#include "appmain.h"

#define CMD_TOUT        500

typedef enum {
    CMD_TST = 0xA0,
    CMD_TCP_OPEN = 0xA1,
    CMD_TCP_CLOSE = 0xA2,
    CMD_TCP_CLALL = 0xA3,
    CMD_TCP_WR = 0xA4,
    CMD_TCP_RD = 0xA5,
    CMD_TCP_RDA = 0xA6,
    CMD_TCP_AVB = 0xA7,
} NgCmd;

#define RSP_OK          0xB0
#define RSP_ERR         0xB1

#define ERR_TOUT        0x01
#define ERR_CRESP       0x02

u8 ngRxData(void *dst, u32 len) {

    u32 t = get_ticks_ms();

    while (len) {

        u32 rd = ed_fifo_rda(dst, len);

        u32 ct = get_ticks_ms();

        if (rd == 0) {

            if (ct - t > CMD_TOUT) {
                return ERR_TOUT;
            }

        } else {
            t = ct;
            dst += rd;
            len -= rd;
        }
    }

    return 0;
}

u8 ngRxAck() {

    u8 resp;
    u8 cresp;

    resp = ngRxData(&cresp, 1);
    if (resp)return resp;
    if (cresp != RSP_OK) {
        return ERR_CRESP;
    }
    return 0;
}

void ngStrTx(u8 *str) {

    u16 str_len = 0;
    while (str[str_len] != 0) {
        str_len++;
    }
    ed_usb_wr(&str_len, 2);
    ed_usb_wr(str, str_len);
}

void ngCmdTx(NgCmd cmd) {

    u8 buff[4];

    buff[0] = '-';
    buff[1] = '-' ^ 0xff;
    buff[2] = cmd;
    buff[3] = cmd ^ 0xff;

    ed_usb_wr(buff, sizeof (buff));
}

u8 ngTest() {

    ngCmdTx(CMD_TST);
    return ngRxAck();
}

u8 ngOpen(u8 *host, u8 *con_id) {

    u8 resp;

    ngCmdTx(CMD_TCP_OPEN);
    ngStrTx(host);

    resp = ngRxAck();
    if (resp)return resp;

    resp = ngRxData(con_id, 1);
    if (resp)return resp;

    return 0;
}

u8 ngReadAvb(u8 con_id, u8 *dst, u16 len, u16 *r) {

    u8 resp;

    //Read avaialble data
    //Do not read more than 2048 bytes at once (FIFO limit).
    ngCmdTx(CMD_TCP_RDA);
    ed_usb_wr(&con_id, 1);
    ed_usb_wr(&len, 2);

    resp = ngRxData(r, 2);
    if (resp)return resp;

    if (*r == 0) {
        return 0;
    }

    resp = ngRxData(dst, *r);
    if (resp)return resp;

    return 0;
}

u8 ngWrite(u8 con_id, u8 *src, u16 len) {

    ngCmdTx(CMD_TCP_WR);
    ed_usb_wr(&con_id, 1);
    ed_usb_wr(&len, 2);
    ed_usb_wr(src, len);
    
    return 0;
}

void ngCloseAll() {
    ngCmdTx(CMD_TCP_CLALL);

}
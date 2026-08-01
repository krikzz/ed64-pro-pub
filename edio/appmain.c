
#include "appmain.h"

u8 *tst_file_rd = (u8 *) "ed64/sysdata/config.ini";
u8 *tst_file_wr = (u8 *) "ed64/sysdata/tst-file.dat";

int printf_stub(char *buf, unsigned int len) {
    //this stub here because i don't know how to return default stderr_write
    return len;
}

void waitAB() {

    printf("press [A] or [B] key to continue\n");
    while (1) {
        joypad_poll();
        joypad_buttons_t joy = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        console_render();
        if (joy.a || joy.b) {
            break;
        }
    }
}

void printtError(u8 resp) {

    //console_clear();
    printf("\n\n");
    printf("error: 0x%02X\n", resp);
    console_render();
    waitAB();
}

u8 fileToRam(void) {

    u8 resp;
    console_clear();
    printf("\n\n");
    printf("read file to ram: [%s]...\n", tst_file_rd);
    console_render();
    u8 buff[1024];

    resp = ed_fs_file_open(tst_file_rd, FA_READ);
    if (resp)return resp;

    u32 size = min_ed(ed_fs_file_available(), sizeof (buff) - 1);
    buff[size] = 0;

    resp = ed_fs_file_read(buff, size);
    if (resp)return resp;

    printf("ok.file content: \n\n%s\n", buff);
    waitAB();

    return 0;
}

u8 fileToRom(void) {

    u8 resp;
    u32 offset = 0x100000;
    console_clear();
    printf("\n\n");
    printf("read file to rom: [%s]...\n", tst_file_rd);
    console_render();
    u8 buff[1024];

    resp = ed_fs_file_open(tst_file_rd, FA_READ);
    if (resp)return resp;

    u32 size = min_ed(ed_fs_file_available(), sizeof (buff) - 1);
    buff[size] = 0;

    resp = ed_fs_file_read_fci(ADDR_FCI_ROM + offset, size);
    if (resp)return resp;
    pi_rd(buff, ADDR_PI_ROM + offset, size);

    printf("ok.file content: \n\n%s\n", buff);
    waitAB();

    return 0;
}

u8 ramToFile(void) {

    u8 resp;

    console_clear();
    printf("\n\n");
    printf("write file from ram: [%s]...\n", tst_file_wr);
    console_render();
    u8 buff[4096];

    pi_rd(buff, ADDR_PI_ROM, sizeof (buff));

    resp = ed_fs_file_open(tst_file_wr, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (resp)return resp;

    resp = ed_fs_file_write(buff, sizeof (buff));
    if (resp)return resp;

    resp = ed_fs_file_close();
    if (resp)return resp;


    printf("rom header was copied to [%s]\n", tst_file_wr);
    waitAB();

    return 0;
}

u8 romToFile(void) {

    u8 resp;

    console_clear();
    printf("\n\n");
    printf("write file from rom: [%s]...\n", tst_file_wr);
    console_render();

    resp = ed_fs_file_open(tst_file_wr, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (resp)return resp;

    resp = ed_fs_file_write_fci(ADDR_FCI_ROM, 4096);
    if (resp)return resp;

    resp = ed_fs_file_close();
    if (resp)return resp;


    printf("rom header was copied to [%s]\n", tst_file_wr);
    waitAB();

    return 0;
}

void printfToUsb(void) {

    console_clear();
    printf("\n\n");
    printf("run [edlink.exe usbrd --print]\n");
    printf("then press [A] to send message or [B] to return\n");
    console_render();

    //redirect libdragon's dprintf to usb
    stdio_t stdout_calls = {0, 0, ed_dbg_wr_usb};
    hook_stdio_calls(&stdout_calls);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);

    while (1) {

        joypad_poll();
        joypad_buttons_t joy = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        console_render();

        if (joy.b) {
            stdio_t stdout_calls = {0, 0, printf_stub};
            hook_stdio_calls(&stdout_calls);
            setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
            break;
        }
        if (!joy.a) {

            continue;
        }

        printf("hello world n64!\n");
    }
}

void ramToUsb(void) {

    //send current rom header via usb
    console_clear();
    printf("\n\n");
    printf("run [edlink.exe usbrd --file rx.bin --len 4096]\n");
    //printf("then press [A]\n");
    console_render();

    waitAB();

    u8 buff[4096];
    pi_rd(buff, ADDR_PI_ROM, sizeof (buff));

    ed_usb_wr(buff, sizeof (buff));
}

void romToUsb(void) {

    //send current rom header via usb
    console_clear();
    printf("\n\n");
    printf("run [edlink.exe usbrd --file rx.bin --len 4096]\n");
    //printf("then press [A]\n");
    console_render();

    waitAB();

    ed_usb_wr_fci(ADDR_FCI_ROM, 4096);
}

void usbPrint() {

    console_clear();
    printf("\n\n");
    printf("run [edlink.exe fifowr --file message.txt]\n");
    //printf("then press [A]\n");
    console_render();
    u8 buff[128];

    printf("press [A] or [B] key to exit\n");

    while (1) {

        // Warning: FIFO size is only 2048 bytes.
        // Do not send new data until the previous data is fully received.
        int len = ed_fifo_rda(buff, sizeof (buff));

        if (len > 0) {
            printf("%.*s", len, buff);
        }

        joypad_poll();
        joypad_buttons_t joy = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        console_render();
        if (joy.a || joy.b) {
            break;
        }
    }
}

u8 fileList() {

    u8 resp;
    u8 buff[1024];
    FileInfo inf = {.file_name = buff};
    u16 dir_size;

    resp = ed_fs_dir_load((u8 *) "", DIR_OPT_SORTED);
    if (resp)return resp;

    ed_fs_dir_get_size(&dir_size);
    if (resp)return resp;

    console_clear();
    printf("\n\n");

    for (int i = 0; i < dir_size; i++) {

        //Request records. 
        //Faster in bulk, but the total unread data size must not exceed the FIFO size (2048 bytes).
        //Quickly read all records, then process them, or read them one by one.
        ed_fs_dir_get_recs(i, 1, sizeof (buff) - 1);
        resp = ed_fs_rx_next_rec(&inf);
        if (resp)return resp;

        if (inf.is_dir) {
            printf("[FIL]");
        } else {
            printf("[DIR]");
        }

        printf(" %.40s\n", inf.file_name);
    }

    waitAB();

    return 0;
}

void rtcPrint() {

    RtcTime rtc = {0};

    console_clear();
    printf("\n\n");

    ed_rtc_get(&rtc);

    printf("rtc: %02X.%02X.20%02X %02X:%02X:%02X\n",
            rtc.dom, rtc.mon, rtc.yar, rtc.hur, rtc.min, rtc.sec);
    waitAB();

}

void sysPath() {

    u8 path[1024 + 1];
    console_clear();
    printf("\n\n");

    ed_dev_get_path(path, DEV_PATH_GPAK); //boot rom
    printf("gpak : %s\n", path);

    ed_dev_get_path(path, DEV_PATH_GDATA); //folder with game data (saves, configs etc)
    printf("gdata: %s\n", path);

    ed_dev_get_path(path, DEV_PATH_BRM); //save file
    printf("bram : %s\n", path);

    ed_dev_get_path(path, DEV_PATH_DISK); //dd disk (if mounted)
    printf("disk : %s\n", path);

    ed_dev_get_path(path, DEV_PATH_APPF); //target file for emulator (if any)
    printf("appf : %s\n", path);

    waitAB();
}

u8 sysConfig() {

    console_clear();
    printf("\n\n");
    printf("cart reconfigured!\n");
    printf("check results in \'system path\' menu\n");

    DevCfg cfg = {0};
    u8 resp;

    cfg.brm_size = 32768;
    cfg.brm_type = DEV_BRM_SRM32K;
    cfg.brom_type = DEV_ROM_GPAK;
    //cfg.gpak_wren = 1; //turn off wr protection for rom memory
    cfg.rtc_mode = DEV_RTCMODE_RDO;

    //everything turned off, bram handlers stopped
    ed_dev_stop(0);

    //rom path (this cmd does not actually load rom to the memory)
    resp = ed_dev_set_path((u8 *) "edio.n64", DEV_PATH_GPAK);
    if (resp)return resp;

    //where game configs stored
    resp = ed_dev_set_path((u8 *) "ed64/gamedata/edio.n64", DEV_PATH_GDATA);
    if (resp)return resp;

    //backup memory file
    resp = ed_dev_set_path((u8 *) "ed64/gamedata/edio.n64/bram.srm", DEV_PATH_BRM);
    if (resp)return resp;

    resp = ed_dev_set_cfg(&cfg);
    if (resp)return resp;

    //start bram handlers and configure fpga
    resp = ed_dev_start();
    if (resp)return resp;

    //start rom at this point

    waitAB();
    return 0;
}

void readDevId() {

    console_clear();
    printf("\n\n");
    printf("device id: %08X\n", (int) EDIO->EDID);
    waitAB();
}

u8 tcpipTest() {

    u8 *url = (u8 *) "tcp://time.nist.gov:13";
    u8 con_id;
    u8 resp;

    console_clear();
    printf("\n\n");
    printf("run [edlink.exe netgate]\n");
    waitAB();

    printf("Check netgate...\n");
    console_render();

    resp = ngTest();
    if (resp)return resp;

    printf("Establishing connection to %s\n", url);
    console_render();

    resp = ngOpen(url, &con_id);
    if (resp)return resp;

    printf("Read time strings...\n");
    console_render();

    u32 data_ctr = 0;
    u32 t = get_ticks_ms();
    while (get_ticks_ms() - t < 1000) {

        u8 buff[256];
        u16 r;

        resp = ngReadAvb(con_id, buff, sizeof (buff), &r);
        if (resp) return resp;

        if (r != 0) {
            printf("%.*s", r, buff);
            console_render();
            data_ctr += r;
        }
    }

    if (data_ctr == 0) {
        printf("Read timeout\n");
    }

    ngCloseAll();

    waitAB();

    return 0;
}

void mailbox() {

    // General-purpose mailbox register.
    // Can be used for fast status exchange between the n64 and PC.

    u32 ctr = 0xABCD1234;
    u8 joy_wait = 1;

    EDIO->MBX = 0;

    while (1) {

        console_clear();
        printf("run mbx_rd.py to read mailbox on PC\n");
        printf("run mbx_wr.py to write mailbox on PC\n");
        printf("press [A] to write ctr value in mailbox\n");
        printf("press [B] to exit\n");

        printf("mailbox: 0x%08lX\n", EDIO->MBX);
        console_render();

        joypad_poll();
        joypad_buttons_t joy = joypad_get_buttons_pressed(JOYPAD_PORT_1);

        if (joy_wait && joy.raw != 0) {
            continue;
        }

        joy_wait = 0;

        if (joy.a) {
            EDIO->MBX = ctr;
            ctr++;
            joy_wait = 1;
        } else if (joy.b) {
            return;
        }
    }
}

int main(void) {

    /* Initialize peripherals */
    console_init();
    joypad_init();
    timer_init();
    rtc_init();

    console_set_render_mode(RENDER_MANUAL);

    int selector = 0;

    enum {
        MENU_PRINTF_TO_USB, //dbg printf using usb
        MENU_FILE_TO_RAM, //read file via fifo (slow)
        MENU_FILE_TO_ROM, //read file to cart memory (fast)
        MENU_RAM_TO_FILE, //write file via fifo (slow)
        MENU_ROM_TO_FILE, //write file from cart memory (fast)
        MENU_RAM_TO_USB, //write to usb via fifo (slow)
        MENU_ROM_TO_USB, //write to usb from cart memory (fast)
        MENU_USB_PRINT,
        MENU_FILE_LIST,
        MENU_RTC_PRINT,
        MENU_SYS_PATH,
        MENU_SYS_CONFIG,
        MENU_READ_DEVID,
        MENU_TCPIP_TEST,
        MENU_MAILBOX,
        MENU_SIZE
    };

    char *menu[MENU_SIZE];
    menu[MENU_PRINTF_TO_USB] = "printf to usb";
    menu[MENU_FILE_TO_RAM] = "file to mem (ram)";
    menu[MENU_FILE_TO_ROM] = "file to mem (rom)";
    menu[MENU_RAM_TO_FILE] = "mem to file (ram)";
    menu[MENU_ROM_TO_FILE] = "mem to file (rom)";
    menu[MENU_RAM_TO_USB] = "mem to usb (ram)";
    menu[MENU_ROM_TO_USB] = "mem to usb (rom)";
    menu[MENU_USB_PRINT] = "usb read and print";
    menu[MENU_FILE_LIST] = "file list";
    menu[MENU_RTC_PRINT] = "rtc print";
    menu[MENU_SYS_PATH] = "system path";
    menu[MENU_SYS_CONFIG] = "system configuration";
    menu[MENU_READ_DEVID] = "read device id";
    menu[MENU_TCPIP_TEST] = "tcp/ip test";
    menu[MENU_MAILBOX] = "mailbox io";

    while (1) {

        console_clear();
        joypad_poll();
        joypad_buttons_t joy = joypad_get_buttons_pressed(JOYPAD_PORT_1);


        printf("\n\n");

        for (int i = 0; i < MENU_SIZE; i++) {

            printf("%c%s\n", selector == i ? '>' : ' ', menu[i]);
        }

        console_render();

        if (joy.d_up) {
            selector--;
            if (selector < 0) {
                selector = MENU_SIZE - 1;
            }
        }

        if (joy.d_down) {
            selector = (selector + 1) % MENU_SIZE;
        }

        if (!joy.a) {
            continue;
        }

        u8 resp = 0;

        switch (selector) {

            case MENU_PRINTF_TO_USB:
                printfToUsb();
                break;

            case MENU_FILE_TO_RAM:
                resp = fileToRam();
                break;

            case MENU_FILE_TO_ROM:
                resp = fileToRam();
                break;

            case MENU_RAM_TO_FILE:
                resp = ramToFile();
                break;

            case MENU_ROM_TO_FILE:
                resp = romToFile();
                break;

            case MENU_RAM_TO_USB:
                ramToUsb();
                break;

            case MENU_ROM_TO_USB:
                romToUsb();
                break;

            case MENU_USB_PRINT:
                usbPrint();
                break;

            case MENU_FILE_LIST:
                resp = fileList();
                break;

            case MENU_RTC_PRINT:
                rtcPrint();
                break;

            case MENU_SYS_PATH:
                sysPath();
                break;

            case MENU_SYS_CONFIG:
                resp = sysConfig();
                break;

            case MENU_READ_DEVID:
                readDevId();
                break;

            case MENU_TCPIP_TEST:
                resp = tcpipTest();
                break;

            case MENU_MAILBOX:
                mailbox();
                break;
        }

        if (resp) {
            printtError(resp);
        }
    }
}

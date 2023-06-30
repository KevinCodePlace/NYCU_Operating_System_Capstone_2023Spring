// mmio
#define KVA 0xffff000000000000
#define MMIO_BASE (KVA + 0x3f000000)

// SD card command
#define GO_IDLE_STATE 0
#define SEND_OP_CMD 1
#define ALL_SEND_CID 2
#define SEND_RELATIVE_ADDR 3
#define SELECT_CARD 7
#define SEND_IF_COND 8
#define VOLTAGE_CHECK_PATTERN 0x1aa
#define STOP_TRANSMISSION 12
#define SET_BLOCKLEN 16
#define READ_SINGLE_BLOCK 17
#define WRITE_SINGLE_BLOCK 24
#define SD_APP_OP_COND 41
#define SDCARD_3_3V (1 << 21)
#define SDCARD_ISHCS (1 << 30)
#define SDCARD_READY (1 << 31)
#define APP_CMD 55

// gpio
#define GPIO_BASE (MMIO_BASE + 0x200000)
#define GPIO_GPFSEL4 (GPIO_BASE + 0x10)
#define GPIO_GPFSEL5 (GPIO_BASE + 0x14)
#define GPIO_GPPUD (GPIO_BASE + 0x94)
#define GPIO_GPPUDCLK1 (GPIO_BASE + 0x9c)

// sdhost
#define SDHOST_BASE (MMIO_BASE + 0x202000)
#define SDHOST_CMD (SDHOST_BASE + 0)
#define SDHOST_READ 0x40
#define SDHOST_WRITE 0x80
#define SDHOST_LONG_RESPONSE 0x200
#define SDHOST_NO_REPONSE 0x400
#define SDHOST_BUSY 0x800
#define SDHOST_NEW_CMD 0x8000
#define SDHOST_ARG (SDHOST_BASE + 0x4)
#define SDHOST_TOUT (SDHOST_BASE + 0x8)
#define SDHOST_TOUT_DEFAULT 0xf00000
#define SDHOST_CDIV (SDHOST_BASE + 0xc)
#define SDHOST_CDIV_MAXDIV 0x7ff
#define SDHOST_CDIV_DEFAULT 0x148
#define SDHOST_RESP0 (SDHOST_BASE + 0x10)
#define SDHOST_RESP1 (SDHOST_BASE + 0x14)
#define SDHOST_RESP2 (SDHOST_BASE + 0x18)
#define SDHOST_RESP3 (SDHOST_BASE + 0x1c)
#define SDHOST_HSTS (SDHOST_BASE + 0x20)
#define SDHOST_HSTS_MASK (0x7f8)
#define SDHOST_HSTS_ERR_MASK (0xf8)
#define SDHOST_HSTS_DATA (1 << 0)
#define SDHOST_PWR (SDHOST_BASE + 0x30)
#define SDHOST_DBG (SDHOST_BASE + 0x34)
#define SDHOST_DBG_FSM_DATA 1
#define SDHOST_DBG_FSM_MASK 0xf
#define SDHOST_DBG_MASK (0x1f << 14 | 0x1f << 9)
#define SDHOST_DBG_FIFO (0x4 << 14 | 0x4 << 9)
#define SDHOST_CFG (SDHOST_BASE + 0x38)
#define SDHOST_CFG_DATA_EN (1 << 4)
#define SDHOST_CFG_SLOW (1 << 3)
#define SDHOST_CFG_INTBUS (1 << 1)
#define SDHOST_SIZE (SDHOST_BASE + 0x3c)
#define SDHOST_DATA (SDHOST_BASE + 0x40)
#define SDHOST_CNT (SDHOST_BASE + 0x50)

// helper
#define set(io_addr, val)                                 \
    asm volatile("str %w1, [%0]" ::"r"(io_addr), "r"(val) \
                 : "memory");

#define get(io_addr, val)        \
    asm volatile("ldr %w0, [%1]" \
                 : "=r"(val)     \
                 : "r"(io_addr)  \
                 : "memory");

static inline void delay(unsigned long tick)
{
    while (tick--) 
    {
        asm volatile("nop");
    }
}

static int is_hcs; // high capcacity(SDHC)

static void pin_setup()
{
    set(GPIO_GPFSEL4, 0x24000000);
    set(GPIO_GPFSEL5, 0x924);
    set(GPIO_GPPUD, 0);
    delay(15000);
    set(GPIO_GPPUDCLK1, 0xffffffff);
    delay(15000);
    set(GPIO_GPPUDCLK1, 0);
}

static void sdhost_setup()
{
    unsigned int tmp;
    set(SDHOST_PWR, 0);
    set(SDHOST_CMD, 0);
    set(SDHOST_ARG, 0);
    set(SDHOST_TOUT, SDHOST_TOUT_DEFAULT);
    set(SDHOST_CDIV, 0);
    set(SDHOST_HSTS, SDHOST_HSTS_MASK);
    set(SDHOST_CFG, 0);
    set(SDHOST_CNT, 0);
    set(SDHOST_SIZE, 0);
    get(SDHOST_DBG, tmp);
    tmp &= ~SDHOST_DBG_MASK;
    tmp |= SDHOST_DBG_FIFO;
    set(SDHOST_DBG, tmp);
    delay(250000);
    set(SDHOST_PWR, 1);
    delay(250000);
    set(SDHOST_CFG, SDHOST_CFG_SLOW | SDHOST_CFG_INTBUS | SDHOST_CFG_DATA_EN);
    set(SDHOST_CDIV, SDHOST_CDIV_DEFAULT);
}

static int wait_sd()
{
    int cnt = 1000000;
    unsigned int cmd;
    do
    {
        if (cnt == 0)
        {
            return -1;
        }
        get(SDHOST_CMD, cmd);
        --cnt;
    } while (cmd & SDHOST_NEW_CMD);
    return 0;
}

static int sd_cmd(unsigned cmd, unsigned int arg)
{
    set(SDHOST_ARG, arg);
    set(SDHOST_CMD, cmd | SDHOST_NEW_CMD);
    return wait_sd();
}

static int sdcard_setup()
{
    unsigned int tmp;
    sd_cmd(GO_IDLE_STATE | SDHOST_NO_REPONSE, 0);
    sd_cmd(SEND_IF_COND, VOLTAGE_CHECK_PATTERN);
    get(SDHOST_RESP0, tmp);
    if (tmp != VOLTAGE_CHECK_PATTERN)
    {
        return -1;
    }
    while (1)
    {
        if (sd_cmd(APP_CMD, 0) == -1)
        {
            // MMC card or invalid card status
            // currently not support
            continue;
        }
        sd_cmd(SD_APP_OP_COND, SDCARD_3_3V | SDCARD_ISHCS);
        get(SDHOST_RESP0, tmp);
        if (tmp & SDCARD_READY)
        {
            break;
        }
        delay(1000000);
    }

    is_hcs = tmp & SDCARD_ISHCS;
    sd_cmd(ALL_SEND_CID | SDHOST_LONG_RESPONSE, 0);
    sd_cmd(SEND_RELATIVE_ADDR, 0);
    get(SDHOST_RESP0, tmp);
    sd_cmd(SELECT_CARD, tmp);
    sd_cmd(SET_BLOCKLEN, 512);
    return 0;
}

static int wait_fifo()
{
    int cnt = 1000000;
    unsigned int hsts;
    do
    {
        if (cnt == 0)
        {
            return -1;
        }
        get(SDHOST_HSTS, hsts);
        --cnt;
    } while ((hsts & SDHOST_HSTS_DATA) == 0);
    return 0;
}

static void set_block(int size, int cnt)
{
    set(SDHOST_SIZE, size);
    set(SDHOST_CNT, cnt);
}

static void wait_finish()
{
    unsigned int dbg;
    do
    {
        get(SDHOST_DBG, dbg);
    } while ((dbg & SDHOST_DBG_FSM_MASK) != SDHOST_HSTS_DATA);
}

void readblock(int block_idx, void *buf)
{
    unsigned int *buf_u = (unsigned int *)buf;
    int succ = 0;
    if (!is_hcs)
    {
        block_idx <<= 9;
    }
    do
    {
        set_block(512, 1);
        sd_cmd(READ_SINGLE_BLOCK | SDHOST_READ, block_idx);
        for (int i = 0; i < 128; ++i)
        {
            wait_fifo();
            get(SDHOST_DATA, buf_u[i]);
        }
        unsigned int hsts;
        get(SDHOST_HSTS, hsts);
        if (hsts & SDHOST_HSTS_ERR_MASK)
        {
            set(SDHOST_HSTS, SDHOST_HSTS_ERR_MASK);
            sd_cmd(STOP_TRANSMISSION | SDHOST_BUSY, 0);
        }
        else
        {
            succ = 1;
        }
    } while (!succ);
    wait_finish();
}

void writeblock(int block_idx, void *buf)
{
    unsigned int *buf_u = (unsigned int *)buf;
    int succ = 0;
    if (!is_hcs)
    {
        block_idx <<= 9;
    }
    do
    {
        set_block(512, 1);
        sd_cmd(WRITE_SINGLE_BLOCK | SDHOST_WRITE, block_idx);
        for (int i = 0; i < 128; ++i)
        {
            wait_fifo();
            set(SDHOST_DATA, buf_u[i]);
        }
        unsigned int hsts;
        get(SDHOST_HSTS, hsts);
        if (hsts & SDHOST_HSTS_ERR_MASK)
        {
            set(SDHOST_HSTS, SDHOST_HSTS_ERR_MASK);
            sd_cmd(STOP_TRANSMISSION | SDHOST_BUSY, 0);
        }
        else
        {
            succ = 1;
        }
    } while (!succ);
    wait_finish();
}

void sd_init()
{
    pin_setup();
    sdhost_setup();
    sdcard_setup();
}
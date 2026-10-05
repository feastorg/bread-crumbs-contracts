/* A fake I2C bus for the round-trip tests: it decodes every frame written
 * to it and serves one encoded reply on read, so a controller wrapper or
 * getter runs unmodified against the codec the peripheral uses. */
#ifndef BREAD_TEST_FAKE_BUS_H
#define BREAD_TEST_FAKE_BUS_H

#include <stdio.h>
#include <string.h>

#include "crumbs.h"

typedef struct
{
    crumbs_message_t last_write; /* decoded from the last frame written */
    int write_rc;
    int writes;
    uint8_t reply[CRUMBS_MESSAGE_MAX_SIZE]; /* served, padded, on read */
    size_t reply_len;
} fake_bus_t;

static fake_bus_t g_bus;
static crumbs_context_t g_ctx;
static crumbs_device_t g_dev;
static int g_failures;

#define CHECK(cond, what)                                                      \
    do                                                                         \
    {                                                                          \
        if (!(cond))                                                           \
        {                                                                      \
            printf("FAIL %s:%d: %s (%s)\n", __FILE__, __LINE__, what, #cond); \
            g_failures++;                                                      \
        }                                                                      \
    } while (0)

static int fake_bus_write(void *io, uint8_t addr, const uint8_t *data, size_t len)
{
    fake_bus_t *b = (fake_bus_t *)io;
    (void)addr;
    b->writes++;
    b->write_rc = crumbs_decode_message(data, len, &b->last_write, NULL);
    return 0;
}

static int fake_bus_read(void *io, uint8_t addr, uint8_t *buf, size_t len, uint32_t timeout_us)
{
    fake_bus_t *b = (fake_bus_t *)io;
    (void)addr;
    (void)timeout_us;
    memset(buf, 0xFF, len);
    memcpy(buf, b->reply, b->reply_len < len ? b->reply_len : len);
    return (int)len;
}

static void fake_bus_delay(uint32_t us) { (void)us; }

static void fake_bus_bind(void)
{
    memset(&g_bus, 0, sizeof g_bus);
    crumbs_init(&g_ctx, CRUMBS_ROLE_CONTROLLER, 0);
    memset(&g_dev, 0, sizeof g_dev);
    g_dev.ctx = &g_ctx;
    g_dev.addr = 0x20;
    g_dev.write_fn = fake_bus_write;
    g_dev.read_fn = fake_bus_read;
    g_dev.delay_fn = fake_bus_delay;
    g_dev.io = &g_bus;
}

/* The peripheral's side of a GET: serve the reply its handler built. */
static void fake_bus_serve(const crumbs_message_t *reply)
{
    g_bus.reply_len = crumbs_encode_message(reply, g_bus.reply, sizeof g_bus.reply);
}

/* The last frame written was a SET of this type and opcode with this payload. */
static int fake_bus_sent(uint8_t type_id, uint8_t opcode, const uint8_t *payload, uint8_t len)
{
    return g_bus.write_rc == 0 && g_bus.last_write.type_id == type_id &&
           g_bus.last_write.opcode == opcode && g_bus.last_write.data_len == len &&
           memcmp(g_bus.last_write.data, payload, len) == 0;
}

#endif /* BREAD_TEST_FAKE_BUS_H */

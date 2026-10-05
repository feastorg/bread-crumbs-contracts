#ifndef RLHT_OPS_H
#define RLHT_OPS_H

#include "crumbs.h"
#include "crumbs_message_helpers.h"
#include "crumbs_ops.h"
#include "bread_caps.h"
#include "bread_watchdog.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Type and opcodes, declared once: the build fails on a duplicate opcode,
 * an opcode above 0xFF or equal to 0xFE, or a type of 0x00. Each payload
 * layout is declared once below and packed and unpacked through the
 * generated codec, by the wrappers here and by the Slice firmware.
 */
#define RLHT_OPS(X)                                                    \
    X(RLHT_OP_SET_MODE, 0x01)      /* rlht_set_mode, 1 byte */         \
    X(RLHT_OP_SET_SETPOINTS, 0x02) /* rlht_set_setpoints, 4 bytes */   \
    X(RLHT_OP_SET_PID, 0x03)       /* rlht_set_pid, 6 bytes */         \
    X(RLHT_OP_SET_PERIODS, 0x04)   /* rlht_set_periods, 4 bytes */     \
    X(RLHT_OP_SET_TC_SELECT, 0x05) /* rlht_set_tc_select, 2 bytes */   \
    X(RLHT_OP_SET_OPEN_DUTY, 0x06) /* rlht_set_open_duty, 2 bytes */   \
    X(RLHT_OP_GET_STATE, 0x80)     /* reply rlht_state, 19 bytes */
CRUMBS_DEFINE_FAMILY(RLHT, 0x01, RLHT_OPS)

#define RLHT_MODULE_VER_MAJOR 1
#define RLHT_MODULE_VER_MINOR 0
#define RLHT_MODULE_VER_PATCH 0

#define RLHT_MODE_CLOSED_LOOP 0x00
#define RLHT_MODE_OPEN_LOOP 0x01

/* SET_MODE: [mode:u8] (RLHT_MODE_*) */
#define RLHT_SET_MODE_FIELDS(X) X(u8, mode)
CRUMBS_DEFINE_PAYLOAD(rlht_set_mode, 1, RLHT_SET_MODE_FIELDS)

/* SET_SETPOINTS: [sp1:i16][sp2:i16], deci-degrees C */
#define RLHT_SET_SETPOINTS_FIELDS(X) X(i16, sp1_deci_c) X(i16, sp2_deci_c)
CRUMBS_DEFINE_PAYLOAD(rlht_set_setpoints, 4, RLHT_SET_SETPOINTS_FIELDS)

/* SET_PID: [kp1][ki1][kd1][kp2][ki2][kd2], each u8, gain x10 */
#define RLHT_SET_PID_FIELDS(X) \
    X(u8, kp1_x10)             \
    X(u8, ki1_x10)             \
    X(u8, kd1_x10)             \
    X(u8, kp2_x10)             \
    X(u8, ki2_x10)             \
    X(u8, kd2_x10)
CRUMBS_DEFINE_PAYLOAD(rlht_set_pid, 6, RLHT_SET_PID_FIELDS)

/* SET_PERIODS: [p1_ms:u16][p2_ms:u16] */
#define RLHT_SET_PERIODS_FIELDS(X) X(u16, p1_ms) X(u16, p2_ms)
CRUMBS_DEFINE_PAYLOAD(rlht_set_periods, 4, RLHT_SET_PERIODS_FIELDS)

/* SET_TC_SELECT: [tc1:u8][tc2:u8] */
#define RLHT_SET_TC_SELECT_FIELDS(X) X(u8, tc1) X(u8, tc2)
CRUMBS_DEFINE_PAYLOAD(rlht_set_tc_select, 2, RLHT_SET_TC_SELECT_FIELDS)

/* SET_OPEN_DUTY: [duty1_pct:u8][duty2_pct:u8] */
#define RLHT_SET_OPEN_DUTY_FIELDS(X) X(u8, duty1_pct) X(u8, duty2_pct)
CRUMBS_DEFINE_PAYLOAD(rlht_set_open_duty, 2, RLHT_SET_OPEN_DUTY_FIELDS)

/*
 * GET_STATE reply (19 bytes):
 * [mode:u8][flags:u8][t1:i16][t2:i16][sp1:i16][sp2:i16]
 * [on1_ms:u16][on2_ms:u16][period1_ms:u16][period2_ms:u16][tc_select:u8]
 * tc_select packs tc1 in bits 0-1 and tc2 in bits 2-3.
 */
#define RLHT_STATE_FIELDS(X) \
    X(u8, mode)              \
    X(u8, flags)             \
    X(i16, t1_deci_c)        \
    X(i16, t2_deci_c)        \
    X(i16, sp1_deci_c)       \
    X(i16, sp2_deci_c)       \
    X(u16, on1_ms)           \
    X(u16, on2_ms)           \
    X(u16, period1_ms)       \
    X(u16, period2_ms)       \
    X(u8, tc_select)
CRUMBS_DEFINE_PAYLOAD(rlht_state, 19, RLHT_STATE_FIELDS)

#define RLHT_FLAG_ESTOP 0x01
#define RLHT_FLAG_RELAY1_ON 0x02
#define RLHT_FLAG_RELAY2_ON 0x04

#define RLHT_CAP_LEVEL_1 0x01
#define RLHT_CAP_LEVEL_2 0x02
#define RLHT_CAP_LEVEL_3 0x03

#define RLHT_CAP_MODE_CONTROL ((uint32_t)1u << 0)
#define RLHT_CAP_SETPOINT_CONTROL ((uint32_t)1u << 1)
#define RLHT_CAP_PID_TUNING ((uint32_t)1u << 2)
#define RLHT_CAP_PERIOD_CONTROL ((uint32_t)1u << 3)
#define RLHT_CAP_TC_SELECT ((uint32_t)1u << 4)
#define RLHT_CAP_OPEN_DUTY_CONTROL ((uint32_t)1u << 5)
#define RLHT_CAP_CMD_WATCHDOG ((uint32_t)1u << 6)

#define RLHT_CAP_BASELINE_FLAGS (RLHT_CAP_MODE_CONTROL | RLHT_CAP_SETPOINT_CONTROL | \
                                 RLHT_CAP_PID_TUNING | RLHT_CAP_PERIOD_CONTROL | \
                                 RLHT_CAP_TC_SELECT | RLHT_CAP_OPEN_DUTY_CONTROL)

typedef struct
{
    uint8_t mode;
    uint8_t flags;
    int16_t t1_deci_c;
    int16_t t2_deci_c;
    int16_t sp1_deci_c;
    int16_t sp2_deci_c;
    uint16_t on1_ms;
    uint16_t on2_ms;
    uint16_t period1_ms;
    uint16_t period2_ms;
    uint8_t tc_select;
    uint8_t tc1;
    uint8_t tc2;
} rlht_state_result_t;

typedef struct
{
    uint16_t crumbs_version;
    uint8_t fw_major;
    uint8_t fw_minor;
    uint8_t fw_patch;
} rlht_version_result_t;

typedef struct
{
    uint8_t schema;
    uint8_t level;
    uint32_t flags;
} rlht_caps_result_t;

static inline int rlht_validate_write_device(const crumbs_device_t *dev)
{
    if (!dev || !dev->ctx || !dev->write_fn)
        return -1;
    return 0;
}

static inline int rlht_validate_query_device(const crumbs_device_t *dev)
{
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    if (!dev->read_fn || !dev->delay_fn)
        return -1;
    return 0;
}

/*
 * SET wrappers keep their scalar parameters and pack through the payload
 * struct, so the bytes come from the same field list the Slice unpacks.
 */
static inline int rlht_send_set_mode(const crumbs_device_t *dev, uint8_t mode)
{
    crumbs_message_t msg;
    rlht_set_mode_t v;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    v.mode = mode;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_MODE);
    if (rlht_set_mode_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_setpoints(const crumbs_device_t *dev, int16_t sp1_deci_c, int16_t sp2_deci_c)
{
    crumbs_message_t msg;
    rlht_set_setpoints_t v;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    v.sp1_deci_c = sp1_deci_c;
    v.sp2_deci_c = sp2_deci_c;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_SETPOINTS);
    if (rlht_set_setpoints_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_pid_x10(const crumbs_device_t *dev,
                                        uint8_t kp1_x10, uint8_t ki1_x10, uint8_t kd1_x10,
                                        uint8_t kp2_x10, uint8_t ki2_x10, uint8_t kd2_x10)
{
    crumbs_message_t msg;
    rlht_set_pid_t v;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    v.kp1_x10 = kp1_x10;
    v.ki1_x10 = ki1_x10;
    v.kd1_x10 = kd1_x10;
    v.kp2_x10 = kp2_x10;
    v.ki2_x10 = ki2_x10;
    v.kd2_x10 = kd2_x10;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_PID);
    if (rlht_set_pid_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_periods(const crumbs_device_t *dev, uint16_t p1_ms, uint16_t p2_ms)
{
    crumbs_message_t msg;
    rlht_set_periods_t v;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    v.p1_ms = p1_ms;
    v.p2_ms = p2_ms;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_PERIODS);
    if (rlht_set_periods_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_tc_select(const crumbs_device_t *dev, uint8_t tc1, uint8_t tc2)
{
    crumbs_message_t msg;
    rlht_set_tc_select_t v;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    v.tc1 = tc1;
    v.tc2 = tc2;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_TC_SELECT);
    if (rlht_set_tc_select_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_open_duty(const crumbs_device_t *dev, uint8_t duty1_pct, uint8_t duty2_pct)
{
    crumbs_message_t msg;
    rlht_set_open_duty_t v;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    v.duty1_pct = duty1_pct;
    v.duty2_pct = duty2_pct;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_OPEN_DUTY);
    if (rlht_set_open_duty_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_watchdog(const crumbs_device_t *dev, uint16_t timeout_ms)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, BREAD_OP_SET_WATCHDOG);
    crumbs_msg_add_u16(&msg, timeout_ms);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_query_state(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, RLHT_OP_GET_STATE);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_query_version(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, 0x00);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_query_caps(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, BREAD_OP_GET_CAPS);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_query_watchdog(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, BREAD_OP_GET_WATCHDOG);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

/*
 * Parse an RLHT GET_STATE payload into a state result. Shared by
 * rlht_get_state() and by controllers that run the query round-trip through
 * their own transport (retry/locking/timing) and only need the wire layout.
 * Reads the rlht_state layout from the front of the payload and ignores
 * trailing bytes; a shorter payload returns -1 and leaves out untouched.
 */
static inline int rlht_parse_state_payload(const uint8_t *data, uint8_t data_len, rlht_state_result_t *out)
{
    rlht_state_t s;

    if (!data || !out)
        return -1;
    if (rlht_state_unpack(data, data_len, &s) != 0)
        return -1;

    out->mode = s.mode;
    out->flags = s.flags;
    out->t1_deci_c = s.t1_deci_c;
    out->t2_deci_c = s.t2_deci_c;
    out->sp1_deci_c = s.sp1_deci_c;
    out->sp2_deci_c = s.sp2_deci_c;
    out->on1_ms = s.on1_ms;
    out->on2_ms = s.on2_ms;
    out->period1_ms = s.period1_ms;
    out->period2_ms = s.period2_ms;
    out->tc_select = s.tc_select;
    out->tc1 = (uint8_t)(s.tc_select & 0x03);
    out->tc2 = (uint8_t)((s.tc_select >> 2) & 0x03);
    return 0;
}

static inline int rlht_get_state(const crumbs_device_t *dev, rlht_state_result_t *out)
{
    crumbs_message_t reply;
    int rc;

    if (!out || rlht_validate_query_device(dev) != 0)
        return -1;

    rc = rlht_query_state(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != RLHT_TYPE_ID || reply.opcode != RLHT_OP_GET_STATE)
        return -1;

    return rlht_parse_state_payload(reply.data, reply.data_len, out);
}

static inline int rlht_get_version(const crumbs_device_t *dev, rlht_version_result_t *out)
{
    crumbs_message_t reply;
    int rc;

    if (!out || rlht_validate_query_device(dev) != 0)
        return -1;

    rc = rlht_query_version(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != RLHT_TYPE_ID || reply.opcode != 0x00)
        return -1;

    rc = crumbs_msg_read_u16(reply.data, reply.data_len, 0, &out->crumbs_version);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 2, &out->fw_major);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 3, &out->fw_minor);
    if (rc != 0)
        return rc;
    return crumbs_msg_read_u8(reply.data, reply.data_len, 4, &out->fw_patch);
}

static inline int rlht_get_caps(const crumbs_device_t *dev, rlht_caps_result_t *out)
{
    crumbs_message_t reply;
    bread_caps_result_t parsed;
    int rc;

    if (!out || rlht_validate_query_device(dev) != 0)
        return -1;

    rc = rlht_query_caps(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != RLHT_TYPE_ID || reply.opcode != BREAD_OP_GET_CAPS)
        return -1;

    rc = bread_caps_parse_payload(reply.data, reply.data_len, &parsed);
    if (rc != 0)
        return rc;

    out->schema = parsed.schema;
    out->level = parsed.level;
    out->flags = parsed.flags;
    return 0;
}

static inline int rlht_get_watchdog(const crumbs_device_t *dev, bread_watchdog_result_t *out)
{
    crumbs_message_t reply;
    int rc;

    if (!out || rlht_validate_query_device(dev) != 0)
        return -1;

    rc = rlht_query_watchdog(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != RLHT_TYPE_ID || reply.opcode != BREAD_OP_GET_WATCHDOG)
        return -1;

    return bread_watchdog_parse_payload(reply.data, reply.data_len, out);
}

#ifdef __cplusplus
}
#endif

#endif // RLHT_OPS_H

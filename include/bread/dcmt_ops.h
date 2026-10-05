#ifndef DCMT_OPS_H
#define DCMT_OPS_H

#include "crumbs.h"

/* CRUMBS_DEFINE_FAMILY / CRUMBS_DEFINE_PAYLOAD arrived in CRUMBS 0.14.0; an
 * older crumbs_ops.h exists but lacks them, which fails far below here. */
#if !defined(CRUMBS_VERSION) || CRUMBS_VERSION < 1400
#error "bread-crumbs-contracts needs CRUMBS 0.14.0 or newer"
#endif

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
 * generated codec by the wrappers here; Slice firmware is meant to use the
 * same pack/unpack (feastorg/Slice_RLHT#11, feastorg/Slice_DCMT#28).
 */
#define DCMT_OPS(X)                                                      \
    X(DCMT_OP_SET_OPEN_LOOP, 0x01) /* dcmt_set_open_loop, 4 bytes */     \
    X(DCMT_OP_SET_BRAKE, 0x02)     /* dcmt_set_brake, 2 bytes */         \
    X(DCMT_OP_SET_MODE, 0x03)      /* dcmt_set_mode, 1 byte */           \
    X(DCMT_OP_SET_SETPOINT, 0x04)  /* dcmt_set_setpoint, 4 bytes */      \
    X(DCMT_OP_SET_PID, 0x05)       /* dcmt_set_pid, 6 bytes */           \
    X(DCMT_OP_GET_STATE, 0x80)     /* reply dcmt_state, 19 bytes */
CRUMBS_DEFINE_FAMILY(DCMT, 0x02, DCMT_OPS)

#define DCMT_MODULE_VER_MAJOR 1
#define DCMT_MODULE_VER_MINOR 0
#define DCMT_MODULE_VER_PATCH 0

/* SET_OPEN_LOOP: [m1_pwm:i16][m2_pwm:i16] */
#define DCMT_SET_OPEN_LOOP_FIELDS(X) X(i16, m1_pwm) X(i16, m2_pwm)
CRUMBS_DEFINE_PAYLOAD(dcmt_set_open_loop, 4, DCMT_SET_OPEN_LOOP_FIELDS)

/* SET_BRAKE: [m1_brake:u8][m2_brake:u8] */
#define DCMT_SET_BRAKE_FIELDS(X) X(u8, m1_brake) X(u8, m2_brake)
CRUMBS_DEFINE_PAYLOAD(dcmt_set_brake, 2, DCMT_SET_BRAKE_FIELDS)

/* SET_MODE: [mode:u8] (DCMT_MODE_*) */
#define DCMT_SET_MODE_FIELDS(X) X(u8, mode)
CRUMBS_DEFINE_PAYLOAD(dcmt_set_mode, 1, DCMT_SET_MODE_FIELDS)

/* SET_SETPOINT: [target1:i16][target2:i16] */
#define DCMT_SET_SETPOINT_FIELDS(X) X(i16, target1) X(i16, target2)
CRUMBS_DEFINE_PAYLOAD(dcmt_set_setpoint, 4, DCMT_SET_SETPOINT_FIELDS)

/* SET_PID: [kp1][ki1][kd1][kp2][ki2][kd2], each u8, gain x10 */
#define DCMT_SET_PID_FIELDS(X) \
    X(u8, kp1_x10)             \
    X(u8, ki1_x10)             \
    X(u8, kd1_x10)             \
    X(u8, kp2_x10)             \
    X(u8, ki2_x10)             \
    X(u8, kd2_x10)
CRUMBS_DEFINE_PAYLOAD(dcmt_set_pid, 6, DCMT_SET_PID_FIELDS)

// Fixed GET_STATE payload layout (19 bytes):
// [mode:u8][m1_pwm:i16][m2_pwm:i16][sp1:i16][sp2:i16]
// [pos1:i16][pos2:i16][spd1:i16][spd2:i16][brakes:u8][estop:u8]
//
// Field validity:
//   Always valid (never sentinel): m1_pwm, m2_pwm, pos1, pos2, mode, brakes, estop
//   Sentinel-eligible (may be BREAD_INVALID_I16):
//     sp1/sp2  -> BREAD_INVALID_I16 when mode == DCMT_MODE_OPEN_LOOP
//     spd1/spd2 -> BREAD_INVALID_I16 unless mode == DCMT_MODE_CLOSED_SPEED
// Use BREAD_IS_VALID_I16() before consuming sentinel-eligible fields.
#define DCMT_STATE_FIELDS(X)                                                     \
    X(u8, mode)    /* DCMT_MODE_* */                                             \
    X(i16, m1_pwm) /* current PWM output, always valid */                        \
    X(i16, m2_pwm)                                                               \
    X(i16, sp1)    /* active setpoint; BREAD_INVALID_I16 in OPEN_LOOP */         \
    X(i16, sp2)                                                                  \
    X(i16, pos1)   /* encoder position; always populated */                      \
    X(i16, pos2)                                                                 \
    X(i16, spd1)   /* tachometer speed; BREAD_INVALID_I16 unless CLOSED_SPEED */ \
    X(i16, spd2)                                                                 \
    X(u8, brakes)                                                                \
    X(u8, estop)
CRUMBS_DEFINE_PAYLOAD(dcmt_state, 19, DCMT_STATE_FIELDS)

/* The parser's result is the payload struct itself. */
typedef dcmt_state_t dcmt_state_result_t;

/* Byte offsets of the GET_STATE fields, for code that reads the payload
   directly; tests/payload_roundtrip checks each against dcmt_state_pack(). */
#define DCMT_STATE_OFF_MODE 0
#define DCMT_STATE_OFF_M1_PWM 1
#define DCMT_STATE_OFF_M2_PWM 3
#define DCMT_STATE_OFF_SP1 5
#define DCMT_STATE_OFF_SP2 7
#define DCMT_STATE_OFF_POS1 9
#define DCMT_STATE_OFF_POS2 11
#define DCMT_STATE_OFF_SPD1 13
#define DCMT_STATE_OFF_SPD2 15
#define DCMT_STATE_OFF_BRAKES 17
#define DCMT_STATE_OFF_ESTOP 18
#define DCMT_STATE_FIXED_LEN 19
CRUMBS_STATIC_ASSERT(DCMT_STATE_FIXED_LEN == dcmt_state_wire_size,
                     "DCMT_STATE_FIXED_LEN must equal the dcmt_state field list");

#define DCMT_MODE_OPEN_LOOP 0x00
#define DCMT_MODE_CLOSED_POSITION 0x01
#define DCMT_MODE_CLOSED_SPEED 0x02

#define DCMT_CAP_LEVEL_1 0x01
#define DCMT_CAP_LEVEL_2 0x02
#define DCMT_CAP_LEVEL_3 0x03

#define DCMT_CAP_OPEN_LOOP_CONTROL ((uint32_t)1u << 0)
#define DCMT_CAP_BRAKE_CONTROL ((uint32_t)1u << 1)
#define DCMT_CAP_CLOSED_LOOP_POSITION ((uint32_t)1u << 2)
#define DCMT_CAP_CLOSED_LOOP_SPEED ((uint32_t)1u << 3)
#define DCMT_CAP_PID_TUNING ((uint32_t)1u << 4)
#define DCMT_CAP_CMD_WATCHDOG ((uint32_t)1u << 5)
/* The watchdog trip latches until BREAD_OP_CLEAR_WATCHDOG_TRIP, a deliberate
   local operator command on the slice (firmware-defined, not any serial
   input), or reboot: SET_WATCHDOG and ordinary frames no longer clear it,
   and CLEAR_WATCHDOG_TRIP is handled. Advertised only together with
   DCMT_CAP_CMD_WATCHDOG. Without it, SET_WATCHDOG clears the trip. */
#define DCMT_CAP_CLEAR_WATCHDOG_TRIP ((uint32_t)1u << 6)

#define DCMT_CAP_BASELINE_FLAGS (DCMT_CAP_OPEN_LOOP_CONTROL | DCMT_CAP_BRAKE_CONTROL)

static inline int dcmt_validate_write_device(const crumbs_device_t *dev)
{
    if (!dev || !dev->ctx || !dev->write_fn)
        return -1;
    return 0;
}

static inline int dcmt_validate_query_device(const crumbs_device_t *dev)
{
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    if (!dev->read_fn || !dev->delay_fn)
        return -1;
    return 0;
}

/*
 * SET wrappers keep their scalar parameters and pack through the payload
 * struct, so the bytes come from the same field list the Slice unpacks.
 */
static inline int dcmt_send_set_open_loop(const crumbs_device_t *dev, int16_t m1_pwm, int16_t m2_pwm)
{
    crumbs_message_t msg;
    dcmt_set_open_loop_t v;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    v.m1_pwm = m1_pwm;
    v.m2_pwm = m2_pwm;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_OPEN_LOOP);
    if (dcmt_set_open_loop_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_brake(const crumbs_device_t *dev, uint8_t m1_brake, uint8_t m2_brake)
{
    crumbs_message_t msg;
    dcmt_set_brake_t v;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    v.m1_brake = m1_brake;
    v.m2_brake = m2_brake;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_BRAKE);
    if (dcmt_set_brake_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_mode(const crumbs_device_t *dev, uint8_t mode)
{
    crumbs_message_t msg;
    dcmt_set_mode_t v;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    v.mode = mode;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_MODE);
    if (dcmt_set_mode_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_setpoint(const crumbs_device_t *dev, int16_t target1, int16_t target2)
{
    crumbs_message_t msg;
    dcmt_set_setpoint_t v;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    v.target1 = target1;
    v.target2 = target2;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_SETPOINT);
    if (dcmt_set_setpoint_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_pid(const crumbs_device_t *dev,
                                    uint8_t kp1_x10, uint8_t ki1_x10, uint8_t kd1_x10,
                                    uint8_t kp2_x10, uint8_t ki2_x10, uint8_t kd2_x10)
{
    crumbs_message_t msg;
    dcmt_set_pid_t v;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    v.kp1_x10 = kp1_x10;
    v.ki1_x10 = ki1_x10;
    v.kd1_x10 = kd1_x10;
    v.kp2_x10 = kp2_x10;
    v.ki2_x10 = ki2_x10;
    v.kd2_x10 = kd2_x10;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_PID);
    if (dcmt_set_pid_pack(&msg, &v) != 0)
        return -1;
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_watchdog(const crumbs_device_t *dev, uint16_t timeout_ms)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, BREAD_OP_SET_WATCHDOG);
    crumbs_msg_add_u16(&msg, timeout_ms);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

/* Send only when DCMT_CAP_CLEAR_WATCHDOG_TRIP is advertised, and only on an
   operator's request. Empty payload; clears the trip and nothing else. */
static inline int dcmt_send_clear_watchdog_trip(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, BREAD_OP_CLEAR_WATCHDOG_TRIP);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_query_state(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, DCMT_OP_GET_STATE);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_query_version(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, 0x00);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_query_caps(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, BREAD_OP_GET_CAPS);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_query_watchdog(const crumbs_device_t *dev)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, 0, CRUMBS_CMD_SET_REPLY);
    crumbs_msg_add_u8(&msg, BREAD_OP_GET_WATCHDOG);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

typedef struct
{
    uint16_t crumbs_version;
    uint8_t fw_major;
    uint8_t fw_minor;
    uint8_t fw_patch;
} dcmt_version_result_t;

typedef struct
{
    uint8_t schema;
    uint8_t level;
    uint32_t flags;
} dcmt_caps_result_t;


/*
 * Parse a DCMT GET_STATE payload into a state result. Shared by
 * dcmt_get_state() and by controllers that run the query round-trip through
 * their own transport (retry/locking/timing) and only need the wire layout.
 * The payload must be exactly DCMT_STATE_FIXED_LEN bytes; anything else
 * returns -1 and leaves out untouched.
 */
static inline int dcmt_parse_state_payload(const uint8_t *data, uint8_t data_len, dcmt_state_result_t *out)
{
    if (!data || !out)
        return -1;

    if (data_len != DCMT_STATE_FIXED_LEN)
        return -1;

    return dcmt_state_unpack(data, data_len, out);
}

static inline int dcmt_get_state(const crumbs_device_t *dev, dcmt_state_result_t *out)
{
    crumbs_message_t reply;
    int rc;

    if (!out || dcmt_validate_query_device(dev) != 0)
        return -1;

    rc = dcmt_query_state(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != DCMT_TYPE_ID || reply.opcode != DCMT_OP_GET_STATE)
        return -1;

    return dcmt_parse_state_payload(reply.data, reply.data_len, out);
}

static inline int dcmt_get_version(const crumbs_device_t *dev, dcmt_version_result_t *out)
{
    crumbs_message_t reply;
    int rc;

    if (!out || dcmt_validate_query_device(dev) != 0)
        return -1;

    rc = dcmt_query_version(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != DCMT_TYPE_ID || reply.opcode != 0x00)
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

static inline int dcmt_get_caps(const crumbs_device_t *dev, dcmt_caps_result_t *out)
{
    crumbs_message_t reply;
    bread_caps_result_t parsed;
    int rc;

    if (!out || dcmt_validate_query_device(dev) != 0)
        return -1;

    rc = dcmt_query_caps(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != DCMT_TYPE_ID || reply.opcode != BREAD_OP_GET_CAPS)
        return -1;

    rc = bread_caps_parse_payload(reply.data, reply.data_len, &parsed);
    if (rc != 0)
        return rc;

    out->schema = parsed.schema;
    out->level = parsed.level;
    out->flags = parsed.flags;
    return 0;
}

static inline int dcmt_get_watchdog(const crumbs_device_t *dev, bread_watchdog_result_t *out)
{
    crumbs_message_t reply;
    int rc;

    if (!out || dcmt_validate_query_device(dev) != 0)
        return -1;

    rc = dcmt_query_watchdog(dev);
    if (rc != 0)
        return rc;

    dev->delay_fn(CRUMBS_DEFAULT_QUERY_DELAY_US);

    rc = crumbs_controller_read(dev->ctx, dev->addr, &reply, dev->read_fn, dev->io);
    if (rc != 0)
        return rc;

    if (reply.type_id != DCMT_TYPE_ID || reply.opcode != BREAD_OP_GET_WATCHDOG)
        return -1;

    return bread_watchdog_parse_payload(reply.data, reply.data_len, out);
}

#ifdef __cplusplus
}
#endif

#endif // DCMT_OPS_H

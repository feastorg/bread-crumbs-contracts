#ifndef DCMT_OPS_H
#define DCMT_OPS_H

#include "crumbs.h"
#include "crumbs_message_helpers.h"
#include "bread_caps.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define DCMT_TYPE_ID 0x02

#define DCMT_MODULE_VER_MAJOR 1
#define DCMT_MODULE_VER_MINOR 0
#define DCMT_MODULE_VER_PATCH 0

#define DCMT_OP_SET_OPEN_LOOP 0x01
#define DCMT_OP_SET_BRAKE 0x02
#define DCMT_OP_SET_MODE 0x03
#define DCMT_OP_SET_SETPOINT 0x04
#define DCMT_OP_SET_PID 0x05

#define DCMT_OP_GET_STATE 0x80

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

static inline int dcmt_send_set_open_loop(const crumbs_device_t *dev, int16_t m1_pwm, int16_t m2_pwm)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_OPEN_LOOP);
    crumbs_msg_add_i16(&msg, m1_pwm);
    crumbs_msg_add_i16(&msg, m2_pwm);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_brake(const crumbs_device_t *dev, uint8_t m1_brake, uint8_t m2_brake)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_BRAKE);
    crumbs_msg_add_u8(&msg, m1_brake);
    crumbs_msg_add_u8(&msg, m2_brake);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_mode(const crumbs_device_t *dev, uint8_t mode)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_MODE);
    crumbs_msg_add_u8(&msg, mode);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_setpoint(const crumbs_device_t *dev, int16_t target1, int16_t target2)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_SETPOINT);
    crumbs_msg_add_i16(&msg, target1);
    crumbs_msg_add_i16(&msg, target2);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int dcmt_send_set_pid(const crumbs_device_t *dev,
                                    uint8_t kp1_x10, uint8_t ki1_x10, uint8_t kd1_x10,
                                    uint8_t kp2_x10, uint8_t ki2_x10, uint8_t kd2_x10)
{
    crumbs_message_t msg;
    if (dcmt_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, DCMT_TYPE_ID, DCMT_OP_SET_PID);
    crumbs_msg_add_u8(&msg, kp1_x10);
    crumbs_msg_add_u8(&msg, ki1_x10);
    crumbs_msg_add_u8(&msg, kd1_x10);
    crumbs_msg_add_u8(&msg, kp2_x10);
    crumbs_msg_add_u8(&msg, ki2_x10);
    crumbs_msg_add_u8(&msg, kd2_x10);
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

typedef struct
{
    uint16_t crumbs_version;
    uint8_t fw_major;
    uint8_t fw_minor;
    uint8_t fw_patch;
} dcmt_version_result_t;

typedef struct
{
    uint8_t  mode;    /* DCMT_MODE_* */
    int16_t  m1_pwm;  /* current PWM output, always valid */
    int16_t  m2_pwm;
    int16_t  sp1;     /* active setpoint; BREAD_INVALID_I16 in OPEN_LOOP */
    int16_t  sp2;
    int16_t  pos1;    /* encoder position; always populated */
    int16_t  pos2;
    int16_t  spd1;    /* tachometer speed; BREAD_INVALID_I16 unless CLOSED_SPEED */
    int16_t  spd2;
    uint8_t  brakes;
    uint8_t  estop;
} dcmt_state_result_t;

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
 */
static inline int dcmt_parse_state_payload(const uint8_t *data, uint8_t data_len, dcmt_state_result_t *out)
{
    int rc;

    if (!data || !out)
        return -1;

    if (data_len != DCMT_STATE_FIXED_LEN)
        return -1;

    rc = crumbs_msg_read_u8(data, data_len, DCMT_STATE_OFF_MODE, &out->mode);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(data, data_len, DCMT_STATE_OFF_M1_PWM, &out->m1_pwm);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(data, data_len, DCMT_STATE_OFF_M2_PWM, &out->m2_pwm);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(data, data_len, DCMT_STATE_OFF_SP1, &out->sp1);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(data, data_len, DCMT_STATE_OFF_SP2, &out->sp2);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(data, data_len, DCMT_STATE_OFF_POS1, &out->pos1);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(data, data_len, DCMT_STATE_OFF_POS2, &out->pos2);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(data, data_len, DCMT_STATE_OFF_SPD1, &out->spd1);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(data, data_len, DCMT_STATE_OFF_SPD2, &out->spd2);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u8(data, data_len, DCMT_STATE_OFF_BRAKES, &out->brakes);
    if (rc != 0)
        return rc;
    return crumbs_msg_read_u8(data, data_len, DCMT_STATE_OFF_ESTOP, &out->estop);
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

#ifdef __cplusplus
}
#endif

#endif // DCMT_OPS_H

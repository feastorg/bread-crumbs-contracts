#ifndef RLHT_OPS_H
#define RLHT_OPS_H

#include "crumbs.h"
#include "crumbs_message_helpers.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define RLHT_TYPE_ID 0x01

#define RLHT_MODULE_VER_MAJOR 1
#define RLHT_MODULE_VER_MINOR 0
#define RLHT_MODULE_VER_PATCH 0

#define RLHT_MODE_CLOSED_LOOP 0x00
#define RLHT_MODE_OPEN_LOOP 0x01

#define RLHT_OP_SET_MODE 0x01
#define RLHT_OP_SET_SETPOINTS 0x02
#define RLHT_OP_SET_PID 0x03
#define RLHT_OP_SET_PERIODS 0x04
#define RLHT_OP_SET_TC_SELECT 0x05
#define RLHT_OP_SET_OPEN_DUTY 0x06

#define RLHT_OP_GET_STATE 0x80

#define RLHT_FLAG_ESTOP 0x01
#define RLHT_FLAG_RELAY1_ON 0x02
#define RLHT_FLAG_RELAY2_ON 0x04

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

static inline int rlht_send_set_mode(const crumbs_device_t *dev, uint8_t mode)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_MODE);
    crumbs_msg_add_u8(&msg, mode);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_setpoints(const crumbs_device_t *dev, int16_t sp1_deci_c, int16_t sp2_deci_c)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_SETPOINTS);
    crumbs_msg_add_i16(&msg, sp1_deci_c);
    crumbs_msg_add_i16(&msg, sp2_deci_c);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_pid_x10(const crumbs_device_t *dev,
                                        uint8_t kp1_x10, uint8_t ki1_x10, uint8_t kd1_x10,
                                        uint8_t kp2_x10, uint8_t ki2_x10, uint8_t kd2_x10)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_PID);
    crumbs_msg_add_u8(&msg, kp1_x10);
    crumbs_msg_add_u8(&msg, ki1_x10);
    crumbs_msg_add_u8(&msg, kd1_x10);
    crumbs_msg_add_u8(&msg, kp2_x10);
    crumbs_msg_add_u8(&msg, ki2_x10);
    crumbs_msg_add_u8(&msg, kd2_x10);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_periods(const crumbs_device_t *dev, uint16_t p1_ms, uint16_t p2_ms)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_PERIODS);
    crumbs_msg_add_u16(&msg, p1_ms);
    crumbs_msg_add_u16(&msg, p2_ms);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_tc_select(const crumbs_device_t *dev, uint8_t tc1, uint8_t tc2)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_TC_SELECT);
    crumbs_msg_add_u8(&msg, tc1);
    crumbs_msg_add_u8(&msg, tc2);
    return crumbs_controller_send(dev->ctx, dev->addr, &msg, dev->write_fn, dev->io);
}

static inline int rlht_send_set_open_duty(const crumbs_device_t *dev, uint8_t duty1_pct, uint8_t duty2_pct)
{
    crumbs_message_t msg;
    if (rlht_validate_write_device(dev) != 0)
        return -1;
    crumbs_msg_init(&msg, RLHT_TYPE_ID, RLHT_OP_SET_OPEN_DUTY);
    crumbs_msg_add_u8(&msg, duty1_pct);
    crumbs_msg_add_u8(&msg, duty2_pct);
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

    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 0, &out->mode);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 1, &out->flags);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 2, &out->t1_deci_c);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 4, &out->t2_deci_c);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 6, &out->sp1_deci_c);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 8, &out->sp2_deci_c);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u16(reply.data, reply.data_len, 10, &out->on1_ms);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u16(reply.data, reply.data_len, 12, &out->on2_ms);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u16(reply.data, reply.data_len, 14, &out->period1_ms);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u16(reply.data, reply.data_len, 16, &out->period2_ms);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 18, &out->tc_select);
    if (rc != 0)
        return rc;

    out->tc1 = (uint8_t)(out->tc_select & 0x03);
    out->tc2 = (uint8_t)((out->tc_select >> 2) & 0x03);
    return 0;
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

#ifdef __cplusplus
}
#endif

#endif // RLHT_OPS_H

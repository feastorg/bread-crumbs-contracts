#ifndef DCMT_OPS_H
#define DCMT_OPS_H

#include "crumbs.h"
#include "crumbs_message_helpers.h"

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

#define DCMT_MODE_OPEN_LOOP 0x00
#define DCMT_MODE_CLOSED_POSITION 0x01
#define DCMT_MODE_CLOSED_SPEED 0x02

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

typedef struct
{
    uint16_t crumbs_version;
    uint8_t fw_major;
    uint8_t fw_minor;
    uint8_t fw_patch;
} dcmt_version_result_t;

typedef struct
{
    uint8_t mode;
    int16_t target1;
    int16_t target2;
    int16_t value1;
    int16_t value2;
    uint8_t brakes;
    uint8_t estop;
} dcmt_state_result_t;

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

    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 0, &out->mode);
    if (rc != 0)
        return rc;
    if (out->mode == DCMT_MODE_OPEN_LOOP)
    {
        rc = crumbs_msg_read_i16(reply.data, reply.data_len, 1, &out->target1);
        if (rc != 0)
            return rc;
        rc = crumbs_msg_read_i16(reply.data, reply.data_len, 3, &out->target2);
        if (rc != 0)
            return rc;
        out->value1 = out->target1;
        out->value2 = out->target2;
        rc = crumbs_msg_read_u8(reply.data, reply.data_len, 5, &out->brakes);
        if (rc != 0)
            return rc;
        return crumbs_msg_read_u8(reply.data, reply.data_len, 6, &out->estop);
    }

    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 1, &out->target1);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 3, &out->target2);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 5, &out->value1);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_i16(reply.data, reply.data_len, 7, &out->value2);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u8(reply.data, reply.data_len, 9, &out->brakes);
    if (rc != 0)
        return rc;
    return crumbs_msg_read_u8(reply.data, reply.data_len, 10, &out->estop);
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

#ifdef __cplusplus
}
#endif

#endif // DCMT_OPS_H

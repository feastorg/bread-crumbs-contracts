#ifndef BREAD_CAPS_H
#define BREAD_CAPS_H

#include <stdint.h>

#include "crumbs.h"
#include "crumbs_message_helpers.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Protocol-wide sentinel values for fixed-layout fields that are not applicable
 * in the current operational mode or build configuration.
 *
 * Firmware emits these instead of zero so controllers can distinguish
 * "measured/commanded zero" from "field not populated in this mode".
 *
 * INT16_MIN (-32768 / 0x8000) is chosen because it lies outside realistic
 * operational ranges for position ticks, speed RPM, and temperature deci-C,
 * and is the conventional "not available" indicator in embedded binary protocols
 * (CAN/CANopen, J1939-style profiles).
 *
 * Use BREAD_IS_VALID_I16() / BREAD_IS_VALID_U8() before consuming a field.
 */
#define BREAD_INVALID_I16 ((int16_t)(-32768))
#define BREAD_INVALID_U8  ((uint8_t)(0xFF))

#define BREAD_IS_VALID_I16(v) ((int16_t)(v) != BREAD_INVALID_I16)
#define BREAD_IS_VALID_U8(v)  ((uint8_t)(v) != BREAD_INVALID_U8)

/* Shared capability query opcode across BREADS-compatible Slices. */
#define BREAD_OP_GET_CAPS 0x7F

/* Capability payload schema version. */
#define BREAD_CAPS_SCHEMA_V1 0x01

/* v1 payload: [schema:u8][level:u8][flags:u32] */
#define BREAD_CAPS_V1_PAYLOAD_LEN 6

typedef struct
{
    uint8_t schema;
    uint8_t level;
    uint32_t flags;
} bread_caps_result_t;

static inline int bread_caps_parse_payload(const uint8_t *data, uint8_t len, bread_caps_result_t *out)
{
    if (!data || !out)
        return -1;
    if (len < BREAD_CAPS_V1_PAYLOAD_LEN)
        return -1;

    out->schema = data[0];
    out->level = data[1];
    return crumbs_msg_read_u32(data, len, 2, &out->flags);
}

static inline int bread_caps_build_reply(crumbs_message_t *reply, uint8_t type_id, uint8_t level, uint32_t flags)
{
    if (!reply)
        return -1;

    crumbs_msg_init(reply, type_id, BREAD_OP_GET_CAPS);
    if (crumbs_msg_add_u8(reply, BREAD_CAPS_SCHEMA_V1) != 0)
        return -1;
    if (crumbs_msg_add_u8(reply, level) != 0)
        return -1;
    return crumbs_msg_add_u32(reply, flags);
}

#ifdef __cplusplus
}
#endif

#endif /* BREAD_CAPS_H */

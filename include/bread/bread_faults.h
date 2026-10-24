#ifndef BREAD_FAULTS_H
#define BREAD_FAULTS_H

#include <stdint.h>

#include "crumbs.h"
#include "crumbs_message_helpers.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Latched faults, shared across BREADS-compatible Slices.
 *
 * A fault is a condition the firmware detects on one channel (a stalled
 * motor, an over-temperature heater) and latches until BREAD_OP_CLEAR_FAULTS,
 * a deliberate local operator command on the slice (firmware-defined), or a
 * reboot clears it. While any fault is latched, the slice holds its safe
 * state on every channel and refuses a command that could start motion or
 * heat with BREAD_REFUSE_FAULT. Clearing resumes nothing.
 *
 * Which conditions exist and what each bit means is per family
 * (DCMT_FAULT_*, RLHT_FAULT_*). Support is advertised per type
 * (DCMT_CAP_FAULTS, RLHT_CAP_FAULTS); a controller must not send
 * CLEAR_FAULTS or query GET_FAULTS unless the flag is present.
 */
#define BREAD_OP_CLEAR_FAULTS 0x7B
#define BREAD_OP_GET_FAULTS 0x7A

/* CLEAR_FAULTS payload: empty. Clears every latched fault of the slice and
   nothing else. Firmware rejects a frame with a non-empty payload and
   leaves the faults latched. Like CLEAR_WATCHDOG_TRIP, it is an operator's
   acknowledgement; a controller must not send it on its own initiative. */
#define BREAD_FAULTS_CLEAR_PAYLOAD_LEN 0

/* GET_FAULTS payload layout (1 byte): [faults:u8], one bit per latched
   fault, per family. The parser reads the prefix and ignores trailing
   bytes, so a family can extend the reply. */
#define BREAD_FAULTS_OFF_FAULTS 0
#define BREAD_FAULTS_FIXED_LEN 1

typedef struct
{
    uint8_t faults;
} bread_faults_result_t;

static inline int bread_faults_parse_payload(const uint8_t *data, uint8_t data_len, bread_faults_result_t *out)
{
    if (!data || !out)
        return -1;
    if (data_len < BREAD_FAULTS_FIXED_LEN)
        return -1;
    return crumbs_msg_read_u8(data, data_len, BREAD_FAULTS_OFF_FAULTS, &out->faults);
}

static inline int bread_faults_build_reply(crumbs_message_t *reply, uint8_t type_id, uint8_t faults)
{
    if (!reply)
        return -1;

    crumbs_msg_init(reply, type_id, BREAD_OP_GET_FAULTS);
    return crumbs_msg_add_u8(reply, faults);
}

#ifdef __cplusplus
}
#endif

#endif /* BREAD_FAULTS_H */

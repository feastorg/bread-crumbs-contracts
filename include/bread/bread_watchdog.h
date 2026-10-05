#ifndef BREAD_WATCHDOG_H
#define BREAD_WATCHDOG_H

#include <stdint.h>

#include "crumbs.h"
#include "crumbs_message_helpers.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Shared bus-liveness command-watchdog ops across BREADS-compatible Slices.
 *
 * Firmware boots disarmed; a controller arms it with SET_WATCHDOG. While
 * armed, any valid inbound frame or reply build refreshes it; expiry drives
 * the slice's actuators to their safe state (motors braked, heaters off)
 * without touching estop. Standalone/serial use never arms it and sees no
 * behavior change.
 *
 * Support is advertised per type via a capability flag (e.g.
 * DCMT_CAP_CMD_WATCHDOG); controllers must not send SET_WATCHDOG unless the
 * flag is present.
 *
 * What clears a trip depends on the firmware. On firmware that also
 * advertises the per-type CLEAR_WATCHDOG_TRIP capability
 * (DCMT_CAP_CLEAR_WATCHDOG_TRIP, RLHT_CAP_CLEAR_WATCHDOG_TRIP), the trip
 * latches: only CLEAR_WATCHDOG_TRIP, a deliberate local operator command on
 * the slice (firmware-defined, not any serial input), or a reboot clears
 * it. SET_WATCHDOG re-arms without clearing, and ordinary frames refresh
 * liveness without clearing. A controller that predates the flag has no
 * remote way to clear a trip on such firmware.
 *
 * On firmware without that flag, SET_WATCHDOG and any other valid command
 * frame (not a SET_REPLY-staged query such as GET_STATE or GET_WATCHDOG)
 * clear the trip. Such firmware has no CLEAR_WATCHDOG_TRIP handler, but the
 * frame is still a command frame and so clears the trip too, which is why a
 * controller must not send CLEAR_WATCHDOG_TRIP without the flag.
 */
#define BREAD_OP_SET_WATCHDOG 0x7E
#define BREAD_OP_GET_WATCHDOG 0x7D
#define BREAD_OP_CLEAR_WATCHDOG_TRIP 0x7C

/* SET_WATCHDOG payload (2 bytes): [timeout_ms:u16], 0 = disarm. */
#define BREAD_WATCHDOG_SET_PAYLOAD_LEN 2

/*
 * CLEAR_WATCHDOG_TRIP payload: empty. Clears `tripped` and nothing else:
 * it does not arm, disarm or change the timeout, and does not reset
 * trip_count (like any valid command frame, it refreshes liveness).
 * Firmware rejects a CLEAR_WATCHDOG_TRIP frame with a non-empty payload and
 * leaves `tripped` set. It is an operator's acknowledgement that releasing
 * the hold is safe; a controller must not send it on its own initiative
 * (startup, recovery, re-arm).
 */
#define BREAD_WATCHDOG_CLEAR_TRIP_PAYLOAD_LEN 0

/*
 * GET_WATCHDOG payload layout (5 bytes):
 * [armed:u8][timeout_ms:u16][tripped:u8][trip_count:u8]
 *   tripped    -> watchdog has expired since the last clear. With the
 *                 per-type CLEAR_WATCHDOG_TRIP capability it clears only on
 *                 CLEAR_WATCHDOG_TRIP, a deliberate local operator
 *                 command, or reboot; without it, the next valid command
 *                 frame (SET_WATCHDOG included) clears it
 *   trip_count -> cumulative trips since boot (never cleared)
 */
#define BREAD_WATCHDOG_OFF_ARMED 0
#define BREAD_WATCHDOG_OFF_TIMEOUT_MS 1
#define BREAD_WATCHDOG_OFF_TRIPPED 3
#define BREAD_WATCHDOG_OFF_TRIP_COUNT 4
#define BREAD_WATCHDOG_FIXED_LEN 5

typedef struct
{
    uint8_t armed;
    uint16_t timeout_ms;
    uint8_t tripped;
    uint8_t trip_count;
} bread_watchdog_result_t;

static inline int bread_watchdog_parse_payload(const uint8_t *data, uint8_t data_len, bread_watchdog_result_t *out)
{
    int rc;

    if (!data || !out)
        return -1;

    if (data_len != BREAD_WATCHDOG_FIXED_LEN)
        return -1;

    rc = crumbs_msg_read_u8(data, data_len, BREAD_WATCHDOG_OFF_ARMED, &out->armed);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u16(data, data_len, BREAD_WATCHDOG_OFF_TIMEOUT_MS, &out->timeout_ms);
    if (rc != 0)
        return rc;
    rc = crumbs_msg_read_u8(data, data_len, BREAD_WATCHDOG_OFF_TRIPPED, &out->tripped);
    if (rc != 0)
        return rc;
    return crumbs_msg_read_u8(data, data_len, BREAD_WATCHDOG_OFF_TRIP_COUNT, &out->trip_count);
}

static inline int bread_watchdog_build_reply(crumbs_message_t *reply, uint8_t type_id,
                                             uint8_t armed, uint16_t timeout_ms,
                                             uint8_t tripped, uint8_t trip_count)
{
    if (!reply)
        return -1;

    crumbs_msg_init(reply, type_id, BREAD_OP_GET_WATCHDOG);
    if (crumbs_msg_add_u8(reply, armed) != 0)
        return -1;
    if (crumbs_msg_add_u16(reply, timeout_ms) != 0)
        return -1;
    if (crumbs_msg_add_u8(reply, tripped) != 0)
        return -1;
    return crumbs_msg_add_u8(reply, trip_count);
}

#ifdef __cplusplus
}
#endif

#endif /* BREAD_WATCHDOG_H */

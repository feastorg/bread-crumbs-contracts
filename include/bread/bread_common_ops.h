#ifndef BREAD_COMMON_OPS_H
#define BREAD_COMMON_OPS_H

#include <stdint.h>

#include "crumbs_ops.h"
#include "bread_caps.h"
#include "bread_watchdog.h"

/*
 * The opcodes every BREADS-compatible Slice implements, whatever its
 * family. CRUMBS_DEFINE_FAMILY checks a family's own opcodes against each
 * other only, so a family opcode equal to one of these would build and
 * collide with a common operation: one handler registration would silently
 * replace the other. BREAD_CHECK_FAMILY_OPS() turns that into a build error.
 */
#define BREAD_COMMON_OPS(X)                                         \
    X(BREAD_OP_CLEAR_WATCHDOG_TRIP, BREAD_OP_CLEAR_WATCHDOG_TRIP)   \
    X(BREAD_OP_GET_WATCHDOG, BREAD_OP_GET_WATCHDOG)                 \
    X(BREAD_OP_SET_WATCHDOG, BREAD_OP_SET_WATCHDOG)                 \
    X(BREAD_OP_GET_CAPS, BREAD_OP_GET_CAPS)

#define BREAD_OP_CASE_(name, value) \
    case (value):                   \
        break;

/*
 * Call after CRUMBS_DEFINE_FAMILY(PREFIX, type_id, OPS): the build fails
 * with a duplicate case value if an opcode in OPS equals a common opcode.
 * Like the CRUMBS check, it is an unused static inline function, marked
 * with the same CRUMBS_UNUSED_FN_, whose switch lists both sets of
 * opcodes.
 */
#define BREAD_CHECK_FAMILY_OPS(PREFIX, OPS)                                         \
    CRUMBS_UNUSED_FN_ static inline void PREFIX##_opcodes_avoid_common_(uint8_t op) \
    {                                                                               \
        switch (op)                                                                 \
        {                                                                           \
            OPS(BREAD_OP_CASE_)                                                     \
            BREAD_COMMON_OPS(BREAD_OP_CASE_)                                        \
        default:                                                                    \
            break;                                                                  \
        }                                                                           \
    }

#endif /* BREAD_COMMON_OPS_H */

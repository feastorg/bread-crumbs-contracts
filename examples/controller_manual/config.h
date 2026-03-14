#ifndef BREAD_MANUAL_CONFIG_H
#define BREAD_MANUAL_CONFIG_H

#include <stdint.h>
#include <bread/bread_ops.h>

typedef struct
{
    uint8_t type_id;
    uint8_t addr;
} bread_device_config_t;

/*
 * Default lab mapping:
 * - RLHT: 0x0A
 * - Legacy DCMT address: 0x15
 *
 * Alternate DCMT address is often 0x19.
 */
static const bread_device_config_t DEVICE_CONFIG[] = {
    {RLHT_TYPE_ID, 0x0A},
    {DCMT_TYPE_ID, 0x15},
    /* {DCMT_TYPE_ID, 0x19}, */
};

#define DEVICE_CONFIG_COUNT (sizeof(DEVICE_CONFIG) / sizeof(DEVICE_CONFIG[0]))

#endif /* BREAD_MANUAL_CONFIG_H */

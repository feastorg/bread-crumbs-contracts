#ifndef BREAD_VERSION_HELPERS_H
#define BREAD_VERSION_HELPERS_H

#include "crumbs_version.h"
#include <stdint.h>

#ifndef BREAD_MIN_CRUMBS_VERSION
#define BREAD_MIN_CRUMBS_VERSION 1100
#endif

/* Parse opcode 0x00 version payload:
 * [CRUMBS_VERSION:u16][module_major:u8][module_minor:u8][module_patch:u8]
 */
static inline int bread_parse_version(const uint8_t *data, uint8_t len,
                                      uint16_t *crumbs_ver,
                                      uint8_t *mod_major,
                                      uint8_t *mod_minor,
                                      uint8_t *mod_patch)
{
    if (!data || len < 5)
        return -1;

    if (crumbs_ver)
        *crumbs_ver = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
    if (mod_major)
        *mod_major = data[2];
    if (mod_minor)
        *mod_minor = data[3];
    if (mod_patch)
        *mod_patch = data[4];
    return 0;
}

/* Compatible when peripheral CRUMBS version is at or above minimum. */
static inline int bread_check_crumbs_compat(uint16_t peripheral_ver)
{
    return (peripheral_ver >= (uint16_t)BREAD_MIN_CRUMBS_VERSION) ? 0 : -1;
}

/* Module compatibility:
 * - MAJOR must match
 * - peripheral MINOR must be >= expected MINOR
 * - PATCH ignored
 */
static inline int bread_check_module_compat(uint8_t peri_major, uint8_t peri_minor,
                                            uint8_t expect_major, uint8_t expect_minor)
{
    if (peri_major != expect_major)
        return -1;
    if (peri_minor < expect_minor)
        return -2;
    return 0;
}

#endif /* BREAD_VERSION_HELPERS_H */

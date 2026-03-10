#ifndef BREAD_OPS_H
#define BREAD_OPS_H

/**
 * @file bread_ops.h
 * @brief Family convenience header for all BREAD CRUMBS contracts.
 *
 * Include this header in controllers to access all current BREAD device
 * operation contracts and shared version compatibility helpers.
 */

#include "dcmt_ops.h"
#include "rlht_ops.h"
#include "crumbs_version.h"

#ifndef BREAD_MIN_CRUMBS_VERSION
#define BREAD_MIN_CRUMBS_VERSION 1100
#endif

/**
 * @brief Parse opcode 0x00 version payload.
 *
 * Payload layout:
 * [CRUMBS_VERSION:u16][module_major:u8][module_minor:u8][module_patch:u8]
 *
 * @return 0 on success, -1 on invalid payload.
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

/**
 * @brief Check CRUMBS core compatibility against minimum supported version.
 *
 * Override BREAD_MIN_CRUMBS_VERSION before including this header to tighten
 * or relax the accepted CRUMBS floor in a specific controller.
 */
static inline int bread_check_crumbs_compat(uint16_t peripheral_ver)
{
    return (peripheral_ver >= (uint16_t)BREAD_MIN_CRUMBS_VERSION) ? 0 : -1;
}

/**
 * @brief Check module semver compatibility.
 *
 * Rules:
 * - MAJOR must match exactly.
 * - Peripheral MINOR must be >= controller expected MINOR.
 * - PATCH is ignored.
 *
 * @return 0 if compatible, -1 on major mismatch, -2 on minor too old.
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

#endif /* BREAD_OPS_H */

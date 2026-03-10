#include <bread/bread_ops.h>

int bread_contracts_smoke(void)
{
    int ok = 0;

    ok += (RLHT_TYPE_ID == 0x01) ? 0 : 1;
    ok += (DCMT_TYPE_ID == 0x02) ? 0 : 1;

    ok += (bread_check_crumbs_compat((uint16_t)BREAD_MIN_CRUMBS_VERSION) == 0) ? 0 : 1;
    ok += (bread_check_module_compat(1, 2, 1, 1) == 0) ? 0 : 1;
    ok += (bread_check_module_compat(2, 0, 1, 0) == -1) ? 0 : 1;

    return ok;
}

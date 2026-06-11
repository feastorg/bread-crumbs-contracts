#include <bread/bread_ops.h>

int main(void)
{
    int ok = 0;

    ok += (RLHT_TYPE_ID == 0x01) ? 0 : 1;
    ok += (DCMT_TYPE_ID == 0x02) ? 0 : 1;
    ok += (BREAD_OP_GET_CAPS == 0x7F) ? 0 : 1;
    ok += (DCMT_STATE_FIXED_LEN == 19) ? 0 : 1;
    ok += BREAD_IS_VALID_I16(BREAD_INVALID_I16) ? 1 : 0;
    ok += (bread_check_crumbs_compat((uint16_t)BREAD_MIN_CRUMBS_VERSION) == 0) ? 0 : 1;

    return ok;
}

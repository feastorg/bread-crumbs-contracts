#include <bread/bread_ops.h>

int bread_contracts_smoke(void)
{
    int ok = 0;

    ok += (RLHT_TYPE_ID == 0x01) ? 0 : 1;
    ok += (DCMT_TYPE_ID == 0x02) ? 0 : 1;
    ok += (BREAD_OP_GET_CAPS == 0x7F) ? 0 : 1;
    ok += (DCMT_CAP_BASELINE_FLAGS != 0u) ? 0 : 1;
    ok += (RLHT_CAP_BASELINE_FLAGS != 0u) ? 0 : 1;

    ok += (bread_check_crumbs_compat((uint16_t)BREAD_MIN_CRUMBS_VERSION) == 0) ? 0 : 1;
    ok += (bread_check_module_compat(1, 2, 1, 1) == 0) ? 0 : 1;
    ok += (bread_check_module_compat(2, 0, 1, 0) == -1) ? 0 : 1;

    /* State payload parsers: valid fixed-layout payloads round-trip; short
       or NULL payloads are rejected. */
    {
        uint8_t dcmt_payload[DCMT_STATE_FIXED_LEN] = {0};
        dcmt_state_result_t dcmt_state;
        dcmt_payload[DCMT_STATE_OFF_MODE] = DCMT_MODE_OPEN_LOOP;
        dcmt_payload[DCMT_STATE_OFF_M1_PWM] = 0x2A; /* 42 LE */
        dcmt_payload[DCMT_STATE_OFF_BRAKES] = 0x01;
        ok += (dcmt_parse_state_payload(dcmt_payload, DCMT_STATE_FIXED_LEN, &dcmt_state) == 0) ? 0 : 1;
        ok += (dcmt_state.mode == DCMT_MODE_OPEN_LOOP) ? 0 : 1;
        ok += (dcmt_state.m1_pwm == 42) ? 0 : 1;
        ok += (dcmt_state.brakes == 0x01) ? 0 : 1;
        ok += (dcmt_parse_state_payload(dcmt_payload, DCMT_STATE_FIXED_LEN - 1, &dcmt_state) == -1) ? 0 : 1;
        ok += (dcmt_parse_state_payload(NULL, DCMT_STATE_FIXED_LEN, &dcmt_state) == -1) ? 0 : 1;
    }
    {
        uint8_t rlht_payload[19] = {0};
        rlht_state_result_t rlht_state;
        rlht_payload[0] = RLHT_MODE_CLOSED_LOOP;
        rlht_payload[1] = RLHT_FLAG_RELAY1_ON;
        rlht_payload[2] = 0xF4; /* t1 = 500 deci-C LE */
        rlht_payload[3] = 0x01;
        rlht_payload[18] = 0x06; /* tc_select: tc1=2, tc2=1 */
        ok += (rlht_parse_state_payload(rlht_payload, sizeof(rlht_payload), &rlht_state) == 0) ? 0 : 1;
        ok += (rlht_state.flags == RLHT_FLAG_RELAY1_ON) ? 0 : 1;
        ok += (rlht_state.t1_deci_c == 500) ? 0 : 1;
        ok += (rlht_state.tc1 == 2 && rlht_state.tc2 == 1) ? 0 : 1;
        ok += (rlht_parse_state_payload(rlht_payload, 18, &rlht_state) != 0) ? 0 : 1;
        ok += (rlht_parse_state_payload(NULL, sizeof(rlht_payload), &rlht_state) == -1) ? 0 : 1;
    }

    return ok;
}

int main(void)
{
    return bread_contracts_smoke();
}

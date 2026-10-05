/* Every RLHT payload round-trips through the family's own codec: each SET
 * goes out through its controller wrapper, is decoded off the fake bus,
 * compared with the bytes the layout comment states and unpacked with the
 * unpack the Slice uses; the GET_STATE reply is packed with the pack the
 * Slice uses, served on the bus and read back through rlht_get_state(). */
#include <bread/rlht_ops.h>

#include "fake_bus.h"

static void test_set_mode(void)
{
    static const uint8_t wire[] = {RLHT_MODE_OPEN_LOOP};
    rlht_set_mode_t rx = {0};

    fake_bus_bind();
    CHECK(rlht_send_set_mode(&g_dev, RLHT_MODE_OPEN_LOOP) == 0, "send set_mode");
    CHECK(fake_bus_sent(RLHT_TYPE_ID, RLHT_OP_SET_MODE, wire, rlht_set_mode_wire_size), "set_mode bytes");
    CHECK(rlht_set_mode_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0, "unpack set_mode");
    CHECK(rx.mode == RLHT_MODE_OPEN_LOOP, "mode");
}

static void test_set_setpoints(void)
{
    /* [sp1:i16 LE][sp2:i16 LE] */
    static const uint8_t wire[] = {0x06, 0xFF, 0x00, 0x80};
    rlht_set_setpoints_t rx = {0};

    fake_bus_bind();
    CHECK(rlht_send_set_setpoints(&g_dev, -250, -32768) == 0, "send set_setpoints");
    CHECK(fake_bus_sent(RLHT_TYPE_ID, RLHT_OP_SET_SETPOINTS, wire, rlht_set_setpoints_wire_size),
          "set_setpoints bytes");
    CHECK(rlht_set_setpoints_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0,
          "unpack set_setpoints");
    CHECK(rx.sp1_deci_c == -250, "sp1");
    CHECK(rx.sp2_deci_c == -32768, "sp2");
}

static void test_set_pid(void)
{
    static const uint8_t wire[] = {1, 2, 3, 0x80, 0xFE, 0xFF};
    rlht_set_pid_t rx = {0};

    fake_bus_bind();
    CHECK(rlht_send_set_pid_x10(&g_dev, 1, 2, 3, 0x80, 0xFE, 0xFF) == 0, "send set_pid");
    CHECK(fake_bus_sent(RLHT_TYPE_ID, RLHT_OP_SET_PID, wire, rlht_set_pid_wire_size), "set_pid bytes");
    CHECK(rlht_set_pid_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0, "unpack set_pid");
    CHECK(rx.kp1_x10 == 1 && rx.ki1_x10 == 2 && rx.kd1_x10 == 3, "channel 1 gains");
    CHECK(rx.kp2_x10 == 0x80 && rx.ki2_x10 == 0xFE && rx.kd2_x10 == 0xFF, "channel 2 gains");
}

static void test_set_periods(void)
{
    /* [p1_ms:u16 LE][p2_ms:u16 LE] */
    static const uint8_t wire[] = {0x40, 0x9C, 0xFF, 0xFF};
    rlht_set_periods_t rx = {0};

    fake_bus_bind();
    CHECK(rlht_send_set_periods(&g_dev, 40000, 65535) == 0, "send set_periods");
    CHECK(fake_bus_sent(RLHT_TYPE_ID, RLHT_OP_SET_PERIODS, wire, rlht_set_periods_wire_size),
          "set_periods bytes");
    CHECK(rlht_set_periods_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0,
          "unpack set_periods");
    CHECK(rx.p1_ms == 40000, "p1_ms");
    CHECK(rx.p2_ms == 65535, "p2_ms");
}

static void test_set_tc_select(void)
{
    static const uint8_t wire[] = {2, 1};
    rlht_set_tc_select_t rx = {0};

    fake_bus_bind();
    CHECK(rlht_send_set_tc_select(&g_dev, 2, 1) == 0, "send set_tc_select");
    CHECK(fake_bus_sent(RLHT_TYPE_ID, RLHT_OP_SET_TC_SELECT, wire, rlht_set_tc_select_wire_size),
          "set_tc_select bytes");
    CHECK(rlht_set_tc_select_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0,
          "unpack set_tc_select");
    CHECK(rx.tc1 == 2 && rx.tc2 == 1, "tc1, tc2");
}

static void test_set_open_duty(void)
{
    static const uint8_t wire[] = {35, 100};
    rlht_set_open_duty_t rx = {0};

    fake_bus_bind();
    CHECK(rlht_send_set_open_duty(&g_dev, 35, 100) == 0, "send set_open_duty");
    CHECK(fake_bus_sent(RLHT_TYPE_ID, RLHT_OP_SET_OPEN_DUTY, wire, rlht_set_open_duty_wire_size),
          "set_open_duty bytes");
    CHECK(rlht_set_open_duty_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0,
          "unpack set_open_duty");
    CHECK(rx.duty1_pct == 35 && rx.duty2_pct == 100, "duty1, duty2");
}

static void test_clear_watchdog_trip(void)
{
    /* [type_id][opcode][data_len = 0][crc8]: no payload at all. */
    static const uint8_t header[] = {0x01, 0x7C, 0x00};
    static const uint8_t empty[1] = {0};

    fake_bus_bind();
    CHECK(rlht_send_clear_watchdog_trip(&g_dev) == 0, "send clear_watchdog_trip");
    CHECK(g_bus.writes == 1, "one frame written");
    CHECK(fake_bus_sent(RLHT_TYPE_ID, BREAD_OP_CLEAR_WATCHDOG_TRIP, empty, 0), "clear_watchdog_trip decodes");
    CHECK(g_bus.last_frame_len == sizeof header + 1 && memcmp(g_bus.last_frame, header, sizeof header) == 0,
          "clear_watchdog_trip frame bytes");
    CHECK(rlht_send_clear_watchdog_trip(NULL) == -1, "NULL device is refused");
}

static void test_get_state(void)
{
    /* Every field distinct, so a swapped or shifted field cannot pass. */
    static const uint8_t wire[19] = {
        0x01,       /* mode */
        0x06,       /* flags */
        0x0C, 0xFE, /* t1 = -500 */
        0xF6, 0xFF, /* t2 = -10 */
        0x06, 0xFF, /* sp1 = -250 */
        0x00, 0x80, /* sp2 = -32768 */
        0x40, 0x9C, /* on1 = 40000 */
        0x01, 0x80, /* on2 = 32769 */
        0xFF, 0xFF, /* period1 = 65535 */
        0x50, 0xC3, /* period2 = 50000 */
        0x06,       /* tc_select: tc1 = 2, tc2 = 1 */
    };
    rlht_state_t tx, back;
    rlht_state_result_t rx;
    crumbs_message_t reply;

    tx.mode = 0x01;
    tx.flags = RLHT_FLAG_RELAY1_ON | RLHT_FLAG_RELAY2_ON;
    tx.t1_deci_c = -500;
    tx.t2_deci_c = -10;
    tx.sp1_deci_c = -250;
    tx.sp2_deci_c = -32768;
    tx.on1_ms = 40000;
    tx.on2_ms = 32769;
    tx.period1_ms = 65535;
    tx.period2_ms = 50000;
    tx.tc_select = 0x06;

    fake_bus_bind();
    crumbs_msg_init(&reply, RLHT_TYPE_ID, RLHT_OP_GET_STATE);
    CHECK(rlht_state_pack(&reply, &tx) == 0, "pack state");
    CHECK(reply.data_len == sizeof wire && memcmp(reply.data, wire, sizeof wire) == 0,
          "state bytes, as the layout comment states");

    /* The Slice-side struct, unpacked directly: each member keeps the
       width and signedness of its field (compared with literals, since tx
       went through the same member types). */
    memset(&back, 0, sizeof back);
    CHECK(rlht_state_unpack(reply.data, reply.data_len, &back) == 0, "unpack state");
    CHECK(back.mode == 0x01 && back.flags == 0x06, "unpacked mode, flags");
    CHECK(back.t1_deci_c == -500 && back.t2_deci_c == -10, "unpacked t1, t2");
    CHECK(back.sp1_deci_c == -250 && back.sp2_deci_c == -32768, "unpacked sp1, sp2");
    CHECK(back.on1_ms == 40000u && back.on2_ms == 32769u, "unpacked on1, on2");
    CHECK(back.period1_ms == 65535u && back.period2_ms == 50000u, "unpacked period1, period2");
    CHECK(back.tc_select == 0x06, "unpacked tc_select");

    fake_bus_serve(&reply);

    memset(&rx, 0, sizeof rx);
    CHECK(rlht_get_state(&g_dev, &rx) == 0, "get state");
    CHECK(g_bus.last_write.type_id == 0 && g_bus.last_write.opcode == CRUMBS_CMD_SET_REPLY &&
              g_bus.last_write.data_len == 1 && g_bus.last_write.data[0] == RLHT_OP_GET_STATE,
          "the query selected GET_STATE");
    CHECK(rx.mode == 0x01, "mode");
    CHECK(rx.flags == (RLHT_FLAG_RELAY1_ON | RLHT_FLAG_RELAY2_ON), "flags");
    CHECK(rx.t1_deci_c == -500, "t1");
    CHECK(rx.t2_deci_c == -10, "t2");
    CHECK(rx.sp1_deci_c == -250, "sp1");
    CHECK(rx.sp2_deci_c == -32768, "sp2");
    CHECK(rx.on1_ms == 40000, "on1");
    CHECK(rx.on2_ms == 32769, "on2");
    CHECK(rx.period1_ms == 65535, "period1");
    CHECK(rx.period2_ms == 50000, "period2");
    CHECK(rx.tc_select == 0x06, "tc_select");
    CHECK(rx.tc1 == 2 && rx.tc2 == 1, "tc1, tc2 derived from tc_select");
}

static void test_state_length_rule(void)
{
    uint8_t buf[20];
    rlht_state_result_t rx;

    memset(buf, 0x11, sizeof buf);
    CHECK(rlht_parse_state_payload(buf, 18, &rx) == -1, "one byte short is refused");
    CHECK(rlht_parse_state_payload(buf, 19, &rx) == 0, "exact length is accepted");
    CHECK(rlht_parse_state_payload(buf, 20, &rx) == 0, "a trailing byte is ignored");
    CHECK(rlht_parse_state_payload(NULL, 19, &rx) == -1, "NULL payload is refused");
}

int main(void)
{
    test_set_mode();
    test_set_setpoints();
    test_set_pid();
    test_set_periods();
    test_set_tc_select();
    test_set_open_duty();
    test_clear_watchdog_trip();
    test_get_state();
    test_state_length_rule();
    if (g_failures)
    {
        printf("rlht_roundtrip: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("rlht_roundtrip: PASS\n");
    return 0;
}

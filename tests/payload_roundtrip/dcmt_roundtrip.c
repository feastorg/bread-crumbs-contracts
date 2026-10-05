/* Every DCMT payload round-trips through the family's own codec: each SET
 * goes out through its controller wrapper, is decoded off the fake bus,
 * compared with the bytes the layout comment states and unpacked with the
 * unpack the Slice uses; the GET_STATE reply is packed with the pack the
 * Slice uses, served on the bus and read back through dcmt_get_state(). */
#include <bread/dcmt_ops.h>

#include "fake_bus.h"

static void test_set_open_loop(void)
{
    /* [m1_pwm:i16 LE][m2_pwm:i16 LE] */
    static const uint8_t wire[] = {0x00, 0xFF, 0x01, 0xFF};
    dcmt_set_open_loop_t rx = {0};

    fake_bus_bind();
    CHECK(dcmt_send_set_open_loop(&g_dev, -256, -255) == 0, "send set_open_loop");
    CHECK(fake_bus_sent(DCMT_TYPE_ID, DCMT_OP_SET_OPEN_LOOP, wire, dcmt_set_open_loop_wire_size),
          "set_open_loop bytes");
    CHECK(dcmt_set_open_loop_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0,
          "unpack set_open_loop");
    CHECK(rx.m1_pwm == -256, "m1_pwm");
    CHECK(rx.m2_pwm == -255, "m2_pwm");
}

static void test_set_brake(void)
{
    static const uint8_t wire[] = {1, 0};
    dcmt_set_brake_t rx = {0};

    fake_bus_bind();
    CHECK(dcmt_send_set_brake(&g_dev, 1, 0) == 0, "send set_brake");
    CHECK(fake_bus_sent(DCMT_TYPE_ID, DCMT_OP_SET_BRAKE, wire, dcmt_set_brake_wire_size), "set_brake bytes");
    CHECK(dcmt_set_brake_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0, "unpack set_brake");
    CHECK(rx.m1_brake == 1 && rx.m2_brake == 0, "m1_brake, m2_brake");
}

static void test_set_mode(void)
{
    static const uint8_t wire[] = {DCMT_MODE_CLOSED_SPEED};
    dcmt_set_mode_t rx = {0};

    fake_bus_bind();
    CHECK(dcmt_send_set_mode(&g_dev, DCMT_MODE_CLOSED_SPEED) == 0, "send set_mode");
    CHECK(fake_bus_sent(DCMT_TYPE_ID, DCMT_OP_SET_MODE, wire, dcmt_set_mode_wire_size), "set_mode bytes");
    CHECK(dcmt_set_mode_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0, "unpack set_mode");
    CHECK(rx.mode == DCMT_MODE_CLOSED_SPEED, "mode");
}

static void test_set_setpoint(void)
{
    /* [target1:i16 LE][target2:i16 LE] */
    static const uint8_t wire[] = {0xCB, 0xED, 0xFE, 0xFF};
    dcmt_set_setpoint_t rx = {0};

    fake_bus_bind();
    CHECK(dcmt_send_set_setpoint(&g_dev, -4661, -2) == 0, "send set_setpoint");
    CHECK(fake_bus_sent(DCMT_TYPE_ID, DCMT_OP_SET_SETPOINT, wire, dcmt_set_setpoint_wire_size),
          "set_setpoint bytes");
    CHECK(dcmt_set_setpoint_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0,
          "unpack set_setpoint");
    CHECK(rx.target1 == -4661, "target1");
    CHECK(rx.target2 == -2, "target2");
}

static void test_set_pid(void)
{
    static const uint8_t wire[] = {10, 20, 30, 40, 50, 0xFF};
    dcmt_set_pid_t rx = {0};

    fake_bus_bind();
    CHECK(dcmt_send_set_pid(&g_dev, 10, 20, 30, 40, 50, 0xFF) == 0, "send set_pid");
    CHECK(fake_bus_sent(DCMT_TYPE_ID, DCMT_OP_SET_PID, wire, dcmt_set_pid_wire_size), "set_pid bytes");
    CHECK(dcmt_set_pid_unpack(g_bus.last_write.data, g_bus.last_write.data_len, &rx) == 0, "unpack set_pid");
    CHECK(rx.kp1_x10 == 10 && rx.ki1_x10 == 20 && rx.kd1_x10 == 30, "motor 1 gains");
    CHECK(rx.kp2_x10 == 40 && rx.ki2_x10 == 50 && rx.kd2_x10 == 0xFF, "motor 2 gains");
}

/* A state with every field distinct, so a swapped or shifted field cannot
   pass, and every i16 negative, so an unsigned member cannot either. */
static dcmt_state_t distinct_state(void)
{
    dcmt_state_t s;
    s.mode = DCMT_MODE_CLOSED_SPEED;
    s.m1_pwm = -200;
    s.m2_pwm = -201;
    s.sp1 = -1000;
    s.sp2 = BREAD_INVALID_I16;
    s.pos1 = -12345;
    s.pos2 = -23456;
    s.spd1 = -300;
    s.spd2 = -301;
    s.brakes = 0x02;
    s.estop = 0x01;
    return s;
}

static void test_clear_watchdog_trip(void)
{
    /* [type_id][opcode][data_len = 0][crc8]: no payload at all. */
    static const uint8_t header[] = {0x02, 0x7C, 0x00};
    static const uint8_t empty[1] = {0};

    fake_bus_bind();
    CHECK(dcmt_send_clear_watchdog_trip(&g_dev) == 0, "send clear_watchdog_trip");
    CHECK(g_bus.writes == 1, "one frame written");
    CHECK(fake_bus_sent(DCMT_TYPE_ID, BREAD_OP_CLEAR_WATCHDOG_TRIP, empty, 0), "clear_watchdog_trip decodes");
    CHECK(g_bus.last_frame_len == sizeof header + 1 && memcmp(g_bus.last_frame, header, sizeof header) == 0,
          "clear_watchdog_trip frame bytes");
    CHECK(dcmt_send_clear_watchdog_trip(NULL) == -1, "NULL device is refused");
}

static void test_get_state(void)
{
    static const uint8_t wire[19] = {
        0x02,       /* mode */
        0x38, 0xFF, /* m1_pwm = -200 */
        0x37, 0xFF, /* m2_pwm = -201 */
        0x18, 0xFC, /* sp1 = -1000 */
        0x00, 0x80, /* sp2 = BREAD_INVALID_I16 */
        0xC7, 0xCF, /* pos1 = -12345 */
        0x60, 0xA4, /* pos2 = -23456 */
        0xD4, 0xFE, /* spd1 = -300 */
        0xD3, 0xFE, /* spd2 = -301 */
        0x02,       /* brakes */
        0x01,       /* estop */
    };
    dcmt_state_t tx = distinct_state();
    dcmt_state_result_t rx;
    crumbs_message_t reply;

    fake_bus_bind();
    crumbs_msg_init(&reply, DCMT_TYPE_ID, DCMT_OP_GET_STATE);
    CHECK(dcmt_state_pack(&reply, &tx) == 0, "pack state");
    CHECK(reply.data_len == DCMT_STATE_FIXED_LEN && memcmp(reply.data, wire, sizeof wire) == 0,
          "state bytes, as the layout comment states");
    fake_bus_serve(&reply);

    memset(&rx, 0, sizeof rx);
    CHECK(dcmt_get_state(&g_dev, &rx) == 0, "get state");
    CHECK(g_bus.last_write.type_id == 0 && g_bus.last_write.opcode == CRUMBS_CMD_SET_REPLY &&
              g_bus.last_write.data_len == 1 && g_bus.last_write.data[0] == DCMT_OP_GET_STATE,
          "the query selected GET_STATE");
    /* Compared with literals: tx went through the same member types. */
    CHECK(rx.mode == DCMT_MODE_CLOSED_SPEED, "mode");
    CHECK(rx.m1_pwm == -200, "m1_pwm");
    CHECK(rx.m2_pwm == -201, "m2_pwm");
    CHECK(rx.sp1 == -1000, "sp1");
    CHECK(rx.sp2 == BREAD_INVALID_I16, "sp2");
    CHECK(rx.pos1 == -12345, "pos1");
    CHECK(rx.pos2 == -23456, "pos2");
    CHECK(rx.spd1 == -300, "spd1");
    CHECK(rx.spd2 == -301, "spd2");
    CHECK(rx.brakes == 0x02, "brakes");
    CHECK(rx.estop == 0x01, "estop");
}

/* DCMT_STATE_OFF_* name the same bytes the field list packs. */
static void test_state_offsets(void)
{
    dcmt_state_t tx = distinct_state();
    crumbs_message_t m;
    uint8_t u8v = 0;
    int16_t i16v = 0;

    crumbs_msg_init(&m, DCMT_TYPE_ID, DCMT_OP_GET_STATE);
    CHECK(dcmt_state_pack(&m, &tx) == 0, "pack state");
    CHECK(crumbs_msg_read_u8(m.data, m.data_len, DCMT_STATE_OFF_MODE, &u8v) == 0 && u8v == tx.mode, "OFF_MODE");
    CHECK(crumbs_msg_read_i16(m.data, m.data_len, DCMT_STATE_OFF_M1_PWM, &i16v) == 0 && i16v == tx.m1_pwm,
          "OFF_M1_PWM");
    CHECK(crumbs_msg_read_i16(m.data, m.data_len, DCMT_STATE_OFF_M2_PWM, &i16v) == 0 && i16v == tx.m2_pwm,
          "OFF_M2_PWM");
    CHECK(crumbs_msg_read_i16(m.data, m.data_len, DCMT_STATE_OFF_SP1, &i16v) == 0 && i16v == tx.sp1, "OFF_SP1");
    CHECK(crumbs_msg_read_i16(m.data, m.data_len, DCMT_STATE_OFF_SP2, &i16v) == 0 && i16v == tx.sp2, "OFF_SP2");
    CHECK(crumbs_msg_read_i16(m.data, m.data_len, DCMT_STATE_OFF_POS1, &i16v) == 0 && i16v == tx.pos1, "OFF_POS1");
    CHECK(crumbs_msg_read_i16(m.data, m.data_len, DCMT_STATE_OFF_POS2, &i16v) == 0 && i16v == tx.pos2, "OFF_POS2");
    CHECK(crumbs_msg_read_i16(m.data, m.data_len, DCMT_STATE_OFF_SPD1, &i16v) == 0 && i16v == tx.spd1, "OFF_SPD1");
    CHECK(crumbs_msg_read_i16(m.data, m.data_len, DCMT_STATE_OFF_SPD2, &i16v) == 0 && i16v == tx.spd2, "OFF_SPD2");
    CHECK(crumbs_msg_read_u8(m.data, m.data_len, DCMT_STATE_OFF_BRAKES, &u8v) == 0 && u8v == tx.brakes,
          "OFF_BRAKES");
    CHECK(crumbs_msg_read_u8(m.data, m.data_len, DCMT_STATE_OFF_ESTOP, &u8v) == 0 && u8v == tx.estop, "OFF_ESTOP");
    CHECK(m.data_len == DCMT_STATE_FIXED_LEN, "FIXED_LEN");
}

static void test_state_length_rule(void)
{
    uint8_t buf[20];
    dcmt_state_result_t rx;

    memset(buf, 0x11, sizeof buf);
    CHECK(dcmt_parse_state_payload(buf, 18, &rx) == -1, "one byte short is refused");
    CHECK(dcmt_parse_state_payload(buf, 19, &rx) == 0, "exact length is accepted");
    CHECK(dcmt_parse_state_payload(buf, 20, &rx) == -1, "a trailing byte is refused (exact-length rule)");
    CHECK(dcmt_parse_state_payload(NULL, 19, &rx) == -1, "NULL payload is refused");
}

int main(void)
{
    test_set_open_loop();
    test_set_brake();
    test_set_mode();
    test_set_setpoint();
    test_set_pid();
    test_clear_watchdog_trip();
    test_get_state();
    test_state_offsets();
    test_state_length_rule();
    if (g_failures)
    {
        printf("dcmt_roundtrip: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("dcmt_roundtrip: PASS\n");
    return 0;
}

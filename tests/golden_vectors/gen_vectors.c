/*
 * Golden-vector generator for the Python codec.
 *
 * Calls every controller-side send and query helper in the contract headers
 * through a write function that captures the frame, decodes the frame with
 * CRUMBS, and records the type id, opcode and payload bytes. Calls every
 * payload parser on known replies and records the parsed fields, or the
 * rejection of a short payload.
 *
 * The output is deterministic: fixed key order, no timestamps, no
 * environment-dependent values. CI regenerates tests/golden_vectors/vectors.json
 * and fails on any difference, so a header change that moves bytes must be
 * committed together with the vectors it changes.
 *
 * Usage: bread_contracts_gen_vectors <output.json | ->
 */
#include <bread/bread_ops.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Frame capture
 * ------------------------------------------------------------------------- */

typedef struct
{
    uint8_t frame[CRUMBS_MESSAGE_MAX_SIZE];
    size_t len;
    int calls;
} capture_t;

static capture_t g_capture;
static crumbs_context_t g_ctx;
static crumbs_device_t g_dev;

static int capture_write(void *user_ctx, uint8_t addr, const uint8_t *data, size_t len)
{
    capture_t *cap = (capture_t *)user_ctx;
    (void)addr;
    if (len > sizeof(cap->frame))
        return -1;
    memcpy(cap->frame, data, len);
    cap->len = len;
    cap->calls++;
    return 0;
}

static void die(const char *what)
{
    fprintf(stderr, "gen_vectors: %s\n", what);
    exit(1);
}

/* ---------------------------------------------------------------------------
 * JSON output
 * ------------------------------------------------------------------------- */

static FILE *g_out;
static int g_first_in_array;

static void put_hex(const uint8_t *data, size_t len)
{
    size_t i;
    fputc('"', g_out);
    for (i = 0; i < len; i++)
        fprintf(g_out, "%02x", data[i]);
    fputc('"', g_out);
}

static void array_begin(const char *key)
{
    fprintf(g_out, "  \"%s\": [", key);
    g_first_in_array = 1;
}

static void array_end(int last)
{
    fprintf(g_out, "\n  ]%s\n", last ? "" : ",");
}

static void element_begin(void)
{
    fprintf(g_out, "%s\n    {\n", g_first_in_array ? "" : ",");
    g_first_in_array = 0;
}

static void element_end(void)
{
    fprintf(g_out, "    }");
}

static void field_str(const char *key, const char *value, int last)
{
    fprintf(g_out, "      \"%s\": \"%s\"%s\n", key, value, last ? "" : ",");
}

static void field_int(const char *key, long long value, int last)
{
    fprintf(g_out, "      \"%s\": %lld%s\n", key, value, last ? "" : ",");
}

static void field_hex(const char *key, const uint8_t *data, size_t len, int last)
{
    fprintf(g_out, "      \"%s\": ", key);
    put_hex(data, len);
    fprintf(g_out, "%s\n", last ? "" : ",");
}

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------- */

static int g_first_const;

static void emit_const(const char *name, long long value)
{
    fprintf(g_out, "%s\n    \"%s\": %lld", g_first_const ? "" : ",", name, value);
    g_first_const = 0;
}

#define CONST(x) emit_const(#x, (long long)(x))

static void emit_constants(void)
{
    fprintf(g_out, "  \"constants\": {");
    g_first_const = 1;

    CONST(CRUMBS_TYPE_ID_ANY);
    CONST(CRUMBS_CMD_SET_REPLY);
    CONST(CRUMBS_MAX_PAYLOAD);

    CONST(BREAD_INVALID_I16);
    CONST(BREAD_INVALID_U8);
    CONST(BREAD_OP_GET_CAPS);
    CONST(BREAD_CAPS_SCHEMA_V1);
    CONST(BREAD_CAPS_V1_PAYLOAD_LEN);

    CONST(BREAD_OP_SET_WATCHDOG);
    CONST(BREAD_OP_GET_WATCHDOG);
    CONST(BREAD_WATCHDOG_SET_PAYLOAD_LEN);
    CONST(BREAD_WATCHDOG_OFF_ARMED);
    CONST(BREAD_WATCHDOG_OFF_TIMEOUT_MS);
    CONST(BREAD_WATCHDOG_OFF_TRIPPED);
    CONST(BREAD_WATCHDOG_OFF_TRIP_COUNT);
    CONST(BREAD_WATCHDOG_FIXED_LEN);

    CONST(BREAD_MIN_CRUMBS_VERSION);

    CONST(RLHT_TYPE_ID);
    CONST(RLHT_MODULE_VER_MAJOR);
    CONST(RLHT_MODULE_VER_MINOR);
    CONST(RLHT_MODULE_VER_PATCH);
    CONST(RLHT_MODE_CLOSED_LOOP);
    CONST(RLHT_MODE_OPEN_LOOP);
    CONST(RLHT_OP_SET_MODE);
    CONST(RLHT_OP_SET_SETPOINTS);
    CONST(RLHT_OP_SET_PID);
    CONST(RLHT_OP_SET_PERIODS);
    CONST(RLHT_OP_SET_TC_SELECT);
    CONST(RLHT_OP_SET_OPEN_DUTY);
    CONST(RLHT_OP_GET_STATE);
    CONST(RLHT_FLAG_ESTOP);
    CONST(RLHT_FLAG_RELAY1_ON);
    CONST(RLHT_FLAG_RELAY2_ON);
    CONST(RLHT_CAP_LEVEL_1);
    CONST(RLHT_CAP_LEVEL_2);
    CONST(RLHT_CAP_LEVEL_3);
    CONST(RLHT_CAP_MODE_CONTROL);
    CONST(RLHT_CAP_SETPOINT_CONTROL);
    CONST(RLHT_CAP_PID_TUNING);
    CONST(RLHT_CAP_PERIOD_CONTROL);
    CONST(RLHT_CAP_TC_SELECT);
    CONST(RLHT_CAP_OPEN_DUTY_CONTROL);
    CONST(RLHT_CAP_CMD_WATCHDOG);
    CONST(RLHT_CAP_BASELINE_FLAGS);

    CONST(DCMT_TYPE_ID);
    CONST(DCMT_MODULE_VER_MAJOR);
    CONST(DCMT_MODULE_VER_MINOR);
    CONST(DCMT_MODULE_VER_PATCH);
    CONST(DCMT_OP_SET_OPEN_LOOP);
    CONST(DCMT_OP_SET_BRAKE);
    CONST(DCMT_OP_SET_MODE);
    CONST(DCMT_OP_SET_SETPOINT);
    CONST(DCMT_OP_SET_PID);
    CONST(DCMT_OP_GET_STATE);
    CONST(DCMT_STATE_OFF_MODE);
    CONST(DCMT_STATE_OFF_M1_PWM);
    CONST(DCMT_STATE_OFF_M2_PWM);
    CONST(DCMT_STATE_OFF_SP1);
    CONST(DCMT_STATE_OFF_SP2);
    CONST(DCMT_STATE_OFF_POS1);
    CONST(DCMT_STATE_OFF_POS2);
    CONST(DCMT_STATE_OFF_SPD1);
    CONST(DCMT_STATE_OFF_SPD2);
    CONST(DCMT_STATE_OFF_BRAKES);
    CONST(DCMT_STATE_OFF_ESTOP);
    CONST(DCMT_STATE_FIXED_LEN);
    CONST(DCMT_MODE_OPEN_LOOP);
    CONST(DCMT_MODE_CLOSED_POSITION);
    CONST(DCMT_MODE_CLOSED_SPEED);
    CONST(DCMT_CAP_LEVEL_1);
    CONST(DCMT_CAP_LEVEL_2);
    CONST(DCMT_CAP_LEVEL_3);
    CONST(DCMT_CAP_OPEN_LOOP_CONTROL);
    CONST(DCMT_CAP_BRAKE_CONTROL);
    CONST(DCMT_CAP_CLOSED_LOOP_POSITION);
    CONST(DCMT_CAP_CLOSED_LOOP_SPEED);
    CONST(DCMT_CAP_PID_TUNING);
    CONST(DCMT_CAP_CMD_WATCHDOG);
    CONST(DCMT_CAP_BASELINE_FLAGS);

    fprintf(g_out, "\n  },\n");
}

/* ---------------------------------------------------------------------------
 * Encode vectors: one record per captured frame
 * ------------------------------------------------------------------------- */

/*
 * Record the frame captured by the last send helper. The frame is decoded by
 * CRUMBS itself, so the recorded type id, opcode and payload are what a
 * CRUMBS peripheral would see, not what this program assumes about the
 * frame layout.
 */
static void emit_encode(const char *name, const char *args_json,
                        const char *type_id_name, const char *opcode_name,
                        int send_rc)
{
    crumbs_message_t decoded;

    if (send_rc != 0)
        die(name);
    if (g_capture.calls != 1)
        die("send helper did not write exactly one frame");
    if (crumbs_decode_message(g_capture.frame, g_capture.len, &decoded, NULL) != 0)
        die("captured frame does not decode");

    element_begin();
    field_str("name", name, 0);
    fprintf(g_out, "      \"args\": {%s},\n", args_json);
    field_str("type_id_name", type_id_name, 0);
    field_int("type_id", decoded.type_id, 0);
    field_str("opcode_name", opcode_name, 0);
    field_int("opcode", decoded.opcode, 0);
    field_hex("payload", decoded.data, decoded.data_len, 1);
    element_end();

    memset(&g_capture, 0, sizeof(g_capture));
}

#define ARGS(buf, ...) snprintf(buf, sizeof(buf), __VA_ARGS__)

static void vec_rlht_send_set_mode(uint8_t mode)
{
    char a[64];
    ARGS(a, "\"mode\": %u", mode);
    emit_encode("rlht_send_set_mode", a, "RLHT_TYPE_ID", "RLHT_OP_SET_MODE",
                rlht_send_set_mode(&g_dev, mode));
}

static void vec_rlht_send_set_setpoints(int16_t sp1, int16_t sp2)
{
    char a[96];
    ARGS(a, "\"sp1_deci_c\": %d, \"sp2_deci_c\": %d", sp1, sp2);
    emit_encode("rlht_send_set_setpoints", a, "RLHT_TYPE_ID", "RLHT_OP_SET_SETPOINTS",
                rlht_send_set_setpoints(&g_dev, sp1, sp2));
}

static void vec_rlht_send_set_pid_x10(uint8_t kp1, uint8_t ki1, uint8_t kd1,
                                      uint8_t kp2, uint8_t ki2, uint8_t kd2)
{
    char a[160];
    ARGS(a, "\"kp1_x10\": %u, \"ki1_x10\": %u, \"kd1_x10\": %u, "
            "\"kp2_x10\": %u, \"ki2_x10\": %u, \"kd2_x10\": %u",
         kp1, ki1, kd1, kp2, ki2, kd2);
    emit_encode("rlht_send_set_pid_x10", a, "RLHT_TYPE_ID", "RLHT_OP_SET_PID",
                rlht_send_set_pid_x10(&g_dev, kp1, ki1, kd1, kp2, ki2, kd2));
}

static void vec_rlht_send_set_periods(uint16_t p1, uint16_t p2)
{
    char a[64];
    ARGS(a, "\"p1_ms\": %u, \"p2_ms\": %u", p1, p2);
    emit_encode("rlht_send_set_periods", a, "RLHT_TYPE_ID", "RLHT_OP_SET_PERIODS",
                rlht_send_set_periods(&g_dev, p1, p2));
}

static void vec_rlht_send_set_tc_select(uint8_t tc1, uint8_t tc2)
{
    char a[64];
    ARGS(a, "\"tc1\": %u, \"tc2\": %u", tc1, tc2);
    emit_encode("rlht_send_set_tc_select", a, "RLHT_TYPE_ID", "RLHT_OP_SET_TC_SELECT",
                rlht_send_set_tc_select(&g_dev, tc1, tc2));
}

static void vec_rlht_send_set_open_duty(uint8_t d1, uint8_t d2)
{
    char a[64];
    ARGS(a, "\"duty1_pct\": %u, \"duty2_pct\": %u", d1, d2);
    emit_encode("rlht_send_set_open_duty", a, "RLHT_TYPE_ID", "RLHT_OP_SET_OPEN_DUTY",
                rlht_send_set_open_duty(&g_dev, d1, d2));
}

static void vec_rlht_send_set_watchdog(uint16_t timeout_ms)
{
    char a[64];
    ARGS(a, "\"timeout_ms\": %u", timeout_ms);
    emit_encode("rlht_send_set_watchdog", a, "RLHT_TYPE_ID", "BREAD_OP_SET_WATCHDOG",
                rlht_send_set_watchdog(&g_dev, timeout_ms));
}

static void vec_dcmt_send_set_open_loop(int16_t m1, int16_t m2)
{
    char a[64];
    ARGS(a, "\"m1_pwm\": %d, \"m2_pwm\": %d", m1, m2);
    emit_encode("dcmt_send_set_open_loop", a, "DCMT_TYPE_ID", "DCMT_OP_SET_OPEN_LOOP",
                dcmt_send_set_open_loop(&g_dev, m1, m2));
}

static void vec_dcmt_send_set_brake(uint8_t b1, uint8_t b2)
{
    char a[64];
    ARGS(a, "\"m1_brake\": %u, \"m2_brake\": %u", b1, b2);
    emit_encode("dcmt_send_set_brake", a, "DCMT_TYPE_ID", "DCMT_OP_SET_BRAKE",
                dcmt_send_set_brake(&g_dev, b1, b2));
}

static void vec_dcmt_send_set_mode(uint8_t mode)
{
    char a[64];
    ARGS(a, "\"mode\": %u", mode);
    emit_encode("dcmt_send_set_mode", a, "DCMT_TYPE_ID", "DCMT_OP_SET_MODE",
                dcmt_send_set_mode(&g_dev, mode));
}

static void vec_dcmt_send_set_setpoint(int16_t t1, int16_t t2)
{
    char a[64];
    ARGS(a, "\"target1\": %d, \"target2\": %d", t1, t2);
    emit_encode("dcmt_send_set_setpoint", a, "DCMT_TYPE_ID", "DCMT_OP_SET_SETPOINT",
                dcmt_send_set_setpoint(&g_dev, t1, t2));
}

static void vec_dcmt_send_set_pid(uint8_t kp1, uint8_t ki1, uint8_t kd1,
                                  uint8_t kp2, uint8_t ki2, uint8_t kd2)
{
    char a[160];
    ARGS(a, "\"kp1_x10\": %u, \"ki1_x10\": %u, \"kd1_x10\": %u, "
            "\"kp2_x10\": %u, \"ki2_x10\": %u, \"kd2_x10\": %u",
         kp1, ki1, kd1, kp2, ki2, kd2);
    emit_encode("dcmt_send_set_pid", a, "DCMT_TYPE_ID", "DCMT_OP_SET_PID",
                dcmt_send_set_pid(&g_dev, kp1, ki1, kd1, kp2, ki2, kd2));
}

static void vec_dcmt_send_set_watchdog(uint16_t timeout_ms)
{
    char a[64];
    ARGS(a, "\"timeout_ms\": %u", timeout_ms);
    emit_encode("dcmt_send_set_watchdog", a, "DCMT_TYPE_ID", "BREAD_OP_SET_WATCHDOG",
                dcmt_send_set_watchdog(&g_dev, timeout_ms));
}

/* Query helpers carry no arguments: [CRUMBS_TYPE_ID_ANY][CRUMBS_CMD_SET_REPLY][opcode]. */
static void vec_query(const char *name, int rc)
{
    emit_encode(name, "", "CRUMBS_TYPE_ID_ANY", "CRUMBS_CMD_SET_REPLY", rc);
}

static void emit_encode_vectors(void)
{
    array_begin("encode");

    vec_rlht_send_set_mode(RLHT_MODE_CLOSED_LOOP);
    vec_rlht_send_set_mode(RLHT_MODE_OPEN_LOOP);
    vec_rlht_send_set_mode(255);

    vec_rlht_send_set_setpoints(250, 300);
    vec_rlht_send_set_setpoints(-10, 0);
    vec_rlht_send_set_setpoints(0, 0);
    vec_rlht_send_set_setpoints(INT16_MIN, INT16_MAX);
    vec_rlht_send_set_setpoints(INT16_MAX, INT16_MIN);

    vec_rlht_send_set_pid_x10(1, 2, 3, 4, 5, 6);
    vec_rlht_send_set_pid_x10(0, 0, 0, 0, 0, 0);
    vec_rlht_send_set_pid_x10(255, 255, 255, 255, 255, 255);

    vec_rlht_send_set_periods(1000, 2000);
    vec_rlht_send_set_periods(0, 0);
    vec_rlht_send_set_periods(UINT16_MAX, 1);

    vec_rlht_send_set_tc_select(0, 1);
    vec_rlht_send_set_tc_select(3, 2);
    vec_rlht_send_set_tc_select(255, 255);

    vec_rlht_send_set_open_duty(0, 0);
    vec_rlht_send_set_open_duty(50, 100);
    vec_rlht_send_set_open_duty(255, 255);

    vec_rlht_send_set_watchdog(0);
    vec_rlht_send_set_watchdog(5000);
    vec_rlht_send_set_watchdog(UINT16_MAX);

    vec_query("rlht_query_state", rlht_query_state(&g_dev));
    vec_query("rlht_query_version", rlht_query_version(&g_dev));
    vec_query("rlht_query_caps", rlht_query_caps(&g_dev));
    vec_query("rlht_query_watchdog", rlht_query_watchdog(&g_dev));

    vec_dcmt_send_set_open_loop(0, 0);
    vec_dcmt_send_set_open_loop(-255, 255);
    vec_dcmt_send_set_open_loop(INT16_MIN, INT16_MAX);

    vec_dcmt_send_set_brake(0, 0);
    vec_dcmt_send_set_brake(1, 0);
    vec_dcmt_send_set_brake(255, 255);

    vec_dcmt_send_set_mode(DCMT_MODE_OPEN_LOOP);
    vec_dcmt_send_set_mode(DCMT_MODE_CLOSED_POSITION);
    vec_dcmt_send_set_mode(DCMT_MODE_CLOSED_SPEED);
    vec_dcmt_send_set_mode(255);

    vec_dcmt_send_set_setpoint(100, -100);
    vec_dcmt_send_set_setpoint(0, 0);
    vec_dcmt_send_set_setpoint(INT16_MIN, INT16_MAX);

    vec_dcmt_send_set_pid(1, 2, 3, 4, 5, 6);
    vec_dcmt_send_set_pid(0, 0, 0, 0, 0, 0);
    vec_dcmt_send_set_pid(255, 255, 255, 255, 255, 255);

    vec_dcmt_send_set_watchdog(0);
    vec_dcmt_send_set_watchdog(5000);
    vec_dcmt_send_set_watchdog(UINT16_MAX);

    vec_query("dcmt_query_state", dcmt_query_state(&g_dev));
    vec_query("dcmt_query_version", dcmt_query_version(&g_dev));
    vec_query("dcmt_query_caps", dcmt_query_caps(&g_dev));
    vec_query("dcmt_query_watchdog", dcmt_query_watchdog(&g_dev));

    array_end(0);
}

/* ---------------------------------------------------------------------------
 * Parse vectors: one record per (payload, parser) call
 * ------------------------------------------------------------------------- */

static void parse_header(const char *name, const char *type_id_name, uint8_t type_id,
                         const char *opcode_name, uint8_t opcode,
                         const uint8_t *payload, size_t len, int rc)
{
    element_begin();
    field_str("name", name, 0);
    field_str("type_id_name", type_id_name, 0);
    field_int("type_id", type_id, 0);
    field_str("opcode_name", opcode_name, 0);
    field_int("opcode", opcode, 0);
    field_hex("payload", payload, len, 0);
    field_int("rc", rc, 0);
    if (rc != 0)
    {
        fprintf(g_out, "      \"result\": null\n");
        element_end();
    }
}

#define RESULT_FIELD(key, value, last) \
    fprintf(g_out, "        \"%s\": %lld%s\n", key, (long long)(value), (last) ? "" : ",")

static void result_begin(void)
{
    fprintf(g_out, "      \"result\": {\n");
}

static void result_end(void)
{
    fprintf(g_out, "      }\n");
    element_end();
}

static void vec_rlht_parse_state(const crumbs_message_t *reply, uint8_t len)
{
    rlht_state_result_t r;
    int rc = rlht_parse_state_payload(reply->data, len, &r);

    parse_header("rlht_parse_state_payload", "RLHT_TYPE_ID", RLHT_TYPE_ID,
                 "RLHT_OP_GET_STATE", RLHT_OP_GET_STATE, reply->data, len, rc);
    if (rc != 0)
        return;

    result_begin();
    RESULT_FIELD("mode", r.mode, 0);
    RESULT_FIELD("flags", r.flags, 0);
    RESULT_FIELD("t1_deci_c", r.t1_deci_c, 0);
    RESULT_FIELD("t2_deci_c", r.t2_deci_c, 0);
    RESULT_FIELD("sp1_deci_c", r.sp1_deci_c, 0);
    RESULT_FIELD("sp2_deci_c", r.sp2_deci_c, 0);
    RESULT_FIELD("on1_ms", r.on1_ms, 0);
    RESULT_FIELD("on2_ms", r.on2_ms, 0);
    RESULT_FIELD("period1_ms", r.period1_ms, 0);
    RESULT_FIELD("period2_ms", r.period2_ms, 0);
    RESULT_FIELD("tc_select", r.tc_select, 0);
    RESULT_FIELD("tc1", r.tc1, 0);
    RESULT_FIELD("tc2", r.tc2, 1);
    result_end();
}

static void rlht_state_reply(crumbs_message_t *m, uint8_t mode, uint8_t flags,
                             int16_t t1, int16_t t2, int16_t sp1, int16_t sp2,
                             uint16_t on1, uint16_t on2, uint16_t period1, uint16_t period2,
                             uint8_t tc_select)
{
    crumbs_msg_init(m, RLHT_TYPE_ID, RLHT_OP_GET_STATE);
    crumbs_msg_add_u8(m, mode);
    crumbs_msg_add_u8(m, flags);
    crumbs_msg_add_i16(m, t1);
    crumbs_msg_add_i16(m, t2);
    crumbs_msg_add_i16(m, sp1);
    crumbs_msg_add_i16(m, sp2);
    crumbs_msg_add_u16(m, on1);
    crumbs_msg_add_u16(m, on2);
    crumbs_msg_add_u16(m, period1);
    crumbs_msg_add_u16(m, period2);
    if (crumbs_msg_add_u8(m, tc_select) != 0)
        die("rlht state reply overflow");
}

static void vec_dcmt_parse_state(const crumbs_message_t *reply, uint8_t len)
{
    dcmt_state_result_t r;
    int rc = dcmt_parse_state_payload(reply->data, len, &r);

    parse_header("dcmt_parse_state_payload", "DCMT_TYPE_ID", DCMT_TYPE_ID,
                 "DCMT_OP_GET_STATE", DCMT_OP_GET_STATE, reply->data, len, rc);
    if (rc != 0)
        return;

    result_begin();
    RESULT_FIELD("mode", r.mode, 0);
    RESULT_FIELD("m1_pwm", r.m1_pwm, 0);
    RESULT_FIELD("m2_pwm", r.m2_pwm, 0);
    RESULT_FIELD("sp1", r.sp1, 0);
    RESULT_FIELD("sp2", r.sp2, 0);
    RESULT_FIELD("pos1", r.pos1, 0);
    RESULT_FIELD("pos2", r.pos2, 0);
    RESULT_FIELD("spd1", r.spd1, 0);
    RESULT_FIELD("spd2", r.spd2, 0);
    RESULT_FIELD("brakes", r.brakes, 0);
    RESULT_FIELD("estop", r.estop, 1);
    result_end();
}

static void dcmt_state_reply(crumbs_message_t *m, uint8_t mode,
                             int16_t m1_pwm, int16_t m2_pwm, int16_t sp1, int16_t sp2,
                             int16_t pos1, int16_t pos2, int16_t spd1, int16_t spd2,
                             uint8_t brakes, uint8_t estop)
{
    crumbs_msg_init(m, DCMT_TYPE_ID, DCMT_OP_GET_STATE);
    crumbs_msg_add_u8(m, mode);
    crumbs_msg_add_i16(m, m1_pwm);
    crumbs_msg_add_i16(m, m2_pwm);
    crumbs_msg_add_i16(m, sp1);
    crumbs_msg_add_i16(m, sp2);
    crumbs_msg_add_i16(m, pos1);
    crumbs_msg_add_i16(m, pos2);
    crumbs_msg_add_i16(m, spd1);
    crumbs_msg_add_i16(m, spd2);
    crumbs_msg_add_u8(m, brakes);
    if (crumbs_msg_add_u8(m, estop) != 0)
        die("dcmt state reply overflow");
    if (m->data_len != DCMT_STATE_FIXED_LEN)
        die("dcmt state reply length");
}

static void vec_bread_caps_parse(const char *type_id_name, const crumbs_message_t *reply, uint8_t len)
{
    bread_caps_result_t r;
    int rc = bread_caps_parse_payload(reply->data, len, &r);

    parse_header("bread_caps_parse_payload", type_id_name, reply->type_id,
                 "BREAD_OP_GET_CAPS", BREAD_OP_GET_CAPS, reply->data, len, rc);
    if (rc != 0)
        return;

    result_begin();
    RESULT_FIELD("schema", r.schema, 0);
    RESULT_FIELD("level", r.level, 0);
    RESULT_FIELD("flags", r.flags, 1);
    result_end();
}

static void vec_bread_watchdog_parse(const char *type_id_name, const crumbs_message_t *reply, uint8_t len)
{
    bread_watchdog_result_t r;
    int rc = bread_watchdog_parse_payload(reply->data, len, &r);

    parse_header("bread_watchdog_parse_payload", type_id_name, reply->type_id,
                 "BREAD_OP_GET_WATCHDOG", BREAD_OP_GET_WATCHDOG, reply->data, len, rc);
    if (rc != 0)
        return;

    result_begin();
    RESULT_FIELD("armed", r.armed, 0);
    RESULT_FIELD("timeout_ms", r.timeout_ms, 0);
    RESULT_FIELD("tripped", r.tripped, 0);
    RESULT_FIELD("trip_count", r.trip_count, 1);
    result_end();
}

/*
 * The version reply has no opcode macro in the headers; rlht_query_version()
 * and dcmt_query_version() request the literal opcode 0x00.
 */
static void vec_bread_parse_version(const char *type_id_name, const crumbs_message_t *reply, uint8_t len)
{
    uint16_t crumbs_ver = 0;
    uint8_t mod_major = 0, mod_minor = 0, mod_patch = 0;
    int rc = bread_parse_version(reply->data, len, &crumbs_ver, &mod_major, &mod_minor, &mod_patch);

    parse_header("bread_parse_version", type_id_name, reply->type_id,
                 "BREAD_OP_GET_VERSION", 0x00, reply->data, len, rc);
    if (rc != 0)
        return;

    result_begin();
    RESULT_FIELD("crumbs_ver", crumbs_ver, 0);
    RESULT_FIELD("mod_major", mod_major, 0);
    RESULT_FIELD("mod_minor", mod_minor, 0);
    RESULT_FIELD("mod_patch", mod_patch, 1);
    result_end();
}

static void version_reply(crumbs_message_t *m, uint8_t type_id, uint16_t crumbs_ver,
                          uint8_t major, uint8_t minor, uint8_t patch)
{
    crumbs_msg_init(m, type_id, 0x00);
    crumbs_msg_add_u16(m, crumbs_ver);
    crumbs_msg_add_u8(m, major);
    crumbs_msg_add_u8(m, minor);
    crumbs_msg_add_u8(m, patch);
}

static void emit_parse_vectors(void)
{
    crumbs_message_t m;

    array_begin("parse");

    /* RLHT GET_STATE: typical, sentinel/extreme, zero, short, empty. */
    rlht_state_reply(&m, RLHT_MODE_CLOSED_LOOP, RLHT_FLAG_RELAY1_ON,
                     500, -10, 250, 300, 100, 200, 1000, 2000, 0x06);
    vec_rlht_parse_state(&m, m.data_len);
    rlht_state_reply(&m, RLHT_MODE_OPEN_LOOP, RLHT_FLAG_ESTOP | RLHT_FLAG_RELAY1_ON | RLHT_FLAG_RELAY2_ON,
                     BREAD_INVALID_I16, INT16_MAX, INT16_MIN, -1, UINT16_MAX, 0, UINT16_MAX, 1, 0xFF);
    vec_rlht_parse_state(&m, m.data_len);
    rlht_state_reply(&m, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    vec_rlht_parse_state(&m, m.data_len);
    vec_rlht_parse_state(&m, (uint8_t)(m.data_len - 1));
    vec_rlht_parse_state(&m, 0);

    /* DCMT GET_STATE: closed-speed, open-loop with sentinels, extremes, zero, short, empty. */
    dcmt_state_reply(&m, DCMT_MODE_CLOSED_SPEED, 100, -100, 500, -500, 1234, -1234, 60, -60, 0, 0);
    vec_dcmt_parse_state(&m, m.data_len);
    dcmt_state_reply(&m, DCMT_MODE_OPEN_LOOP, 255, -255, BREAD_INVALID_I16, BREAD_INVALID_I16,
                     0, 0, BREAD_INVALID_I16, BREAD_INVALID_I16, 0x03, 1);
    vec_dcmt_parse_state(&m, m.data_len);
    dcmt_state_reply(&m, DCMT_MODE_CLOSED_POSITION, INT16_MIN, INT16_MAX, INT16_MAX, INT16_MIN,
                     INT16_MIN, INT16_MAX, -1, 1, 255, 255);
    vec_dcmt_parse_state(&m, m.data_len);
    dcmt_state_reply(&m, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    vec_dcmt_parse_state(&m, m.data_len);
    vec_dcmt_parse_state(&m, (uint8_t)(m.data_len - 1));
    vec_dcmt_parse_state(&m, 0);

    /* GET_CAPS: replies built by the header's own builder. */
    if (bread_caps_build_reply(&m, RLHT_TYPE_ID, RLHT_CAP_LEVEL_3,
                               RLHT_CAP_BASELINE_FLAGS | RLHT_CAP_CMD_WATCHDOG) != 0)
        die("caps build");
    vec_bread_caps_parse("RLHT_TYPE_ID", &m, m.data_len);
    if (bread_caps_build_reply(&m, DCMT_TYPE_ID, DCMT_CAP_LEVEL_1, DCMT_CAP_BASELINE_FLAGS) != 0)
        die("caps build");
    vec_bread_caps_parse("DCMT_TYPE_ID", &m, m.data_len);
    if (bread_caps_build_reply(&m, DCMT_TYPE_ID, 255, 0xFFFFFFFFu) != 0)
        die("caps build");
    vec_bread_caps_parse("DCMT_TYPE_ID", &m, m.data_len);
    if (bread_caps_build_reply(&m, RLHT_TYPE_ID, 0, 0) != 0)
        die("caps build");
    vec_bread_caps_parse("RLHT_TYPE_ID", &m, m.data_len);
    vec_bread_caps_parse("RLHT_TYPE_ID", &m, (uint8_t)(m.data_len - 1));
    vec_bread_caps_parse("RLHT_TYPE_ID", &m, 0);

    /* GET_WATCHDOG: replies built by the header's own builder. */
    if (bread_watchdog_build_reply(&m, DCMT_TYPE_ID, 1, 5000, 1, 3) != 0)
        die("watchdog build");
    vec_bread_watchdog_parse("DCMT_TYPE_ID", &m, m.data_len);
    if (bread_watchdog_build_reply(&m, RLHT_TYPE_ID, 0, 0, 0, 0) != 0)
        die("watchdog build");
    vec_bread_watchdog_parse("RLHT_TYPE_ID", &m, m.data_len);
    if (bread_watchdog_build_reply(&m, RLHT_TYPE_ID, 1, UINT16_MAX, 1, 255) != 0)
        die("watchdog build");
    vec_bread_watchdog_parse("RLHT_TYPE_ID", &m, m.data_len);
    vec_bread_watchdog_parse("RLHT_TYPE_ID", &m, (uint8_t)(m.data_len - 1));
    vec_bread_watchdog_parse("RLHT_TYPE_ID", &m, 0);

    /* Version: CRUMBS 0.14.0 (1400) and the minimum (1200), module 1.0.0 and extremes. */
    version_reply(&m, RLHT_TYPE_ID, CRUMBS_VERSION,
                  RLHT_MODULE_VER_MAJOR, RLHT_MODULE_VER_MINOR, RLHT_MODULE_VER_PATCH);
    vec_bread_parse_version("RLHT_TYPE_ID", &m, m.data_len);
    version_reply(&m, DCMT_TYPE_ID, BREAD_MIN_CRUMBS_VERSION,
                  DCMT_MODULE_VER_MAJOR, DCMT_MODULE_VER_MINOR, DCMT_MODULE_VER_PATCH);
    vec_bread_parse_version("DCMT_TYPE_ID", &m, m.data_len);
    version_reply(&m, DCMT_TYPE_ID, UINT16_MAX, 255, 255, 255);
    vec_bread_parse_version("DCMT_TYPE_ID", &m, m.data_len);
    version_reply(&m, RLHT_TYPE_ID, 0, 0, 0, 0);
    vec_bread_parse_version("RLHT_TYPE_ID", &m, m.data_len);
    vec_bread_parse_version("RLHT_TYPE_ID", &m, (uint8_t)(m.data_len - 1));
    vec_bread_parse_version("RLHT_TYPE_ID", &m, 0);

    array_end(1);
}

/* ---------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <output.json | ->\n", argv[0]);
        return 2;
    }

    crumbs_init(&g_ctx, CRUMBS_ROLE_CONTROLLER, 0);
    g_dev.ctx = &g_ctx;
    g_dev.addr = 0x08;
    g_dev.write_fn = capture_write;
    g_dev.read_fn = NULL;
    g_dev.delay_fn = NULL;
    g_dev.io = &g_capture;

    if (strcmp(argv[1], "-") == 0)
        g_out = stdout;
    else
        g_out = fopen(argv[1], "wb");
    if (!g_out)
    {
        perror(argv[1]);
        return 1;
    }

    fprintf(g_out, "{\n");
    fprintf(g_out, "  \"schema\": 1,\n");
    fprintf(g_out, "  \"generator\": \"tests/golden_vectors/gen_vectors.c\",\n");
    emit_constants();
    emit_encode_vectors();
    emit_parse_vectors();
    fprintf(g_out, "}\n");

    if (g_out != stdout && fclose(g_out) != 0)
    {
        perror(argv[1]);
        return 1;
    }
    return 0;
}

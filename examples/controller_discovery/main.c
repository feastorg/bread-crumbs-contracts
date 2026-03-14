#define _DEFAULT_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bread/bread_ops.h>
#include <crumbs.h>
#include <crumbs_linux.h>

#define MAX_DEVICES 32
#define MAX_CMD_LEN 256

typedef struct
{
    uint8_t type_id;
    uint16_t crumbs_ver;
    uint8_t mod_major;
    uint8_t mod_minor;
    uint8_t mod_patch;
    int compat; /* 0=OK, <0 error */
    crumbs_device_t dev;
} device_info_t;

static device_info_t g_devices[MAX_DEVICES];
static int g_device_count = 0;

static void trim_whitespace(char *s)
{
    char *start;
    char *end;

    if (!s)
        return;

    start = s;
    while (*start && isspace((unsigned char)*start))
        start++;

    if (*start == '\0')
    {
        s[0] = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
        end--;

    memmove(s, start, (size_t)(end - start + 1));
    s[end - start + 1] = '\0';
}

static const char *next_token(const char *in, char *out, size_t out_sz)
{
    size_t i = 0;

    while (*in && isspace((unsigned char)*in))
        in++;

    if (*in == '\0')
    {
        if (out_sz > 0)
            out[0] = '\0';
        return in;
    }

    while (*in && !isspace((unsigned char)*in))
    {
        if (i + 1 < out_sz)
            out[i] = *in;
        i++;
        in++;
    }

    if (out_sz > 0)
        out[(i < out_sz) ? i : (out_sz - 1)] = '\0';

    while (*in && isspace((unsigned char)*in))
        in++;

    return in;
}

static const char *type_name(uint8_t type_id)
{
    if (type_id == RLHT_TYPE_ID)
        return "RLHT";
    if (type_id == DCMT_TYPE_ID)
        return "DCMT";
    return "UNKNOWN";
}

static void format_crumbs_version(uint16_t v, char *buf, size_t n)
{
    unsigned int major = (unsigned int)(v / 10000);
    unsigned int minor = (unsigned int)((v / 100) % 100);
    unsigned int patch = (unsigned int)(v % 100);
    snprintf(buf, n, "%u.%u.%u", major, minor, patch);
}

static const char *dcmt_mode_name(uint8_t mode)
{
    if (mode == DCMT_MODE_OPEN_LOOP)
        return "open";
    if (mode == DCMT_MODE_CLOSED_POSITION)
        return "closed_pos";
    if (mode == DCMT_MODE_CLOSED_SPEED)
        return "closed_speed";
    return "unknown";
}

static const char *rlht_mode_name(uint8_t mode)
{
    if (mode == RLHT_MODE_CLOSED_LOOP)
        return "closed";
    if (mode == RLHT_MODE_OPEN_LOOP)
        return "open";
    return "unknown";
}

static void default_caps_for_type(uint8_t type_id, uint8_t *schema, uint8_t *level, uint32_t *flags)
{
    if (schema)
        *schema = BREAD_CAPS_SCHEMA_V1;

    if (type_id == DCMT_TYPE_ID)
    {
        if (level)
            *level = DCMT_CAP_LEVEL_1;
        if (flags)
            *flags = DCMT_CAP_BASELINE_FLAGS;
        return;
    }

    if (type_id == RLHT_TYPE_ID)
    {
        if (level)
            *level = RLHT_CAP_LEVEL_1;
        if (flags)
            *flags = RLHT_CAP_BASELINE_FLAGS;
        return;
    }

    if (level)
        *level = 0;
    if (flags)
        *flags = 0;
}

/* Returns 1 if queried from device, 0 if baseline fallback was used. */
static int query_caps_for_type(const crumbs_device_t *dev, uint8_t type_id,
                               uint8_t *schema, uint8_t *level, uint32_t *flags)
{
    int rc;

    if (type_id == DCMT_TYPE_ID)
    {
        dcmt_caps_result_t caps;
        rc = dcmt_get_caps(dev, &caps);
        if (rc == 0)
        {
            if (schema)
                *schema = caps.schema;
            if (level)
                *level = caps.level;
            if (flags)
                *flags = caps.flags;
            return 1;
        }
    }
    else if (type_id == RLHT_TYPE_ID)
    {
        rlht_caps_result_t caps;
        rc = rlht_get_caps(dev, &caps);
        if (rc == 0)
        {
            if (schema)
                *schema = caps.schema;
            if (level)
                *level = caps.level;
            if (flags)
                *flags = caps.flags;
            return 1;
        }
    }

    default_caps_for_type(type_id, schema, level, flags);
    return 0;
}

static int probe_device(crumbs_context_t *ctx, crumbs_linux_i2c_t *lw, device_info_t *d)
{
    int crumbs_ok;
    int module_ok;
    uint8_t exp_major = 0;
    uint8_t exp_minor = 0;
    int rc;

    d->dev.ctx = ctx;
    d->dev.write_fn = crumbs_linux_i2c_write;
    d->dev.read_fn = crumbs_linux_read;
    d->dev.delay_fn = crumbs_linux_delay_us;
    d->dev.io = (void *)lw;

    if (d->type_id == RLHT_TYPE_ID)
    {
        rlht_version_result_t v;
        rc = rlht_get_version(&d->dev, &v);
        if (rc != 0)
        {
            d->compat = -10;
            return rc;
        }
        d->crumbs_ver = v.crumbs_version;
        d->mod_major = v.fw_major;
        d->mod_minor = v.fw_minor;
        d->mod_patch = v.fw_patch;
        exp_major = RLHT_MODULE_VER_MAJOR;
        exp_minor = RLHT_MODULE_VER_MINOR;
    }
    else if (d->type_id == DCMT_TYPE_ID)
    {
        dcmt_version_result_t v;
        rc = dcmt_get_version(&d->dev, &v);
        if (rc != 0)
        {
            d->compat = -10;
            return rc;
        }
        d->crumbs_ver = v.crumbs_version;
        d->mod_major = v.fw_major;
        d->mod_minor = v.fw_minor;
        d->mod_patch = v.fw_patch;
        exp_major = DCMT_MODULE_VER_MAJOR;
        exp_minor = DCMT_MODULE_VER_MINOR;
    }
    else
    {
        d->compat = -20;
        return -1;
    }

    crumbs_ok = bread_check_crumbs_compat(d->crumbs_ver);
    module_ok = bread_check_module_compat(d->mod_major, d->mod_minor, exp_major, exp_minor);

    if (crumbs_ok != 0)
    {
        d->compat = -1;
        return -1;
    }
    if (module_ok == -1)
    {
        d->compat = -2;
        return -1;
    }
    if (module_ok == -2)
    {
        d->compat = -3;
        return -1;
    }

    d->compat = 0;
    return 0;
}

static void print_help(void)
{
    printf("\nBREAD Discovery Controller\n");
    printf("==========================\n");
    printf("General:\n");
    printf("  help                          - show help\n");
    printf("  scan                          - discover devices\n");
    printf("  list                          - list discovered devices\n");
    printf("  quit | exit                   - quit\n\n");

    printf("Device select:\n");
    printf("  rlht <idx|@addr> <cmd> ...\n");
    printf("  dcmt <idx|@addr> <cmd> ...\n\n");

    printf("RLHT cmds:\n");
    printf("  state\n");
    printf("  version\n");
    printf("  caps\n");
    printf("  mode <closed|open|0|1>\n");
    printf("  setpoints <sp1_deciC> <sp2_deciC>\n");
    printf("  pid <kp1 ki1 kd1 kp2 ki2 kd2> (x10 units)\n");
    printf("  periods <p1_ms> <p2_ms>\n");
    printf("  tc <tc1> <tc2>\n");
    printf("  duty <d1_pct> <d2_pct>\n\n");

    printf("DCMT cmds:\n");
    printf("  state\n");
    printf("  version\n");
    printf("  caps\n");
    printf("  open <m1_pwm> <m2_pwm>\n");
    printf("  brake <m1> <m2>\n");
    printf("  mode <open|pos|speed|0|1|2>\n");
    printf("  setpoint <t1> <t2>\n");
    printf("  pid <kp1 ki1 kd1 kp2 ki2 kd2> (x10 units)\n\n");
}

static void cmd_list(void)
{
    int i;
    int type_idx[256] = {0};

    if (g_device_count == 0)
    {
        printf("No devices. Run 'scan'.\n");
        return;
    }

    printf("\nDiscovered devices:\n");
    for (i = 0; i < g_device_count; i++)
    {
        int idx = type_idx[g_devices[i].type_id]++;
        printf("  [%d] %s #%d @0x%02X compat=%s\n",
               i,
               type_name(g_devices[i].type_id),
               idx,
               g_devices[i].dev.addr,
               (g_devices[i].compat == 0) ? "OK" : "NO");
    }
    printf("\n");
}

static int resolve_device(uint8_t type_id, const char *args, device_info_t **out_dev, const char **out_rest)
{
    const char *rest;

    while (*args && isspace((unsigned char)*args))
        args++;

    if (*args == '@')
    {
        unsigned int addr;
        int i;

        if (sscanf(args, "@%i", &addr) != 1)
        {
            printf("Invalid @addr selector\n");
            return -1;
        }

        rest = strchr(args, ' ');
        rest = rest ? rest + 1 : args + strlen(args);
        while (*rest && isspace((unsigned char)*rest))
            rest++;

        for (i = 0; i < g_device_count; i++)
        {
            if (g_devices[i].dev.addr == (uint8_t)addr && g_devices[i].type_id == type_id)
            {
                if (g_devices[i].compat != 0)
                {
                    printf("Device at 0x%02X is incompatible.\n", (uint8_t)addr);
                    return -1;
                }
                *out_dev = &g_devices[i];
                *out_rest = rest;
                return 0;
            }
        }
        printf("No %s at 0x%02X\n", type_name(type_id), (uint8_t)addr);
        return -1;
    }
    else
    {
        int want_idx;
        int i;
        int seen = 0;

        if (sscanf(args, "%d", &want_idx) != 1)
        {
            printf("Expected idx or @addr\n");
            return -1;
        }

        rest = strchr(args, ' ');
        rest = rest ? rest + 1 : args + strlen(args);
        while (*rest && isspace((unsigned char)*rest))
            rest++;

        for (i = 0; i < g_device_count; i++)
        {
            if (g_devices[i].type_id == type_id && g_devices[i].compat == 0)
            {
                if (seen == want_idx)
                {
                    *out_dev = &g_devices[i];
                    *out_rest = rest;
                    return 0;
                }
                seen++;
            }
        }

        printf("No %s index %d\n", type_name(type_id), want_idx);
        return -1;
    }
}

static int cmd_scan(crumbs_context_t *ctx, crumbs_linux_i2c_t *lw)
{
    uint8_t addrs[MAX_DEVICES];
    uint8_t types[MAX_DEVICES];
    int count;
    int i;
    int usable = 0;

    printf("Scanning bus...\n");
    count = crumbs_linux_scan_for_crumbs_with_types(ctx, lw, 0x08, 0x77, 0,
                                                     addrs, types, MAX_DEVICES, 100000);
    if (count < 0)
    {
        printf("Scan failed: %d\n", count);
        g_device_count = 0;
        return count;
    }
    if (count == 0)
    {
        printf("No CRUMBS devices found.\n");
        g_device_count = 0;
        return 0;
    }

    g_device_count = count;

    for (i = 0; i < count; i++)
    {
        device_info_t *d = &g_devices[i];
        char pver[16];
        char cver[16];

        memset(d, 0, sizeof(*d));
        d->type_id = types[i];
        d->dev.addr = addrs[i];
        d->compat = -99;

        printf("[0x%02X] type=0x%02X (%s)\n", d->dev.addr, d->type_id, type_name(d->type_id));

        if (d->type_id != RLHT_TYPE_ID && d->type_id != DCMT_TYPE_ID)
        {
            printf("       Skipping unsupported type for this tool\n");
            d->compat = -20;
            continue;
        }

        if (probe_device(ctx, lw, d) != 0)
        {
            printf("       Version probe/compat failed (compat=%d)\n", d->compat);
            continue;
        }

        format_crumbs_version(d->crumbs_ver, pver, sizeof(pver));
        format_crumbs_version(CRUMBS_VERSION, cver, sizeof(cver));
        printf("       CRUMBS %s (controller %s)\n", pver, cver);
        printf("       Module %u.%u.%u  [compatible]\n",
               d->mod_major, d->mod_minor, d->mod_patch);
        {
            uint8_t caps_schema = 0;
            uint8_t caps_level = 0;
            uint32_t caps_flags = 0;
            int caps_queried = query_caps_for_type(&d->dev, d->type_id, &caps_schema, &caps_level, &caps_flags);
            printf("       Caps schema=%u level=%u flags=0x%08lX (%s)\n",
                   (unsigned int)caps_schema,
                   (unsigned int)caps_level,
                   (unsigned long)caps_flags,
                   caps_queried ? "queried" : "fallback");
        }
        usable++;
    }

    printf("Usable devices: %d/%d\n", usable, count);
    return usable;
}

static int cmd_rlht(const device_info_t *info, const char *rest)
{
    const crumbs_device_t *dev = &info->dev;
    char sub[32];
    int rc;

    rest = next_token(rest, sub, sizeof(sub));
    if (sub[0] == '\0')
    {
        printf("Usage: rlht <idx|@addr> <state|version|caps|mode|setpoints|pid|periods|tc|duty> ...\n");
        return -1;
    }

    if (strcmp(sub, "state") == 0)
    {
        rlht_state_result_t s;
        rc = rlht_get_state(dev, &s);
        if (rc != 0)
            return rc;

        printf("RLHT state: mode=%s flags=0x%02X t1=%.1fC t2=%.1fC sp1=%.1fC sp2=%.1fC on1=%ums on2=%ums p1=%ums p2=%ums tc1=%u tc2=%u\n",
               rlht_mode_name(s.mode),
               s.flags,
               (double)s.t1_deci_c / 10.0,
               (double)s.t2_deci_c / 10.0,
               (double)s.sp1_deci_c / 10.0,
               (double)s.sp2_deci_c / 10.0,
               s.on1_ms, s.on2_ms, s.period1_ms, s.period2_ms,
               s.tc1, s.tc2);
        return 0;
    }

    if (strcmp(sub, "version") == 0)
    {
        rlht_version_result_t v;
        rc = rlht_get_version(dev, &v);
        if (rc != 0)
            return rc;
        printf("RLHT version: crumbs=%u fw=%u.%u.%u\n", v.crumbs_version, v.fw_major, v.fw_minor, v.fw_patch);
        return 0;
    }

    if (strcmp(sub, "caps") == 0)
    {
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        int caps_queried = query_caps_for_type(dev, RLHT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        printf("RLHT caps: schema=%u level=%u flags=0x%08lX source=%s\n",
               (unsigned int)caps_schema,
               (unsigned int)caps_level,
               (unsigned long)caps_flags,
               caps_queried ? "queried" : "fallback");
        return 0;
    }

    if (strcmp(sub, "mode") == 0)
    {
        char tok[32];
        uint8_t mode;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, RLHT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & RLHT_CAP_MODE_CONTROL) == 0)
        {
            printf("RLHT mode command unsupported by reported capabilities.\n");
            return -2;
        }
        rest = next_token(rest, tok, sizeof(tok));
        if (tok[0] == '\0')
            return -1;
        if (strcmp(tok, "open") == 0 || strcmp(tok, "1") == 0)
            mode = RLHT_MODE_OPEN_LOOP;
        else if (strcmp(tok, "closed") == 0 || strcmp(tok, "0") == 0)
            mode = RLHT_MODE_CLOSED_LOOP;
        else
            return -1;
        return rlht_send_set_mode(dev, mode);
    }

    if (strcmp(sub, "setpoints") == 0)
    {
        int sp1;
        int sp2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, RLHT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & RLHT_CAP_SETPOINT_CONTROL) == 0)
        {
            printf("RLHT setpoints command unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%d %d", &sp1, &sp2) != 2)
            return -1;
        return rlht_send_set_setpoints(dev, (int16_t)sp1, (int16_t)sp2);
    }

    if (strcmp(sub, "pid") == 0)
    {
        unsigned int kp1, ki1, kd1, kp2, ki2, kd2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, RLHT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & RLHT_CAP_PID_TUNING) == 0)
        {
            printf("RLHT pid command unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%u %u %u %u %u %u", &kp1, &ki1, &kd1, &kp2, &ki2, &kd2) != 6)
            return -1;
        return rlht_send_set_pid_x10(dev,
                                     (uint8_t)kp1, (uint8_t)ki1, (uint8_t)kd1,
                                     (uint8_t)kp2, (uint8_t)ki2, (uint8_t)kd2);
    }

    if (strcmp(sub, "periods") == 0)
    {
        unsigned int p1;
        unsigned int p2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, RLHT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & RLHT_CAP_PERIOD_CONTROL) == 0)
        {
            printf("RLHT periods command unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%u %u", &p1, &p2) != 2)
            return -1;
        return rlht_send_set_periods(dev, (uint16_t)p1, (uint16_t)p2);
    }

    if (strcmp(sub, "tc") == 0)
    {
        unsigned int tc1;
        unsigned int tc2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, RLHT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & RLHT_CAP_TC_SELECT) == 0)
        {
            printf("RLHT tc command unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%u %u", &tc1, &tc2) != 2)
            return -1;
        return rlht_send_set_tc_select(dev, (uint8_t)tc1, (uint8_t)tc2);
    }

    if (strcmp(sub, "duty") == 0)
    {
        unsigned int d1;
        unsigned int d2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, RLHT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & RLHT_CAP_OPEN_DUTY_CONTROL) == 0)
        {
            printf("RLHT duty command unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%u %u", &d1, &d2) != 2)
            return -1;
        return rlht_send_set_open_duty(dev, (uint8_t)d1, (uint8_t)d2);
    }

    return -1;
}

static int cmd_dcmt(const device_info_t *info, const char *rest)
{
    const crumbs_device_t *dev = &info->dev;
    char sub[32];
    int rc;

    rest = next_token(rest, sub, sizeof(sub));
    if (sub[0] == '\0')
    {
        printf("Usage: dcmt <idx|@addr> <state|version|caps|open|brake|mode|setpoint|pid> ...\n");
        return -1;
    }

    if (strcmp(sub, "state") == 0)
    {
        dcmt_state_result_t s;
        rc = dcmt_get_state(dev, &s);
        if (rc != 0)
            return rc;

        printf("DCMT state: mode=%s t1=%d t2=%d v1=%d v2=%d brakes=0x%02X estop=%u\n",
               dcmt_mode_name(s.mode),
               s.target1, s.target2, s.value1, s.value2,
               s.brakes, s.estop);
        return 0;
    }

    if (strcmp(sub, "version") == 0)
    {
        dcmt_version_result_t v;
        rc = dcmt_get_version(dev, &v);
        if (rc != 0)
            return rc;
        printf("DCMT version: crumbs=%u fw=%u.%u.%u\n", v.crumbs_version, v.fw_major, v.fw_minor, v.fw_patch);
        return 0;
    }

    if (strcmp(sub, "caps") == 0)
    {
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        int caps_queried = query_caps_for_type(dev, DCMT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        printf("DCMT caps: schema=%u level=%u flags=0x%08lX source=%s\n",
               (unsigned int)caps_schema,
               (unsigned int)caps_level,
               (unsigned long)caps_flags,
               caps_queried ? "queried" : "fallback");
        return 0;
    }

    if (strcmp(sub, "open") == 0)
    {
        int m1;
        int m2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, DCMT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & DCMT_CAP_OPEN_LOOP_CONTROL) == 0)
        {
            printf("DCMT open command unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%d %d", &m1, &m2) != 2)
            return -1;
        return dcmt_send_set_open_loop(dev, (int16_t)m1, (int16_t)m2);
    }

    if (strcmp(sub, "brake") == 0)
    {
        unsigned int b1;
        unsigned int b2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, DCMT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & DCMT_CAP_BRAKE_CONTROL) == 0)
        {
            printf("DCMT brake command unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%u %u", &b1, &b2) != 2)
            return -1;
        return dcmt_send_set_brake(dev, (uint8_t)b1, (uint8_t)b2);
    }

    if (strcmp(sub, "mode") == 0)
    {
        char tok[32];
        uint8_t mode;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, DCMT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        rest = next_token(rest, tok, sizeof(tok));
        if (tok[0] == '\0')
            return -1;

        if (strcmp(tok, "open") == 0 || strcmp(tok, "0") == 0)
            mode = DCMT_MODE_OPEN_LOOP;
        else if (strcmp(tok, "pos") == 0 || strcmp(tok, "1") == 0)
            mode = DCMT_MODE_CLOSED_POSITION;
        else if (strcmp(tok, "speed") == 0 || strcmp(tok, "2") == 0)
            mode = DCMT_MODE_CLOSED_SPEED;
        else
            return -1;

        if (mode == DCMT_MODE_CLOSED_POSITION && (caps_flags & DCMT_CAP_CLOSED_LOOP_POSITION) == 0)
        {
            printf("DCMT closed position mode unsupported by reported capabilities.\n");
            return -2;
        }
        if (mode == DCMT_MODE_CLOSED_SPEED && (caps_flags & DCMT_CAP_CLOSED_LOOP_SPEED) == 0)
        {
            printf("DCMT closed speed mode unsupported by reported capabilities.\n");
            return -2;
        }

        return dcmt_send_set_mode(dev, mode);
    }

    if (strcmp(sub, "setpoint") == 0)
    {
        int t1;
        int t2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, DCMT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & (DCMT_CAP_CLOSED_LOOP_POSITION | DCMT_CAP_CLOSED_LOOP_SPEED)) == 0)
        {
            printf("DCMT setpoint unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%d %d", &t1, &t2) != 2)
            return -1;
        return dcmt_send_set_setpoint(dev, (int16_t)t1, (int16_t)t2);
    }

    if (strcmp(sub, "pid") == 0)
    {
        unsigned int kp1, ki1, kd1, kp2, ki2, kd2;
        uint8_t caps_schema = 0;
        uint8_t caps_level = 0;
        uint32_t caps_flags = 0;
        query_caps_for_type(dev, DCMT_TYPE_ID, &caps_schema, &caps_level, &caps_flags);
        if ((caps_flags & DCMT_CAP_PID_TUNING) == 0)
        {
            printf("DCMT pid tuning unsupported by reported capabilities.\n");
            return -2;
        }
        if (sscanf(rest, "%u %u %u %u %u %u", &kp1, &ki1, &kd1, &kp2, &ki2, &kd2) != 6)
            return -1;
        return dcmt_send_set_pid(dev,
                                 (uint8_t)kp1, (uint8_t)ki1, (uint8_t)kd1,
                                 (uint8_t)kp2, (uint8_t)ki2, (uint8_t)kd2);
    }

    return -1;
}

int main(int argc, char **argv)
{
    const char *i2c_path = "/dev/i2c-1";
    crumbs_context_t ctx;
    crumbs_linux_i2c_t lw;
    char line[MAX_CMD_LEN];

    if (argc > 1)
        i2c_path = argv[1];

    if (crumbs_linux_init_controller(&ctx, &lw, i2c_path, 100000) != 0)
    {
        fprintf(stderr, "Failed to init I2C controller on %s\n", i2c_path);
        return 1;
    }

    printf("BREAD discovery controller on %s\n", i2c_path);
    print_help();

    while (1)
    {
        char cmd[32];
        const char *args;
        device_info_t *dev_info;
        const char *rest;
        int rc;

        printf("bread> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        trim_whitespace(line);
        if (line[0] == '\0')
            continue;

        args = next_token(line, cmd, sizeof(cmd));

        if (strcmp(cmd, "help") == 0)
        {
            print_help();
            continue;
        }
        if (strcmp(cmd, "scan") == 0)
        {
            cmd_scan(&ctx, &lw);
            continue;
        }
        if (strcmp(cmd, "list") == 0)
        {
            cmd_list();
            continue;
        }
        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0)
        {
            break;
        }

        if (strcmp(cmd, "rlht") == 0)
        {
            if (resolve_device(RLHT_TYPE_ID, args, &dev_info, &rest) != 0)
                continue;
            rc = cmd_rlht(dev_info, rest);
            if (rc != 0)
                printf("RLHT command failed (%d)\n", rc);
            continue;
        }

        if (strcmp(cmd, "dcmt") == 0)
        {
            if (resolve_device(DCMT_TYPE_ID, args, &dev_info, &rest) != 0)
                continue;
            rc = cmd_dcmt(dev_info, rest);
            if (rc != 0)
                printf("DCMT command failed (%d)\n", rc);
            continue;
        }

        printf("Unknown command: %s\n", cmd);
    }

    crumbs_linux_close(&lw);
    return 0;
}

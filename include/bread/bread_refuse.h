#ifndef BREAD_REFUSE_H
#define BREAD_REFUSE_H

/*
 * Reason bytes a BREADS-compatible Slice passes to crumbs_refuse(ctx,
 * reason) when it declines a SET. GET_ACK reports them with status
 * CRUMBS_ACK_REFUSED; CRUMBS leaves the byte's meaning to the family, and
 * these are the meanings every family shares.
 *
 * 0x00 is never a refusal reason: CRUMBS reports it for an applied
 * command. 0x80-0xFF are left to the families for reasons only they have.
 * A released value never changes meaning; new reasons take new values.
 *
 * Applied means every part of the frame was acted on as sent. A frame with
 * parts the firmware can act on and parts it cannot, such as a DCMT
 * SET_BRAKE that engages one brake and asks to release the other while a
 * watchdog trip is held, applies the parts it can and is refused with the
 * reason of the first ignored part in payload order. A refusal therefore
 * does not mean nothing changed: the controller reads GET_STATE to see what
 * took effect.
 */

/* The payload does not decode: it is shorter than the opcode's wire size,
   or, for an opcode the contract fixes at an exact length (DCMT SET_PID,
   CLEAR_WATCHDOG_TRIP), it is any other length. A longer payload for any
   other opcode is not malformed: CRUMBS *_unpack reads the fields from the
   front and ignores trailing bytes, which is how a layout is extended
   compatibly (crumbs_ops.h). */
#define BREAD_REFUSE_MALFORMED 0x01

/* A command-watchdog trip is held, and the command could start motion or
   heat. */
#define BREAD_REFUSE_WATCHDOG_TRIP 0x02

/* A hardware e-stop is held, and the command could start motion or heat.
   On firmware that advertises *_CAP_ESTOP_HOLD, this is the reason when an
   e-stop and a watchdog trip are both held: clearing the trip does not lift
   the e-stop hold. */
#define BREAD_REFUSE_ESTOP 0x03

/* A field holds a value the contract does not define, such as a DCMT
   SET_PID loop other than POSITION or SPEED, or a value outside the field's
   stated range (RLHT_OPEN_DUTY_MAX_PCT, RLHT_PERIOD_MIN_MS/MAX_MS,
   DCMT_PWM_MAX). */
#define BREAD_REFUSE_UNDEFINED_VALUE 0x04

/* A value the contract defines but this build leaves out, such as the DCMT
   speed loop with DCMT_ENABLE_SPEED_LOOP=0. GET_CAPS says what a build
   supports. */
#define BREAD_REFUSE_NOT_BUILT 0x05

/* The command does not apply in the current mode, such as an open-loop
   duty outside open loop. */
#define BREAD_REFUSE_WRONG_MODE 0x06

/* An opcode that is not a command of the family or of bread-common. That
   includes a GET opcode sent as a command frame: a GET is a query, read
   through SET_REPLY. CRUMBS rejects such a frame itself only when no
   on_message callback takes it. */
#define BREAD_REFUSE_UNKNOWN_OP 0x07

#endif /* BREAD_REFUSE_H */

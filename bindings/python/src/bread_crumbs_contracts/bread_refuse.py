"""Reason bytes a slice refuses a SET with (``bread_refuse.h``).

GET_ACK reports one with status ``CRUMBS_ACK_REFUSED``. 0x00 is never a
refusal reason, since CRUMBS reports it for an applied command, and
0x80-0xFF are left to the families for reasons only they have.
"""

__all__ = [
    "BREAD_REFUSE_ESTOP",
    "BREAD_REFUSE_MALFORMED",
    "BREAD_REFUSE_NOT_BUILT",
    "BREAD_REFUSE_UNDEFINED_VALUE",
    "BREAD_REFUSE_UNKNOWN_OP",
    "BREAD_REFUSE_WATCHDOG_TRIP",
    "BREAD_REFUSE_WRONG_MODE",
]

#: The payload does not decode: shorter than the opcode's wire size, or, for
#: an opcode the contract fixes at an exact length (DCMT SET_PID,
#: CLEAR_WATCHDOG_TRIP), any other length. A longer payload for any other
#: opcode is not malformed.
BREAD_REFUSE_MALFORMED = 0x01

#: A command-watchdog trip is held, and the command could start motion or heat.
BREAD_REFUSE_WATCHDOG_TRIP = 0x02

#: A hardware e-stop is held, and the command could start motion or heat. On
#: firmware that advertises ``*_CAP_ESTOP_HOLD``, this is the reason when an
#: e-stop and a watchdog trip are both held.
BREAD_REFUSE_ESTOP = 0x03

#: A field holds a value the contract does not define, including a value
#: outside the field's stated range.
BREAD_REFUSE_UNDEFINED_VALUE = 0x04

#: A value the contract defines but this build leaves out; GET_CAPS says what
#: a build supports.
BREAD_REFUSE_NOT_BUILT = 0x05

#: The command does not apply in the current mode.
BREAD_REFUSE_WRONG_MODE = 0x06

#: An opcode that is not a command of the family or of bread-common,
#: including a GET opcode sent as a command frame instead of through SET_REPLY.
BREAD_REFUSE_UNKNOWN_OP = 0x07

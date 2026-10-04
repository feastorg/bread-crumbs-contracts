"""Pure-Python codec for the BREAD CRUMBS contracts.

Mirrors ``include/bread/*.h``: one module per header, functions named after
the C helpers. Encoders return the payload bytes a ``*_send_*`` or
``*_query_*`` helper puts on the wire; parsers turn a reply payload into a
frozen dataclass named after the C result struct. Framing, CRC and I2C
belong to the transport.

The constants and every encoder and parser are checked against
``tests/golden_vectors/vectors.json``, which the C program
``tests/golden_vectors/gen_vectors.c`` writes from the real headers.
"""

from . import bread_caps, bread_version_helpers, bread_watchdog, crumbs, dcmt_ops, rlht_ops
from .bread_caps import *  # noqa: F403
from .bread_version_helpers import *  # noqa: F403
from .bread_watchdog import *  # noqa: F403
from .crumbs import *  # noqa: F403
from .dcmt_ops import *  # noqa: F403
from .rlht_ops import *  # noqa: F403

__all__: list[str] = []
__all__ += crumbs.__all__
__all__ += bread_caps.__all__
__all__ += bread_watchdog.__all__
__all__ += bread_version_helpers.__all__
__all__ += rlht_ops.__all__
__all__ += dcmt_ops.__all__

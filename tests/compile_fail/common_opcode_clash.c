/* Must fail to compile with: duplicate case value */
/* A family opcode equal to a bread-common opcode (CLASH_OPCODE, set by
   CMakeLists.txt to each of 0x7C-0x7F in turn) must not build. */
#include "bread/bread_common_ops.h"
#include "crumbs_ops.h"
#define FAKE_OPS(X) X(FAKE_OP_SET, 0x01) X(FAKE_OP_CLASH, CLASH_OPCODE) X(FAKE_OP_GET_STATE, 0x80)
CRUMBS_DEFINE_FAMILY(FAKE, 0x09, FAKE_OPS)
BREAD_CHECK_FAMILY_OPS(FAKE, FAKE_OPS)
int main(void) { return 0; }

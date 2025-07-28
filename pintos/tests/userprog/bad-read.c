/* This program attempts to read memory at an address that is not mapped.
   This should terminate the process with a -1 exit code. */

#include "tests/lib.h"
#include "tests/main.h"

void test_main(void) {
    msg("Congratulations - you have successfully dereferenced NULL: %d", *(int *)NULL);
    fail("should have exited with -1");
}
/*
msg() 에서 *(int *)NULL 에 접근하려는 순간
페이지 폴트 발생 -> exit(-1) 해야함
*/

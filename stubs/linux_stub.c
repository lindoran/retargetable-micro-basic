/*
 * linux_stub.c — Linux target stub for Hosted HAL.
 * Each platform target keeps a stub so that specific HAL implementations
 * (stdio-based here) can be swapped without editing BASIC_STAGE1.c.
 */

#include "../io_stdio.c"
#include "../hal_hosted.c"

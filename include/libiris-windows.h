#pragma once

#include <stdint.h>

/* 4-byte integer in the form 0xAABBCC where AA is the major number
 * (incremented when breaking APIs), BB is the minor number
 * (incremented when modifying features), and CC is the bugfix number
 * (incremented on every single release).
 */
#define LIBIRIS_VERSION 0x010101

uint32_t libiris_get_version();

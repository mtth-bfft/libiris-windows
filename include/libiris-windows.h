#pragma once
#include <stdint.h>

/* 4-byte integer in the form 0xAABBCC where AA is the major number
 * (incremented when breaking APIs), BB is the minor number
 * (incremented when modifying features), and CC is the bugfix number
 * (incremented on every single release).
 */
#define LIBIRIS_VERSION 0x010101

#ifndef _LIBIRIS_POLICY_H
typedef void libiris_policy_t;
#endif

#ifndef _LIBIRIS_INSTANTIATE_H
typedef void libiris_process_t;
#endif

uint32_t libiris_get_version();

int libiris_policy_new(libiris_policy_t **out);

int libiris_policy_free(libiris_policy_t *pol);

int libiris_instance_new(libiris_policy_t *policy, const char *commandline, libiris_process_t **out);

int libiris_instance_wait(libiris_process_t *proc, uint64_t timeout_ms);

int libiris_instance_free(libiris_process_t *proc);

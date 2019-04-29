#pragma once
#define _LIBIRIS_POLICY_H
#include <Windows.h>
#include <stdint.h>

static const uint32_t LIBIRIS_POLICY_MAGIC = 0xC0FF33A0;

typedef struct {
   uint32_t magic;
   uint32_t version;

} libiris_policy_t;

int policy_check(libiris_policy_t *pol);

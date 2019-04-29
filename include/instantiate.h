#pragma once
#define _LIBIRIS_INSTANTIATE_H
#include <stdint.h>

static const uint32_t LIBIRIS_PROCESS_MAGIC = 0xC0FF33A1;

typedef struct {
   uint32_t magic;
   uint32_t version;

   LPPROC_THREAD_ATTRIBUTE_LIST pAttrList;
   STARTUPINFOEXA startInfo;
   PROCESS_INFORMATION procInfo;
} libiris_process_t;

#pragma once
#define _LIBIRIS_INSTANTIATE_H
#include <Windows.h>
#include <stdint.h>

static const uint32_t LIBIRIS_PROCESS_MAGIC = 0xC0FF33A1;

typedef struct {
   uint32_t magic;
   uint32_t version;

   // Fields only initialized when running on Windows >= 8
   PWSTR szAppContainerName;
   PSID pAppContainerSID;

   STARTUPINFOEXA startInfo;
   PROCESS_INFORMATION procInfo;
} libiris_process_t;

#pragma once
#include <Windows.h>

int safe_alloc(SIZE_T dwBytes, PVOID *pBuffer);
int safe_realloc(SIZE_T dwBytes, PVOID *pBuffer);
int safe_free(PVOID *pBuffer);

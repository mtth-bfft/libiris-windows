#pragma once
#include "instantiate.h"
#include "libiris-windows.h"

BOOL is_appcontainer_compatible();
int allocate_appcontainer_sid(libiris_process_t *pProc);

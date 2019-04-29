#include <Windows.h>
#include "instantiate.h"
#include "policy.h"
#include "mem.h"
#include "libiris-windows.h"

static int process_check(libiris_process_t *pProc)
{
   if (pProc == NULL || pProc->magic != LIBIRIS_PROCESS_MAGIC)
      return ERROR_INVALID_PARAMETER;
   return 0;
}

int libiris_instance_new(libiris_policy_t *policy, const char *commandline, libiris_process_t **out)
{
   int res = 0;
   libiris_process_t *pProc = NULL;
   DWORD dwProcessPolicy = PROCESS_CREATION_CHILD_PROCESS_RESTRICTED;

   if (policy_check(policy) != 0 || commandline == NULL || out == NULL)
      return ERROR_INVALID_PARAMETER;

   res = safe_alloc(sizeof(*pProc), &pProc);
   if (res != 0)
      return res;

   pProc->magic = LIBIRIS_PROCESS_MAGIC;
   pProc->version = LIBIRIS_VERSION;
   pProc->startInfo.StartupInfo.cb = sizeof(pProc->startInfo);

   // TODO: set process and thread attributes security descriptors
   // TODO: add envp as argument
   // TODO: add flags as argument
   // TODO: add cwd as argument?

   if (!CreateProcessA(NULL, (LPSTR)commandline, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, (LPSTARTUPINFOA)&(pProc->startInfo), &(pProc->procInfo)))
   {
      res = GetLastError();
      goto cleanup;
   }

   *out = pProc;

cleanup:
   if (res != 0 && pProc != NULL)
      safe_free(pProc);
   return res;
}

int libiris_instance_wait(libiris_process_t *proc, uint64_t timeout_ms)
{
   int res = 0;
   if (process_check(proc) != 0)
      return ERROR_INVALID_PARAMETER;
   else if (timeout_ms >= MAXDWORD)
      return ERROR_ARITHMETIC_OVERFLOW;

   if (timeout_ms == 0)
      timeout_ms = INFINITE;

   res = WaitForSingleObject(proc->procInfo.hProcess, (DWORD)timeout_ms);
   if (res != WAIT_OBJECT_0)
      return res;

   return 0;
}

int libiris_instance_free(libiris_process_t *proc)
{
   if (process_check(proc) != 0)
      return ERROR_INVALID_PARAMETER;

   CloseHandle(proc->procInfo.hProcess);
   CloseHandle(proc->procInfo.hThread);
   return safe_free(proc);
}
#include <Windows.h>
#include <tchar.h>
#include <userenv.h>
#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>

#include "instantiate.h"
#include "policy.h"
#include "appcontainer.h"
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
   SIZE_T dwBufLen = 0;
   DWORD dwProcessPolicy = PROCESS_CREATION_CHILD_PROCESS_OVERRIDE;
   SECURITY_CAPABILITIES caps = { 0 };
   PSTR szChdir = NULL;

   if (policy_check(policy) != 0 || commandline == NULL || out == NULL)
      return ERROR_INVALID_PARAMETER;

   res = safe_alloc(sizeof(*pProc), &pProc);
   if (res != 0)
      goto cleanup;

   pProc->magic = LIBIRIS_PROCESS_MAGIC;
   pProc->version = LIBIRIS_VERSION;
   pProc->startInfo.StartupInfo.cb = sizeof(pProc->startInfo);

   // TODO: refactor everything to be UTF-8 compliant: ConvertMultiByteToWide() on input, then replace all functions with their TCHAR equivalents
   // TODO: set process and thread attributes security descriptors
   // TODO: add envp as argument
   // TODO: add flags as argument
   // TODO: add check for process creation in policy, use PROCESS_CREATION_CHILD_PROCESS_RESTRICTED by default

   InitializeProcThreadAttributeList(NULL, 2, 0, &dwBufLen);
   res = safe_alloc(dwBufLen, &(pProc->startInfo.lpAttributeList));
   if (res != 0)
      goto cleanup;
   if (!InitializeProcThreadAttributeList(pProc->startInfo.lpAttributeList, 2, 0, &dwBufLen))
   {
      res = GetLastError();
      printf(" [!] InitializeProcThreadAttributeList(): code %u\n", res);
      goto cleanup;
   }

   if (!UpdateProcThreadAttribute(pProc->startInfo.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_CHILD_PROCESS_POLICY, &dwProcessPolicy, sizeof(dwProcessPolicy), NULL, NULL))
   {
      res = GetLastError();
      printf(" [!] UpdateProcThreadAttribute(): code %u\n", res);
      goto cleanup;
   }

   if (is_appcontainer_compatible())
   {
      res = allocate_appcontainer_sid(pProc);
      if (res != 0)
         goto cleanup;
      caps.CapabilityCount = 0;
      caps.AppContainerSid = pProc->pAppContainerSID;
      if (!UpdateProcThreadAttribute(pProc->startInfo.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES, &caps, sizeof(caps), NULL, NULL))
      {
         res = GetLastError();
         printf(" [!] UpdateProcThreadAttribute(PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES): code %u\n", res);
         goto cleanup;
      }
   }
   else
   {
      res = ERROR_NOT_SUPPORTED;
      printf(" [!] Legacy sandboxing isn't supported yet. Please upgrade your OS to Windows 10.\n");
      goto cleanup;
   }

   // TODO: add cwd as argument, default to %SystemRoot%
   // Set the process' current directory to one it has access to (e.g. cmd.exe refuses to run if it can't stat() its cwd)
   dwBufLen = ExpandEnvironmentStringsA("%SystemRoot%\\", NULL, 0);
   res = safe_alloc(dwBufLen, &szChdir);
   if (res != 0)
      goto cleanup;
   dwBufLen = ExpandEnvironmentStringsA("%SystemRoot%\\", szChdir, (DWORD)dwBufLen);

   printf(" [.] New process starts in %s\n", szChdir);

   if (!CreateProcessA(NULL, (LPSTR)commandline, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT | CREATE_NEW_CONSOLE | INHERIT_PARENT_AFFINITY, NULL, szChdir, (LPSTARTUPINFOA)&(pProc->startInfo), &(pProc->procInfo)))
   {
      res = GetLastError();
      printf(" [!] CreateProcessA(): code %u\n", res);
      goto cleanup;
   }
   printf(" [.] Child process PID: %u\n", pProc->procInfo.dwProcessId);

   *out = pProc;

cleanup:
   if (res != 0 && pProc != NULL)
      libiris_instance_free(pProc);
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

   if (proc->szAppContainerName != NULL)
   {
      HRESULT hRes = DeleteAppContainerProfile(proc->szAppContainerName);
      if (hRes != S_OK)
      {
         printf(" [!] Failed to cleanup AppContainer profile: code %u\n", HRESULT_CODE(hRes));
      }
   }

   if (proc->startInfo.lpAttributeList != NULL)
   {
      DeleteProcThreadAttributeList(proc->startInfo.lpAttributeList);
      safe_free(&(proc->startInfo.lpAttributeList));
   }
   if (proc->procInfo.hProcess != NULL && proc->procInfo.hProcess != INVALID_HANDLE_VALUE)
      CloseHandle(proc->procInfo.hProcess);
   if (proc->procInfo.hThread != NULL && proc->procInfo.hThread != INVALID_HANDLE_VALUE)
      CloseHandle(proc->procInfo.hThread);
   return safe_free(&(proc));
}
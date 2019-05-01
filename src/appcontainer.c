#define _CRT_RAND_S
#include <Windows.h>
#include <userenv.h>
#include <sddl.h>
#include <stdio.h>
#include <VersionHelpers.h>
#include "appcontainer.h"
#include "mem.h"
#include "libiris-windows.h"

BOOL is_appcontainer_compatible()
{
   return IsWindows8OrGreater();
}

int allocate_appcontainer_sid(libiris_process_t *pProc)
{
   int res = 0;
   HRESULT hRes = S_OK;
   PSID pAppContainerSID = NULL;
   PSTR szAppContainerSID = NULL;

   res = safe_alloc(sizeof(WCHAR) * 64, &(pProc->szAppContainerName));
   if (res != 0)
      return res;

   do
   {
      unsigned int randPart1 = 0;
      unsigned int randPart2 = 0;
      uint64_t instanceID = 0;
      errno_t err = rand_s(&randPart1);
      if (err != 0)
      {
         res = err;
         printf(" [!] rand_s() secure number generation failed with code %u\n", res);
         goto cleanup;
      }
      err = rand_s(&randPart2);
      if (err != 0)
      {
         res = err;
         printf(" [!] rand_s() secure number generation failed with code %u\n", res);
         goto cleanup;
      }
      instanceID = (((uint64_t)randPart1) << 32) | randPart2;
      swprintf_s(pProc->szAppContainerName, 64, TEXT("libiris-instance-%020llx"), instanceID);

      printf(" [.] AppContainer name: '%ws'\n", pProc->szAppContainerName);

      hRes = CreateAppContainerProfile(pProc->szAppContainerName, pProc->szAppContainerName, TEXT("Sandboxed process created by libiris"), NULL, 0, &pAppContainerSID);
      if (hRes != S_OK)
      {
         res = HRESULT_CODE(hRes);
         if (res == ERROR_ALREADY_EXISTS)
            continue;
         printf(" [!] CreateAppContainerProfile(): code %u\n", res);
         goto cleanup;
      }

      res = safe_alloc(MAX_SID_SIZE, &(pProc->pAppContainerSID));
      if (res != 0)
         goto cleanup;

      if (!CopySid(MAX_SID_SIZE, pProc->pAppContainerSID, pAppContainerSID))
      {
         res = GetLastError();
         printf(" [!] CopySid() failed on AppContainer SID with code %u\n", res);
         goto cleanup;
      }
      break;
   } while (res == ERROR_ALREADY_EXISTS);

   if (res != 0)
      goto cleanup;

   if (!ConvertSidToStringSidA(pProc->pAppContainerSID, &szAppContainerSID))
   {
      res = GetLastError();
      printf(" [!] ConvertSidToStringSidA(): code %u\n", res);
      goto cleanup;
   }
   printf(" [.] AppContainer SID: %s\n", szAppContainerSID);

cleanup:
   if (res != 0 && pProc->szAppContainerName != NULL)
      safe_free(&(pProc->szAppContainerName));
   if (res != 0 && pProc->pAppContainerSID != NULL)
      safe_free(&(pProc->pAppContainerSID));
   if (pAppContainerSID != NULL)
      FreeSid(pAppContainerSID);
   return res;
}
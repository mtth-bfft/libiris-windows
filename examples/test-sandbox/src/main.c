#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <libiris-windows.h>

int main()
{
   int res = 0;
   uint32_t ver = libiris_get_version();
   uint8_t major = (ver & 0xFF0000) >> 16;
   uint8_t minor = (ver & 0x00FF00) >> 8;
   uint8_t bugfix = (ver & 0x0000FF);
   libiris_policy_t *pPolicy = NULL;
   libiris_process_t *pProcess = NULL;
   printf(" [.] Starting test-sandbox with libiris %u.%u.%u\n", major, minor, bugfix);
   
   res = libiris_policy_new(&pPolicy);
   if (res != 0)
   {
      _ftprintf(stderr, TEXT(" [!] Error: unable to initialize empty sandbox policy, code %d\n"), res);
      goto cleanup;
   }

   res = libiris_instance_new(pPolicy, "C:\\Windows\\System32\\cmd.exe", &pProcess);
   if (res != 0)
   {
      _ftprintf(stderr, TEXT(" [!] Error: unable to create sandboxed child process, code %d\n"), res);
      goto cleanup;
   }

   printf(" [.] Child process created, waiting for it to exit...\n");

   res = libiris_instance_wait(pProcess, 0);
   if (res != 0)
   {
      _ftprintf(stderr, TEXT(" [!] Error: unable to wait for sandboxed process to exit, code %d\n"), res);
      goto cleanup;
   }

   printf(" [.] Child process exited\n");

cleanup:
   if (pProcess != NULL)
   {
      res = libiris_instance_free(pProcess);
      if (res != 0)
      {
         _ftprintf(stderr, TEXT(" [!] Error: unable to free sandboxed instance, code %d\n"), res);
      }
   }
   if (pPolicy != NULL)
   {
      res = libiris_policy_free(pPolicy);
      if (res != 0)
      {
         _ftprintf(stderr, TEXT(" [!] Error: unable to free sandbox policy, code %d\n"), res);
      }
   }
   printf(" [.] Exiting with code %d\n", res);
   return 0;
}
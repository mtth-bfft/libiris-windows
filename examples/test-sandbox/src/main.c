#include <stdio.h>
#include <libiris-windows.h>

int main()
{
   int res = 0;
   uint32_t ver = libiris_get_version();
   uint8_t major = (ver & 0xFF0000) >> 16;
   uint8_t minor = (ver & 0x00FF00) >> 8;
   uint8_t bugfix = (ver & 0x0000FF);
   printf(" [.] Starting test-sandbox with libiris %u.%u.%u\n", major, minor, bugfix);
   

   printf(" [.] Exiting with code %d\n", res);
   return 0;
}
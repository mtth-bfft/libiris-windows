#include "mem.h"

static HANDLE hPrivateHeap = NULL;

static int init_heap()
{
   HANDLE hNewHeap = NULL;

   if (hPrivateHeap != NULL)
      return 0;

   hNewHeap = HeapCreate(0, 0, 0);
   if (hNewHeap == NULL)
      return GetLastError();
   if (InterlockedCompareExchangePointer(&hPrivateHeap, hNewHeap, NULL) != NULL)
   {
      // We were raced by another thread doing a memory allocation.
      // No need to allocate anymore, trash our new useless heap.
      if (!HeapDestroy(hNewHeap))
         return GetLastError();
   }
   return 0;
}

int safe_alloc(SIZE_T dwBytes, PVOID *pBuffer)
{
   int res = 0;
   PVOID pRes = NULL;

   res = init_heap();
   if (res != 0)
      return res;
   
   if (pBuffer == NULL)
      return ERROR_INVALID_PARAMETER;

   pRes = HeapAlloc(hPrivateHeap, HEAP_ZERO_MEMORY, dwBytes);
   if (pRes == NULL)
      return GetLastError();

   *pBuffer = pRes;
   return 0;
}

int safe_realloc(SIZE_T dwBytes, PVOID *pBuffer)
{
   int res = 0;
   PVOID pRes = NULL;

   res = init_heap();
   if (res != 0)
      return res;

   if (pBuffer == NULL)
      return ERROR_INVALID_PARAMETER;

   if (*pBuffer == NULL)
      return safe_alloc(dwBytes, pBuffer);

   pRes = HeapReAlloc(hPrivateHeap, HEAP_ZERO_MEMORY, *pBuffer, dwBytes);
   if (pRes == NULL)
      return GetLastError();

   *pBuffer = pRes;
   return 0;
}

int safe_free(PVOID pBuffer)
{
   int res = init_heap();
   if (res != 0)
      return res;

   if (!HeapFree(hPrivateHeap, 0, pBuffer))
      return GetLastError();
   return 0;
}
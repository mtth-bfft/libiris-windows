#include "policy.h"
#include "mem.h"
#include "libiris-windows.h"

int libiris_policy_new(libiris_policy_t **out)
{
   int res = 0;
   libiris_policy_t *pRes = NULL;
   
   if (out == NULL)
      return ERROR_INVALID_PARAMETER;

   res = safe_alloc(sizeof(*pRes), &pRes);
   if (res != 0)
      return res;

   pRes->magic = LIBIRIS_POLICY_MAGIC;
   pRes->version = LIBIRIS_VERSION;

   return 0;
}

int libiris_policy_free(libiris_policy_t *pol)
{
   if (pol == NULL)
      return ERROR_INVALID_PARAMETER;

   return safe_free(pol);
}

int policy_check(libiris_policy_t *pol)
{
   return (pol != NULL && pol->magic == LIBIRIS_POLICY_MAGIC);
}
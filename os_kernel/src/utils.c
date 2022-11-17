#include "utils.h"


#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

void enable_interrupts()
{
    csr_enable_interrupts();
}

void disable_interrupts()
{
    csr_disable_interrupts();
}

#if defined(__cplusplus)
}
#endif // __cplusplus
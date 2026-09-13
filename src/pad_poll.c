/* Bind the original decoder to the translated controller service.
 * The stack-local context preserves the host SDK user data and is reentrant. */
#include "pad_poll.h"
#include "pad.h"
#include <stdlib.h>
typedef struct RRJPadPollContext { RRJMemory *memory; RRJPadSDK sdk; } RRJPadPollContext;
static void service(RRJMemory *m,uint32_t target,uint32_t player,uint32_t multi)
{
    RRJPadPollContext *context=(RRJPadPollContext *)m->sdk_user;
    if(target!=0x8001DDC4) abort();
    (void)sub_8001DDC4(context->memory,player,multi,context->sdk);
}
uint32_t rrj_pad_poll(RRJMemory *m,RRJPadSDK sdk)
{
    RRJPadPollContext context={m,sdk};
    RRJMemory local=*m;
    local.sdk_user=&context;
    return sub_8001C5F8(&local,service);
}

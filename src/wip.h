#ifndef RRJ_WIP_H
#define RRJ_WIP_H
#include "psx_memory.h"
int rrj_wip_options(int *argc,char **argv,RRJMemory *memory);
void rrj_wip_frame(uint32_t frame);
void rrj_wip_site(RRJMemory *m,uint32_t pc,const char *kind,const char *subsystem,
    const char *function,const char *file,int line,int recoverable,const char *fallback,
    const uint32_t *args,unsigned count);
__declspec(noreturn) void rrj_wip_stop(const char *function,const char *file,int line);
#define RRJ_WIP(m,pc,kind,area,fallback,args,count) \
    rrj_wip_site(m,pc,kind,area,__func__,__FILE__,__LINE__,1,fallback,args,count)
#define RRJ_WIP3(m,pc,area,a,b,c) \
    RRJ_WIP(m,pc,"callee",area,"skip_call_return_zero",((const uint32_t[]){a,b,c}),3)
#endif

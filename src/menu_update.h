#ifndef RRJ_MENU_UPDATE_H
#define RRJ_MENU_UPDATE_H
#include "psx_memory.h"
typedef uint32_t (*RRJMenuUpdateCall)(RRJMemory *,uint32_t target,uint32_t menu);
uint32_t sub_F_800667E4(RRJMemory *,RRJMenuUpdateCall update,RRJSDKCall effect);
#endif

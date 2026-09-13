#ifndef RRJ_RACE_FRAME_H
#define RRJ_RACE_FRAME_H
#include "psx_memory.h"
#include "music_stop.h"
typedef uint32_t (*RRJRaceFrameCall)(RRJMemory *,uint32_t target,const uint32_t args[7]);
uint32_t sub_80011C4C(RRJMemory *,RRJRaceFrameCall);
uint32_t rrj_race_movement_block(RRJMemory *,uint32_t delta);
uint32_t rrj_race_movement_contact_block(RRJMemory *,uint32_t delta);
uint32_t rrj_race_movement_route_block(RRJMemory *,uint32_t delta);
uint32_t rrj_race_movement_surface_block(RRJMemory *,uint32_t delta);
uint32_t rrj_race_movement_placement_block(RRJMemory *,uint32_t delta);
uint32_t rrj_race_movement_wheel_block(RRJMemory *,uint32_t delta);
uint32_t rrj_race_activity_block(RRJMemory *,RRJReverbCall);
uint32_t rrj_race_movement_activity_block(RRJMemory *,uint32_t delta,RRJReverbCall);
uint32_t rrj_race_player_tail(RRJMemory *,uint32_t delta);
uint32_t rrj_race_movement_player_block(RRJMemory *,uint32_t delta,RRJReverbCall);
#endif

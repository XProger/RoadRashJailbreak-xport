#include "wip.h"
/* Native bindings for the main-menu renderer table. */
#include "menu_render.h"
#include "menu_item.h"
#include "menu_hint.h"
#include "menu_center.h"
#include "info_panel.h"
#include "text_id.h"
#include "resource_select.h"
#include <stdio.h>
#include <stdlib.h>
void rrj_render_menu_entry(RRJMemory *m,uint32_t target,uint32_t menu,uint32_t entry,RRJMenuResource resource,RRJVideoCall video)
{
    switch(target) {
    case 0x8006E894:(void)sub_F_8006E894(m,menu,entry,resource);break;
    case 0x8006F764:(void)sub_F_8006F764(m,menu,entry,resource);break;
    case 0x8006E7A4:(void)sub_F_8006E7A4(m,menu,entry,resource,rrj_blink_text);break;
    case 0x8006E8FC:(void)sub_F_8006E8FC(m,menu,entry,resource,rrj_select_menu_image,NULL);break;
    case 0x8006FAC8:(void)sub_F_8006FAC8(m,menu,entry,NULL,NULL);break;
    case 0x8006E4D8:(void)sub_F_8006E4D8(m,menu,entry,video);break;
    default:RRJ_WIP3(m,target,"menu_renderer",menu,entry,0);return;
    }
}

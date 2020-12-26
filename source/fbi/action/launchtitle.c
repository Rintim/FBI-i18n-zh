#include <3ds.h>

#include "action.h"
#include "../resources.h"
#include "../task/uitask.h"
#include "../../core/core.h"

static void action_launch_title_update(ui_view* view, void* data, float* progress, char* text) {
    title_info* info = (title_info*) data;

    Result res = 0;

    if(R_SUCCEEDED(res = APT_PrepareToDoApplicationJump(0, info->titleId, info->mediaType))) {
        u8 param[0x300];
        u8 hmac[0x20];

        res = APT_DoApplicationJump(param, sizeof(param), hmac);
    }

    if(R_FAILED(res)) {
        ui_pop();
        info_destroy(view);

        error_display_res(info, task_draw_title_info, res, "无法启动应用.");
    }
}

static void action_launch_title_onresponse(ui_view* view, void* data, u32 response) {
    if(response == PROMPT_YES) {
        info_display("正在启动", "", false, data, action_launch_title_update, task_draw_title_info);
    }
}

void action_launch_title(linked_list* items, list_item* selected) {
    prompt_display_yes_no("确认", "启动所选应用?", COLOR_TEXT, selected->data, task_draw_title_info, action_launch_title_onresponse);
}

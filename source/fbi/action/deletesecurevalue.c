#include <stdio.h>

#include <3ds.h>

#include "action.h"
#include "../resources.h"
#include "../task/uitask.h"
#include "../../core/core.h"

static void action_delete_secure_value_update(ui_view* view, void* data, float* progress, char* text) {
    title_info* info = (title_info*) data;

    u64 param = ((u64) SECUREVALUE_SLOT_SD << 32) | (info->titleId & 0xFFFFFFF);
    u8 out = 0;
    Result res = FSUSER_ControlSecureSave(SECURESAVE_ACTION_DELETE, &param, sizeof(param), &out, sizeof(out));

    ui_pop();
    info_destroy(view);

    if(R_FAILED(res)) {
        error_display_res(info, task_draw_title_info, res, "無法已刪除安全值");
    } else {
        prompt_display_notify("成功", "已刪除安全值", COLOR_TEXT, info, task_draw_title_info, NULL);
    }
}

static void action_delete_secure_value_onresponse(ui_view* view, void* data, u32 response) {
    if(response == PROMPT_YES) {
        info_display("刪除安全值中", "", false, data, action_delete_secure_value_update, task_draw_title_info);
    }
}

void action_delete_secure_value(linked_list* items, list_item* selected) {
    prompt_display_yes_no("確認", "即將刪除所選程式的安全值，是否繼續？", COLOR_TEXT, selected->data, task_draw_title_info, action_delete_secure_value_onresponse);
}
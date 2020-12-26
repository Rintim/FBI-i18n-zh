#include <stdio.h>

#include <3ds.h>

#include "action.h"
#include "../resources.h"
#include "../task/uitask.h"
#include "../../core/core.h"

static void action_import_seed_update(ui_view* view, void* data, float* progress, char* text) {
    title_info* info = (title_info*) data;

    Result res = http_download_seed(info->titleId);

    ui_pop();
    info_destroy(view);

    if(R_SUCCEEDED(res)) {
        prompt_display_notify("成功", "已导入.", COLOR_TEXT, info, task_draw_title_info, NULL);
    } else {
        error_display_res(info, task_draw_title_info, res, "无法导入种子.");
    }
}

static void action_import_seed_onresponse(ui_view* view, void* data, u32 response) {
    if(response == PROMPT_YES) {
        info_display("正在导入", "", false, data, action_import_seed_update, task_draw_title_info);
    }
}

void action_import_seed(linked_list* items, list_item* selected) {
    prompt_display_yes_no("确认", "导入种子至所选应用?", COLOR_TEXT, selected->data, task_draw_title_info, action_import_seed_onresponse);
}

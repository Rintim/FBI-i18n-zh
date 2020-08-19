#include <stdio.h>

#include <3ds.h>

#include "action.h"
#include "../resources.h"
#include "../task/uitask.h"
#include "../../core/core.h"

static void action_import_secure_value_update(ui_view* view, void* data, float* progress, char* text) {
    title_info* info = (title_info*) data;

    char pathBuf[64];
    snprintf(pathBuf, 64, "/fbi/securevalue/%016llX.dat", info->titleId);

    Result res = 0;

    FS_Path* fsPath = fs_make_path_utf8(pathBuf);
    if(fsPath != NULL) {
        Handle fileHandle = 0;
        if(R_SUCCEEDED(res = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""), *fsPath, FS_OPEN_READ, 0))) {
            u32 bytesRead = 0;
            u64 value = 0;
            if(R_SUCCEEDED(res = FSFILE_Read(fileHandle, &bytesRead, 0, &value, sizeof(u64)))) {
                res = FSUSER_SetSaveDataSecureValue(value, SECUREVALUE_SLOT_SD, (u32) ((info->titleId >> 8) & 0xFFFFF), (u8) (info->titleId & 0xFF));
            }

            FSFILE_Close(fileHandle);
        }

        fs_free_path_utf8(fsPath);
    } else {
        res = R_APP_OUT_OF_MEMORY;
    }

    ui_pop();
    info_destroy(view);

    if(R_SUCCEEDED(res)) {
        prompt_display_notify("成功", "已導入安全值", COLOR_TEXT, info, task_draw_title_info, NULL);
    } else {
        error_display_res(info, task_draw_title_info, res, "無法導入安全值");
    }
}

static void action_import_secure_value_onresponse(ui_view* view, void* data, u32 response) {
    if(response == PROMPT_YES) {
        info_display("正在導入安全值中", "", false, data, action_import_secure_value_update, task_draw_title_info);
    }
}

void action_import_secure_value(linked_list* items, list_item* selected) {
    prompt_display_yes_no("確認", "即將導入所選的安全值到所選的程式中，是否繼續？", COLOR_TEXT, selected->data, task_draw_title_info, action_import_secure_value_onresponse);
}
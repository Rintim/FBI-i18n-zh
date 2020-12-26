#include <malloc.h>
#include <stdio.h>

#include <3ds.h>

#include "resources.h"
#include "section.h"
#include "action/action.h"
#include "task/uitask.h"
#include "../core/core.h"

static list_item delete_pending_title = {"删除未完成的应用", COLOR_TEXT, action_delete_pending_title};
static list_item delete_all_pending_titles = {"删除所有未完成的应用", COLOR_TEXT, action_delete_all_pending_titles};

typedef struct {
    populate_pending_titles_data populateData;

    bool populated;
} pendingtitles_data;

typedef struct {
    linked_list* items;
    list_item* selected;
} pendingtitles_action_data;

static void pendingtitles_action_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item* selected) {
    task_draw_pending_title_info(view, ((pendingtitles_action_data*) data)->selected->data, x1, y1, x2, y2);
}

static void pendingtitles_action_update(ui_view* view, void* data, linked_list* items, list_item* selected, bool selectedTouched) {
    pendingtitles_action_data* actionData = (pendingtitles_action_data*) data;

    if(hidKeysDown() & KEY_B) {
        ui_pop();
        list_destroy(view);

        free(data);

        return;
    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        void(*action)(linked_list*, list_item*) = (void(*)(linked_list*, list_item*)) selected->data;

        ui_pop();
        list_destroy(view);

        action(actionData->items, actionData->selected);

        free(data);

        return;
    }

    if(linked_list_size(items) == 0) {
        linked_list_add(items, &delete_pending_title);
        linked_list_add(items, &delete_all_pending_titles);
    }
}

static void pendingtitles_action_open(linked_list* items, list_item* selected) {
    pendingtitles_action_data* data = (pendingtitles_action_data*) calloc(1, sizeof(pendingtitles_action_data));
    if(data == NULL) {
        error_display(NULL, NULL, "无法分配未完成的应用操作的数据.");

        return;
    }

    data->items = items;
    data->selected = selected;

    list_display("未完成的应用操作", "A: 选择, B: 返回", data, pendingtitles_action_update, pendingtitles_action_draw_top);
}

static void pendingtitles_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item* selected) {
    if(selected != NULL && selected->data != NULL) {
        task_draw_pending_title_info(view, selected->data, x1, y1, x2, y2);
    }
}

static void pendingtitles_update(ui_view* view, void* data, linked_list* items, list_item* selected, bool selectedTouched) {
    pendingtitles_data* listData = (pendingtitles_data*) data;

    if(hidKeysDown() & KEY_B) {
        if(!listData->populateData.finished) {
            svcSignalEvent(listData->populateData.cancelEvent);
            while(!listData->populateData.finished) {
                svcSleepThread(1000000);
            }
        }

        ui_pop();

        task_clear_pending_titles(items);
        list_destroy(view);

        free(listData);
        return;
    }

    if(!listData->populated || (hidKeysDown() & KEY_X)) {
        if(!listData->populateData.finished) {
            svcSignalEvent(listData->populateData.cancelEvent);
            while(!listData->populateData.finished) {
                svcSleepThread(1000000);
            }
        }

        listData->populateData.items = items;
        Result res = task_populate_pending_titles(&listData->populateData);
        if(R_FAILED(res)) {
            error_display_res(NULL, NULL, res, "无法启动未完成的应用列表填充.");
        }

        listData->populated = true;
    }

    if(listData->populateData.finished && R_FAILED(listData->populateData.result)) {
        error_display_res(NULL, NULL, listData->populateData.result, "无法填充未完成的应用列表.");

        listData->populateData.result = 0;
    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        pendingtitles_action_open(items, selected);
        return;
    }
}

void pendingtitles_open() {
    pendingtitles_data* data = (pendingtitles_data*) calloc(1, sizeof(pendingtitles_data));
    if(data == NULL) {
        error_display(NULL, NULL, "无法分配未完成的应用的数据.");

        return;
    }

    data->populateData.finished = true;

    list_display("未完成的应用", "A: 选择, B: 返回, X: 刷新", data, pendingtitles_update, pendingtitles_draw_top);
}

#include <malloc.h>
#include <stdio.h>

#include <3ds.h>

#include "resources.h"
#include "section.h"
#include "action/action.h"
#include "task/uitask.h"
#include "../core/core.h"

static list_item delete_ticket = {"刪除憑據", COLOR_TEXT, action_delete_ticket};
static list_item delete_unused_tickets = {"刪除未使用的憑據", COLOR_TEXT, action_delete_tickets_unused};

typedef struct {
    populate_tickets_data populateData;

    bool populated;
} tickets_data;

typedef struct {
    linked_list* items;
    list_item* selected;
} tickets_action_data;

static void tickets_action_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item* selected) {
    task_draw_ticket_info(view, ((tickets_action_data*) data)->selected->data, x1, y1, x2, y2);
}

static void tickets_action_update(ui_view* view, void* data, linked_list* items, list_item* selected, bool selectedTouched) {
    tickets_action_data* actionData = (tickets_action_data*) data;

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
        linked_list_add(items, &delete_ticket);
        linked_list_add(items, &delete_unused_tickets);
    }
}

static void tickets_action_open(linked_list* items, list_item* selected) {
    tickets_action_data* data = (tickets_action_data*) calloc(1, sizeof(tickets_action_data));
    if(data == NULL) {
        error_display(NULL, NULL, "無法分配憑據操作數據");

        return;
    }

    data->items = items;
    data->selected = selected;

    list_display("憑據操作", "A: 選擇, B: 返回", data, tickets_action_update, tickets_action_draw_top);
}

static void tickets_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item* selected) {
    if(selected != NULL && selected->data != NULL) {
        task_draw_ticket_info(view, selected->data, x1, y1, x2, y2);
    }
}

static void tickets_update(ui_view* view, void* data, linked_list* items, list_item* selected, bool selectedTouched) {
    tickets_data* listData = (tickets_data*) data;

    if(hidKeysDown() & KEY_B) {
        if(!listData->populateData.finished) {
            svcSignalEvent(listData->populateData.cancelEvent);
            while(!listData->populateData.finished) {
                svcSleepThread(1000000);
            }
        }

        ui_pop();

        task_clear_tickets(items);
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
        Result res = task_populate_tickets(&listData->populateData);
        if(R_FAILED(res)) {
            error_display_res(NULL, NULL, res, "無法初始化憑據目錄結構");
        }

        listData->populated = true;
    }

    if(listData->populateData.finished && R_FAILED(listData->populateData.result)) {
        error_display_res(NULL, NULL, listData->populateData.result, "無法列舉憑據目錄");

        listData->populateData.result = 0;
    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        tickets_action_open(items, selected);
        return;
    }
}

void tickets_open() {
    tickets_data* data = (tickets_data*) calloc(1, sizeof(tickets_data));
    if(data == NULL) {
        error_display(NULL, NULL, "無法分配憑據數據");

        return;
    }

    data->populateData.finished = true;

    list_display("憑據", "A: 選擇, B: 返回, X: 刷新", data, tickets_update, tickets_draw_top);
}

// オケー
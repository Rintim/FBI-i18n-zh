#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <3ds.h>
#include <jansson.h>

#include "resources.h"
#include "section.h"
#include "action/action.h"
#include "task/uitask.h"
#include "../core/core.h"

static void update_check_update(ui_view* view, void* data, float* progress, char* text) {
	ui_pop();
    info_destroy(view);
	prompt_display_notify("检查更新", "测试/滚动版不支持检查更新\n仅预览版及稳定版支持更新至最新版本.", COLOR_TEXT, NULL, NULL, NULL);
	return;


    bool hasUpdate = false;
    char updateURL[DOWNLOAD_URL_MAX];

    Result res = 0;

    json_t* json = NULL;
    if(R_SUCCEEDED(res = http_download_json(/*"https://api.github.com/repos/Theopse/fbi-i18n-zh-cn/releases/latest"*/ "https://null", &json, 16 * 1024))) {
        if(json_is_object(json)) {
            json_t* name = json_object_get(json, "name");
            json_t* assets = json_object_get(json, "assets");

            if(json_is_string(name) && json_is_array(assets)) {
                char versionString[16];
                snprintf(versionString, sizeof(versionString), "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);

                if(strncmp(json_string_value(name), versionString, json_string_length(name)) != 0) {
                    const char* url = NULL;

                    for(u32 i = 0; i < json_array_size(assets); i++) {
                        json_t* val = json_array_get(assets, i);
                        if(json_is_object(val)) {
                            json_t* assetName = json_object_get(val, "name");
                            json_t* assetUrl = json_object_get(val, "browser_download_url");

                            if(json_is_string(assetName) && json_is_string(assetUrl)) {
                                if(strncmp(json_string_value(assetName), fs_get_3dsx_path() != NULL ? "FBI.3dsx" : "FBI.cia", json_string_length(assetName)) == 0) {
                                    url = json_string_value(assetUrl);
                                    break;
                                }
                            }
                        }
                    }

                    if(url != NULL) {
                        string_copy(updateURL, url, DOWNLOAD_URL_MAX);
                        hasUpdate = true;
                    } else {
                        res = R_APP_BAD_DATA;
                    }
                }
            } else {
                res = R_APP_BAD_DATA;
            }
        } else {
            res = R_APP_BAD_DATA;
        }

        json_decref(json);
    }

    ui_pop();
    info_destroy(view);

    if(hasUpdate) {
        action_install_url("更新 FBI 至最新版本?", updateURL, fs_get_3dsx_path(), NULL, NULL, NULL, NULL);
    } else {
        if(R_FAILED(res)) {
            error_display_res(NULL, NULL, res, "无法检查更新.");
        } else {
            prompt_display_notify("成功", "无可用的更新.", COLOR_TEXT, NULL, NULL, NULL);
        }
    }
}

void update_open() {
    info_display("正在检查更新", "", false, NULL, update_check_update, NULL);
}

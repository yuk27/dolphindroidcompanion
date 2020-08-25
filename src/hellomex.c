/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hellomex.h"
#include "view.h"
#include "data.h"

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *naviframe;
	Evas_Object *rect[10];
	Eext_Circle_Surface *circle_surface;
	Evas_Object *circle_genlist;
} appdata_s;

static appdata_s *object;
static void win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void _timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}

static void _block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}

static void _popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	elm_popup_dismiss(obj);
}
static void _popup_hide_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	evas_object_del(obj);
}

static void _popup_toast_cb(Evas_Object *parent, char *string)
{
	Evas_Object *popup;


	popup = elm_popup_add(parent);
	elm_object_style_set(popup, "toast/circle");
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _popup_hide_cb, NULL);
	evas_object_smart_callback_add(popup, "dismissed", _popup_hide_finished_cb, NULL);
	elm_object_part_text_set(popup, "elm.text", string);

	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);

	elm_popup_timeout_set(popup, 2.0);

	evas_object_smart_callback_add(popup, "timeout", _timeout_cb, NULL);

	evas_object_show(popup);
}

void update_ui(char *data)
{
	dlog_print(DLOG_INFO, TAG, "Updating UI with data %s", data);
	_popup_toast_cb(object->naviframe, data);
}


typedef struct _item_data {
	int index;
	Elm_Object_Item *item;
} item_data;

static char *_gl_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];

	snprintf(buf, 1023, "%s", "HelloMessage");

	return strdup(buf);
}

static char *_gl_sub_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];

	snprintf(buf, 1023, "%s", "Provider");

	return strdup(buf);
}


static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: Unrealized callback can be called after this.
	// Accessing Item_Data can be dangerous on unrealized callback.
	item_data *id = data;
	if (id) free(id);
}

static Eina_Bool _naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	ui_app_exit();
	return EINA_FALSE;
}

static void create_list_view(appdata_s *ad)
{
	Evas_Object *genlist = NULL;
	Evas_Object *naviframe = ad->naviframe;
	Elm_Object_Item *nf_it = NULL;
	item_data *id = NULL;
	int index = 0;

	Elm_Genlist_Item_Class *titc = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *pitc = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *gic = elm_genlist_item_class_new();

	/* Genlist Title Item Style */
	titc->item_style = "title";
	titc->func.text_get = _gl_title_text_get;
	titc->func.del = _gl_del;

	gic->item_style = "groupindex";
	gic->func.text_get = _gl_sub_title_text_get;
	gic->func.del = _gl_del;

	pitc->item_style = "padding";

	/* Create Genlist */
	genlist = elm_genlist_add(naviframe);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	/* Create Circle Genlist */
	ad->circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);

	/* Set Scroller Policy */
	eext_circle_object_genlist_scroller_policy_set(ad->circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	/* Activate Rotary Event */
	eext_rotary_object_event_activated_set(ad->circle_genlist, EINA_TRUE);

	/* Title Item Here */
	id = calloc(sizeof(item_data), 1);
	elm_genlist_item_append(genlist, titc, NULL, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);

	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, gic, id, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);

	/* Padding Item Here */
	elm_genlist_item_append(genlist, pitc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, ad);

	nf_it = elm_naviframe_item_push(naviframe, NULL, NULL, NULL, genlist, "empty");
	elm_naviframe_item_pop_cb_set(nf_it, _naviframe_pop_cb, ad->win);


}

static void create_base_gui(appdata_s *ad)
{
	Evas_Object *conform = NULL;

	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);

	/* Conformant */
	conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, conform);
	evas_object_show(conform);

	/* Naviframe */
	ad->naviframe = elm_naviframe_add(conform);
	elm_object_content_set(conform, ad->naviframe);

	/* Eext Circle Surface*/
	ad->circle_surface = eext_circle_surface_naviframe_add(ad->naviframe);

	/* Main View */
	create_list_view(ad);

	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static void _btn_down_cb(void *user_data, Evas *e, Evas_Object *obj, void *event_info)
{
	evas_object_color_set(obj, 250, 250, 250, 102);
}

static void _btn_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	evas_object_color_set(obj, 250, 250, 250, 255);

}

static void _left_btn_clicked_cb(){ //void *user_data, Evas_Object *obj, void *event_info){
	dlog_print(DLOG_DEBUG, LOG_TAG, "LEFT ARROW CLICKED");
}
static void _right_btn_clicked_cb(){ //(void *user_data, Evas_Object *obj, void *event_info){

	dlog_print(DLOG_DEBUG, LOG_TAG, "RIGHT ARROW CLICKED");

}
static void _up_btn_clicked_cb(){ //(void *user_data, Evas_Object *obj, void *event_info){(void *user_data, Evas_Object *obj, void *event_info){

	dlog_print(DLOG_DEBUG, LOG_TAG, "UP ARROW CLICKED");

}
static void _down_btn_clicked_cb(){ //(void *user_data, Evas_Object *obj, void *event_info){{ //(void *user_data, Evas_Object *obj, void *event_info){(void *user_data, Evas_Object *obj, void *event_info){

	dlog_print(DLOG_DEBUG, LOG_TAG, "DOWN ARROW CLICKED");
}
static void _a_btn_clicked_cb(){ //(void *user_data, Evas_Object *obj, void *event_info){(void *user_data, Evas_Object *obj, void *event_info){

	dlog_print(DLOG_DEBUG, LOG_TAG, "A BTN CLICKED");

}
static void _b_btn_clicked_cb(){ //(void *user_data, Evas_Object *obj, void *event_info){(void *user_data, Evas_Object *obj, void *event_info){
	boolean b = false;
	dlog_print(DLOG_DEBUG, LOG_TAG, "B BTN CLICKED");

}

static void _content_back_cb(void *user_data, Evas_Object *obj, void *event_info)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "exit application");
}

static bool app_create(void *data)
{

	Evas_Object *conform = NULL;
	Evas_Object *content = NULL;
	char *icon_path = NULL;
	char full_path[PATH_MAX] = { 0, };
	char default_img_path[PATH_MAX] = { 0, };

	view_create(); // create window
	data_get_resource_path(EDJ_FILE, full_path, sizeof(full_path)); // Load the EDJ

	conform = view_get_conformant();
	content = view_create_layout_for_conformant(conform, full_path, GRP_MAIN, _content_back_cb, NULL);
	if (content == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a content.");
		return NULL;
	}

	icon_path = data_get_image("left_arrow");
	view_set_button(content, "left_arrow", "focus", icon_path, NULL, _btn_down_cb, _btn_up_cb, _left_btn_clicked_cb, content);
	view_set_color(content, "left_arrow", 250, 250, 250, 255);
	free(icon_path);

	icon_path = data_get_image("right_arrow");
	view_set_button(content, "right_arrow", "focus", icon_path, NULL, _btn_down_cb, _btn_up_cb, _right_btn_clicked_cb, content);
	view_set_color(content, "right_arrow", 250, 250, 250, 255);
	free(icon_path);

	icon_path = data_get_image("up_arrow");
	view_set_button(content, "up_arrow", "focus", icon_path, NULL, _btn_down_cb, _btn_up_cb, _up_btn_clicked_cb, content);
	view_set_color(content, "up_arrow", 250, 250, 250, 255);
	free(icon_path);

	icon_path = data_get_image("down_arrow");
	view_set_button(content, "down_arrow", "focus", icon_path, NULL, _btn_down_cb, _btn_up_cb, _down_btn_clicked_cb, content);
	view_set_color(content, "down_arrow", 250, 250, 250, 255);
	free(icon_path);

	icon_path = data_get_image("a_btn");
	view_set_button(content, "a_btn", "focus", icon_path, NULL, _btn_down_cb, _btn_up_cb, _a_btn_clicked_cb, content);
	view_set_color(content, "a_btn", 250, 250, 250, 255);
	free(icon_path);

	icon_path = data_get_image("b_btn");
	view_set_button(content, "b_btn", "focus", icon_path, NULL, _btn_down_cb, _btn_up_cb, _b_btn_clicked_cb, content);
	view_set_color(content, "b_btn", 250, 250, 250, 255);
	free(icon_path);

	object = data;
	//create_base_gui(object); //TODO: ADD GUI
	initialize_sap();
	turn_on_screen();
	return TRUE;
}

static void app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void app_terminate(void *data)
{
	/* Release all resources. */
	view_destroy();
	data_finalize();
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[])
{
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, TAG, "ui_app_main() is failed. err = %d", ret);
	}

	return ret;
}

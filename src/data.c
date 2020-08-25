/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dlog.h>
#include <efl_extension.h>
#include <app_common.h>
#include <media_content.h>
#include "data.h"
#include "hellomex.h"

typedef struct _album_data {
	int album_id;
	char *title;
	char *artist;
	char *album_art;
	char *file_path;
	audio_meta_h audio;
} album_data_s;

typedef struct _more_btn_data {
	char *main_txt;
	char *sub_txt;
	char *img_path;
} more_btn_data_s;

static more_btn_data_s more_btn_item[DEVICE_INFO_MAX] = {
	{ "Play music from:", "Gear", "images/More_option/music_more_opt_select_device_gear.png" },
	{ "Play music from:", "Phone", "images/More_option/music_more_opt_select_device_mobile.png" },
};

static struct _s_info {
	filter_h filter;
	media_info_h media_info;
	Eina_List *list;
} s_info = {
	.filter = NULL,
	.list = NULL,
};

static void _destroy_media_info(void);
static void _destroy_media_filter(void);
static void _create_media_filter(void);
static bool _media_item_cb(media_info_h media, void *user_data);
static bool _album_list_cb(media_album_h album, void *user_data);

/*
 * @brief: Get path of resource
 * @param[file_in]: File name
 * @param[file_path_out]: The point to which save full path of the resource
 * @param[file_path_max]: Size of file name include path
 */
void data_get_resource_path(const char *file_in, char *file_path_out, int file_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(file_path_out, file_path_max, "%s%s", res_path, file_in);
		free(res_path);
	}
}

/*
 * @brief: Initialization function for data module
 */
void data_initialize(void)
{
	/*
	 * If you need to initialize managing data,
	 * please use this function.
	 */
}


/*
 * @brief: Get album list
 */
Eina_List *data_get_album_list(void)
{
	return s_info.list;
}

/*
 * @brief: Get album title by index
 * @param[list]: List that contains album information
 * @param[index]: Index in album list
 */
char *data_get_album_title(Eina_List *list, int index)
{
	album_data_s *album_data = eina_list_nth(list, index);
	return album_data->title;
}

/*
 * @brief: Get album artist by index
 * @param[list]: List that contains album information
 * @param[index]: Index in album list
 */
char *data_get_album_artist(Eina_List *list, int index)
{
	album_data_s *album_data = eina_list_nth(list, index);
	return album_data->artist;
}

/*
 * @brief: Get album art by index
 * @param[list]: List that contains album information
 * @param[index]: Index in album list
 */
char *data_get_album_art(Eina_List *list, int index)
{
	album_data_s *album_data = eina_list_nth(list, index);
	return album_data->album_art;
}

/*
 * @brief: Get album file path by index
 * @param[list]: List that contains album information
 * @param[index]: Index in album list
 */
char *data_get_album_file_path(Eina_List *list, int index)
{
	album_data_s *album_data = eina_list_nth(list, index);
	return album_data->file_path;
}

/*
 * @brief: Get device icon image
 * @param[device]: Device type to display
 */
char *data_get_device_image(device_info_e device)
{
	char *file_path = NULL;
	char full_path[PATH_MAX] = { 0, };

	if (device == DEVICE_INFO_GEAR) {
		file_path = "images/Controller/music_gear.png";
	} else {
		file_path = "images/Controller/music_device.png";
	}

	data_get_resource_path(file_path, full_path, (int)PATH_MAX);

	return strdup(full_path);
}

/*
 * @brief: Get a image path
 * @param[part_name]: Part name to which you want to set
 */
char *data_get_image(const char *part_name)
{
	/*
	 * You can use this function to retrieve data.
	 */
	char *file_path = NULL;
	char full_path[PATH_MAX] = { 0, };

	if (!strcmp("left_arrow", part_name)) {
		file_path = "images/Controller/left_arrow.png";
	}
	else if (!strcmp("right_arrow", part_name)) {
		file_path = "images/Controller/right_arrow.png";
	}
	else if (!strcmp("up_arrow", part_name)) {
		file_path = "images/Controller/up_arrow.png";
	}
	else if (!strcmp("down_arrow", part_name)) {
		file_path = "images/Controller/down_arrow.png";
	}
	else if (!strcmp("a_btn", part_name)) {
		file_path = "images/Controller/a_btn.png";
	}
	else if (!strcmp("b_btn", part_name)) {
		file_path = "images/Controller/b_btn.png";
	}
	else {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get image.");
		return NULL;
	}

	data_get_resource_path(file_path, full_path, (int)PATH_MAX);

	return strdup(full_path);
}

/*
 * @brief: Get main text of more button by index
 * @param[index]: The order of more button
 */
char *data_get_more_button_main_text(int index)
{
	return more_btn_item[index].main_txt;
}

/*
 * @brief: Get sub text of more button by index
 * @param[index]: The order of more button
 */
char *data_get_more_button_sub_text(int index)
{
	return more_btn_item[index].sub_txt;
}

/*
 * @brief: Get a more button image path of more button by index
 * @param[index]: The order of more button
 */
char *data_get_more_button_image(int index)
{
	return more_btn_item[index].img_path;
}

/*
 * @brief: Destroy album list data
 */
void data_destroy_album_list(void)
{
	album_data_s *album_data = NULL;

	EINA_LIST_FREE(s_info.list, album_data) {
		free(album_data->title);
		free(album_data->artist);
		free(album_data->album_art);
		free(album_data->file_path);
	}

	s_info.list = NULL;

	_destroy_media_filter();
	_destroy_media_info();
}

/*
 * @brief: Iterates through the media album with optional filter from the media database
 */
Eina_List *data_create_album_list(void)
{
	int ret = MEDIA_CONTENT_ERROR_NONE;

	if (s_info.filter == NULL) {
		_create_media_filter();
	}

	ret = media_album_foreach_album_from_db(s_info.filter, _album_list_cb, NULL);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		_destroy_media_filter();
		return NULL;
	}

	return s_info.list;
}

/*
 * @brief: Update album list data
 */
Eina_List *data_update_album_list(void)
{
	Eina_List *album_list = NULL;

	data_destroy_album_list();
	album_list = data_create_album_list();

	return album_list;
}

/*
 * @brief: Destroys media info
 */
static void _destroy_media_info(void)
{
	if (s_info.media_info == NULL) {
		return;
	}

	if (media_info_destroy(s_info.media_info) != MEDIA_CONTENT_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to destroy media info");
	}
	s_info.media_info = NULL;
}

/*
 * @brief: Destroys a media filter handle
 */
static void _destroy_media_filter(void)
{
	if (s_info.filter == NULL) {
		return;
	}

	if (media_filter_destroy(s_info.filter) != MEDIA_CONTENT_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to destroy filter");
	}
	s_info.filter = NULL;
}

/*
 * @brief: Creates a media filter handle and set condition & order
 */
static void _create_media_filter(void)
{
	char *condition = "MEDIA_TYPE=3"; //0: image, 1: video, 2: sound, 3: music, 4: other
	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_filter_create(&s_info.filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		s_info.filter = NULL;
		return;
	}

	ret = media_filter_set_condition(s_info.filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		_destroy_media_filter();
		return;
	}

	ret = media_filter_set_order(s_info.filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		_destroy_media_filter();
		return;
	}
}

/*
 * @brief: Iterates through the media files with an optional filter from the media database
 * @param[media]: The structure type for the Media info handle
 * @param[user_data]: Data needed in this function
 */
static bool _media_item_cb(media_info_h media, void *user_data)
{
	album_data_s *album_data = user_data;
	char *title = NULL;
	char *path = NULL;
	int ret = MEDIA_CONTENT_ERROR_NONE;
	media_content_type_e media_type = 0;

	ret = media_info_clone(&s_info.media_info, media);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to clone media info");
		return false;
	}

	ret = media_info_get_media_type(media, &media_type);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_info_destroy(s_info.media_info);
		s_info.media_info = NULL;
		return false;
	}

	if (media_type != MEDIA_CONTENT_TYPE_MUSIC) {
		dlog_print(DLOG_ERROR, LOG_TAG, "media type is not music");
		media_info_destroy(s_info.media_info);
		s_info.media_info = NULL;
		return false;
	}

	ret = media_info_get_file_path(media, &path);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_info_destroy(s_info.media_info);
		s_info.media_info = NULL;
		return false;
	}


	ret = media_info_get_title(media, &title);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		media_info_destroy(s_info.media_info);
		s_info.media_info = NULL;
		free(path);
		return false;
	}

	if (title == NULL) {
		ret = media_info_get_display_name(media, &title);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			media_info_destroy(s_info.media_info);
			s_info.media_info = NULL;
			free(path);
			return false;
		}
	}

	album_data->file_path = path ? path : strdup("NULL");
	album_data->title = title ? title : strdup("NULL");

	media_info_destroy(s_info.media_info);
	s_info.media_info = NULL;

	return true;
}

/*
 * @brief: Iterates through the media album with optional filter from the media database
 * @param[album]: The structure type for the Media album handle
 * @param[user_data]: Data needed in this function
 */
static bool _album_list_cb(media_album_h album, void *user_data)
{
	album_data_s *album_data = NULL;
	int album_id = 0;
	int ret = MEDIA_CONTENT_ERROR_NONE;
	char *artist = NULL;
	char *album_art = NULL;

	if (album == NULL) {
		return false;
	}

	if (s_info.filter == NULL) {
		return false;
	}

	album_data = malloc(sizeof(*album_data));

	ret = media_album_get_album_id(album, &album_id);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		_destroy_media_filter();
		free(album_data);
		return false;
	}

	ret = media_album_get_artist(album, &artist);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		_destroy_media_filter();
		free(album_data);
		return false;
	}

	ret = media_album_get_album_art(album, &album_art);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		_destroy_media_filter();
		free(artist);
		free(album_data);
		return false;
	}

	ret = media_album_foreach_media_from_db(album_id, s_info.filter, _media_item_cb, album_data);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		_destroy_media_filter();
		free(artist);
		free(album_art);
		free(album_data);
		return false;
	}

	album_data->album_id = album_id;
	album_data->artist = artist ? artist : strdup("NULL");
	album_data->album_art = album_art ? album_art : strdup("NULL");

	dlog_print(DLOG_DEBUG, LOG_TAG, "[%d] %s - %s (%s, %s)", album_data->album_id, album_data->title, album_data->artist, album_data->album_art, album_data->file_path);

	/*
	 * append album data to album list
	 */
	s_info.list = eina_list_append(s_info.list, album_data);

	return true;
}

/*
 * @brief: Remove a album data in album list
 * @param[file_path]: File path of music
 */
Eina_List *data_remove_album_data_by_file_path(const char *file_path)
{
	Eina_List *l = NULL;
	Eina_List *n = NULL;
	album_data_s *album_data = NULL;

	EINA_LIST_FOREACH_SAFE(s_info.list, l, n, album_data) {
		if (!strcmp(album_data->file_path, file_path)) {
			dlog_print(DLOG_DEBUG, LOG_TAG, "remove album data : %s - %s(%s)", album_data->title, album_data->artist, album_data->file_path);
			s_info.list = eina_list_remove(s_info.list, album_data);

			free(album_data->album_art);
			free(album_data->artist);
			free(album_data->file_path);
			free(album_data->title);
			free(album_data);

			break;
		}
	}

	return s_info.list;
}

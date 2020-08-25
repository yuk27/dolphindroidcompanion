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

#if !defined(_DATA_H)
#define _DATA_H

typedef enum {
	DEVICE_INFO_GEAR = 0,
	DEVICE_INFO_PHONE = 1,
	DEVICE_INFO_MAX,
} device_info_e;

/*
 * Initialize the data component
 */
void data_initialize(void);

/*
 * Finalize the data component
 */
void data_finalize(void);

void data_get_resource_path(const char *file_in, char *file_path_out, int file_path_max);
Eina_List *data_get_album_list(void);
char *data_get_album_title(Eina_List *list, int index);
char *data_get_album_artist(Eina_List *list, int index);
char *data_get_album_art(Eina_List *list, int index);
char *data_get_album_file_path(Eina_List *list, int index);
char *data_get_device_image(device_info_e device);
char *data_get_image(const char *part);
char *data_get_more_button_main_text(int index);
char *data_get_more_button_sub_text(int index);
char *data_get_more_button_image(int index);
void data_destroy_album_list(void);
Eina_List *data_create_album_list(void);
Eina_List *data_update_album_list(void);
Eina_List *data_remove_album_data_by_file_path(const char *file_path);

#endif

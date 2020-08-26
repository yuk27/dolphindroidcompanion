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
#include <glib.h>
#include <sap.h>
#include <sap_message_exchange.h>
#include <sensor.h>
#include <device/power.h>

#define LISTENER_TIMEOUT 0
#define MEX_PROFILE_ID "/sample/hellomessage"
#define KEY_AMNT 7

static struct accel_info {
	float x;
	float y;
	float z;
	char x_array[10];
	char y_array[10];
	char z_array[10];
	char c_x_array[5];
	char c_y_array[5];
	char c_z_array[5];
	char key_pressed[KEY_AMNT];
} a_info = {
	.x = 0,
	.y = 0,
	.z = 0,
	.key_pressed = {0},
	.x_array = {0},
	.y_array = {0},
	.z_array = {0}
};

union {
  float float_variable;
  char bytes_array[5];
} float_convert;

typedef struct _sensor_data {
	sensor_h handle; //returns the handle of the sensor
	sensor_listener_h listener; //you can create various listeners to check on a sensor
} sensor_data_t;
sensor_data_t sensor;

int size = 35; //sizeof(msg) * 8;

char* getButtons(){
	return g_strdup_printf("%s%s%s", a_info.x_array, a_info.y_array, a_info.z_array);
}

void keyReleased(){
	for(int i = 0; i < KEY_AMNT; i++){// Clean the key
		a_info.key_pressed[i] = 'f';
	}
}


void keyPressed(int index){
	a_info.key_pressed[index] = 't';
	dlog_print(DLOG_DEBUG, "PUSH", a_info.key_pressed);
}

char* getAccel(){

	sprintf(a_info.x_array, "%f", a_info.x);
	sprintf(a_info.y_array, "%f", a_info.y);
	sprintf(a_info.z_array, "%f", a_info.z);

	//strncpy(a_info.c_x_array, a_info.x_array, 5);
	//strncpy(a_info.c_y_array, a_info.y_array, 5);
	//strncpy(a_info.c_z_array, a_info.z_array, 5);

	return g_strdup_printf("%s,%s,%s,%s", a_info.x_array, a_info.y_array, a_info.z_array, a_info.key_pressed);
}

void turn_on_screen(){
	power_lock_e type = POWER_LOCK_DISPLAY;
	int timeout_ms = 0;
	int ret = device_power_request_lock(type, timeout_ms);
	if (ret != DEVICE_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] sensor_get_default_sensor() error: %s", __FILE__, __LINE__, get_error_message(ret));
	}
}

void release_screen(){
	power_lock_e type = POWER_LOCK_DISPLAY;
	int ret = device_power_release_lock(type);
	if (ret != DEVICE_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] sensor_get_default_sensor() error: %s", __FILE__, __LINE__, get_error_message(ret));
	}
}

void data_stop_sensor(void)
{
	int ret = sensor_listener_stop(sensor.listener);
	if (ret != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] sensor_listener_stop() error: %s", __FILE__, __LINE__, get_error_message(ret));
		return;
	}
}

void data_start_sensor()
{
	int ret;
	data_stop_sensor();

	ret = sensor_listener_start(sensor.listener);
	if (ret != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] sensor_listener_start() error: %s", __FILE__, __LINE__, get_error_message(ret));
		return;
	}

}

static void _sensor_event_cb(sensor_h sensor, sensor_event_s *event, void *data)
{
	a_info.x = event->values[0];
	a_info.y = event->values[1];
	a_info.z = event->values[2];
}

void data_get_sensor_data(sensor_type_e type)
{
	sensor_event_s event;

	sensor_listener_read_data(sensor.listener, &event);
	a_info.x = event.values[0];
	a_info.y = event.values[1];
	a_info.z = event.values[2];
}

void initialize_sensors(void)
{
	int ret;
	int i = SENSOR_ACCELEROMETER;

	ret = sensor_get_default_sensor(i, &sensor.handle); // Create the handle
	if (ret != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] sensor_get_default_sensor() error: %s", __FILE__, __LINE__, get_error_message(ret));
	}

	ret = sensor_create_listener(sensor.handle, &sensor.listener); //Create the listener
	if (ret != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] sensor_create_listener() error: %s", __FILE__, __LINE__, get_error_message(ret));
	}

	ret = sensor_listener_set_event_cb(sensor.listener, LISTENER_TIMEOUT, _sensor_event_cb, (void *)i); //Set the event when listener is called
	if (ret != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] sensor_listener_set_event_cb() error: %s", __FILE__, __LINE__, get_error_message(ret));
	}

	data_start_sensor();
}

void data_finalize(void)
{
	data_stop_sensor();

	int ret = SENSOR_ERROR_NONE;

	ret = sensor_destroy_listener(sensor.listener);
	if (ret != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] sensor_get_default_sensor() error: %s", __FILE__, __LINE__, get_error_message(ret));
	}

	release_screen();

}

struct priv {
	sap_agent_h agent;
	sap_peer_agent_h peer_agent;
};

gboolean is_agent_added = FALSE;
static struct priv priv_data = { 0 };

void mex_message_delivery_status_cb(sap_peer_agent_h peer_agent_h, int transaction_id, sap_connectionless_transfer_status_e status, void *user_data);

void mex_message_delivery_status_cb(sap_peer_agent_h peer_agent_h, int transaction_id, sap_connectionless_transfer_status_e status, void *user_data)
{
	dlog_print(DLOG_DEBUG, TAG, "sap_pa_message_delivery_status_cb:  transaction_id:%d, status:%d", transaction_id, status);
}

void mex_send(char *message, int length, gboolean is_secured)
{
	int result;
	sap_peer_agent_h pa = priv_data.peer_agent;

	if (sap_peer_agent_is_feature_enabled(pa, SAP_FEATURE_MESSAGE)) {
		result = sap_peer_agent_send_data(pa, (unsigned char *)message, length, is_secured, mex_message_delivery_status_cb, NULL);
		if (result <= 0) {
			dlog_print(DLOG_DEBUG, TAG, "Error in sending mex data");
			free(message);
			dlog_print(DLOG_DEBUG, TAG, "try again or check error val , %d", result);
		}
	} else {
		dlog_print(DLOG_DEBUG, TAG, "MEX is not supported by the Peer framework");
		update_ui("Message feature is not supported by the Peer");
		//Fallback to socket connection
	}

}

void mex_data_received_cb(sap_peer_agent_h peer_agent,unsigned int payload_length,void *buffer, void *user_data)
{
	priv_data.peer_agent = peer_agent;
	char* msg = getAccel();
	mex_send(msg, size, FALSE);
	keyReleased();
	g_free(msg);
}

void on_peer_agent_updated(sap_peer_agent_h peer_agent,
			   sap_peer_agent_status_e peer_status,
			   sap_peer_agent_found_result_e result,
			   void *user_data)
{
	switch (result) {
	case SAP_PEER_AGENT_FOUND_RESULT_DEVICE_NOT_CONNECTED:
		dlog_print(DLOG_DEBUG, TAG, "device is not connected");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_FOUND:

		if (peer_status == SAP_PEER_AGENT_STATUS_AVAILABLE) {
			priv_data.peer_agent = peer_agent;
		} else {
			sap_peer_agent_destroy(peer_agent);
			priv_data.peer_agent = NULL;
		}
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_SERVICE_NOT_FOUND:
		dlog_print(DLOG_DEBUG, TAG, "service not found");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_TIMEDOUT:
		dlog_print(DLOG_DEBUG, TAG, "peer agent find timed out");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_INTERNAL_ERROR:
		dlog_print(DLOG_DEBUG, TAG, "peer agent find search failed");
		break;
	}
}

static gboolean _find_peer_agent()
{
	sap_result_e result = SAP_RESULT_FAILURE;

	result = sap_agent_find_peer_agent(priv_data.agent, on_peer_agent_updated, NULL);

	if (result == SAP_RESULT_SUCCESS) {
		dlog_print(DLOG_DEBUG, TAG, "find peer call succeeded");
	} else {
		dlog_print(DLOG_DEBUG, TAG, "findsap_peer_agent_s is failed (%d)", result);
	}
	dlog_print(DLOG_DEBUG, TAG, "find peer call is over");
	return FALSE;
}

static void on_agent_initialized(sap_agent_h agent,
				 sap_agent_initialized_result_e result,
				 void *user_data)
{
	switch (result) {
	case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:
		dlog_print(DLOG_INFO, TAG, "agent is initialized");

		priv_data.agent = agent;
		sap_agent_set_data_received_cb(agent, mex_data_received_cb, NULL);
		is_agent_added = TRUE;

		_find_peer_agent();
		break;

	case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
		dlog_print(DLOG_DEBUG, TAG, "duplicate registration");
		break;

	case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
		dlog_print(DLOG_DEBUG, TAG, "invalid arguments");
		break;

	case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
		dlog_print(DLOG_DEBUG, TAG, "internal sap error");
		break;

	default:
		dlog_print(DLOG_DEBUG, TAG, "unknown status (%d)", result);
		break;
	}

	dlog_print(DLOG_DEBUG, TAG, "agent initialized callback is over");

}

static void on_device_status_changed(sap_device_status_e status, sap_transport_type_e transport_type,
				     void *user_data)
{
	switch (transport_type) {
	case SAP_TRANSPORT_TYPE_BT:
		dlog_print(DLOG_INFO, TAG, "connectivity type(%d): bt", transport_type);

		switch (status) {
		case SAP_DEVICE_STATUS_DETACHED:
			dlog_print(DLOG_DEBUG, TAG, "DEVICE GOT DISCONNECTED");
			sap_peer_agent_destroy(priv_data.peer_agent);
			priv_data.peer_agent = NULL;
			break;

		case SAP_DEVICE_STATUS_ATTACHED:
			if (is_agent_added == TRUE) {
				_find_peer_agent();
			}
			dlog_print(DLOG_DEBUG, TAG, "DEVICE IS CONNECTED NOW, PLEASE CALL FIND PEER");
			break;

		default:
			dlog_print(DLOG_DEBUG, TAG, "unknown status (%d)", status);
			break;
		}

		break;

	default:
		dlog_print(DLOG_DEBUG, TAG, "unknown connectivity type (%d)", transport_type);
		break;
	}
}

gboolean agent_initialize()
{
	int result = 0;

	do {
		result = sap_agent_initialize(priv_data.agent, MEX_PROFILE_ID, SAP_AGENT_ROLE_PROVIDER,
					      on_agent_initialized, NULL);
		dlog_print(DLOG_DEBUG, TAG, "SAP >>> getRegisteredServiceAgent() >>> %d", result);
	} while (result != SAP_RESULT_SUCCESS);

	return TRUE;
}

void initialize_sap()
{
	sap_agent_h agent = NULL;

	sap_agent_create(&agent);

	if (agent == NULL)
		dlog_print(DLOG_DEBUG, TAG, "ERROR in creating agent");
	else
		dlog_print(DLOG_DEBUG, TAG, "successfully created sap agent");

	priv_data.agent = agent;

	sap_set_device_status_changed_cb(on_device_status_changed, NULL);

	agent_initialize();
	initialize_sensors();
}

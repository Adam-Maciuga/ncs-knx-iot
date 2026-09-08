/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/*
 * Device-agnostic stack lifecycle. Sets the device identity from the registered
 * profile and wires the handler callbacks the stack requires.
 */

#include <knx/knx_device.h>

#include "knx_priv.h"

#include <zephyr/logging/log.h>

#include "oc_api.h"
#include "oc_core_res.h"
#include "oc_endpoint.h"
#include "oc_knx.h"
#include "port/oc_storage.h"

LOG_MODULE_REGISTER(knx_stack, LOG_LEVEL_INF);

/*
 * Development SPAKE2+ credentials.
 *
 * With API Point Stack 1.1 the stack needs pregenerated credentials for SPAKE2+
 */
#define KNX_DEV_PASSWORD "2X4W3TE0DFLLS19Y1FCH"

static const oc_spake_record_t knx_dev_spake_record = {
	.w0 = {0x9a, 0x92, 0xbf, 0xe3, 0x5b, 0xb1, 0x80, 0x4f, 0x67, 0x9b, 0xbf,
	       0x19, 0xa9, 0x86, 0xa6, 0x26, 0xec, 0x6d, 0x4e, 0xf2, 0xd3, 0xa3,
	       0x2d, 0xd0, 0x9b, 0x64, 0xd2, 0xef, 0xa1, 0x61, 0x10, 0x00},
	.L = {0x04, 0x91, 0xeb, 0xa5, 0x0a, 0x66, 0x30, 0xa3, 0x08, 0x4a, 0x54, 0x8a, 0x8d,
	      0x06, 0x60, 0x47, 0xb9, 0x59, 0x55, 0x0a, 0x5c, 0x80, 0xb2, 0xe7, 0x93, 0x62,
	      0xd4, 0x46, 0x0f, 0xdc, 0x4f, 0x86, 0x70, 0xe7, 0x23, 0x5a, 0xdc, 0xe7, 0x92,
	      0xc8, 0x56, 0x3f, 0xad, 0x0d, 0xcd, 0x4a, 0x92, 0x76, 0xbd, 0x09, 0x6d, 0x01,
	      0xb0, 0xd7, 0x1c, 0x65, 0xfd, 0x74, 0x0b, 0x47, 0x71, 0x6b, 0x39, 0x15, 0x7b},
	.salt = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
		 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
		 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f},
	.it = 50000,
	.valid = true,
};

const char *app_get_password(void)
{
	return KNX_DEV_PASSWORD;
}

const oc_spake_record_t *app_get_precalculated_spake_data(void)
{
	return &knx_dev_spake_record;
}

void add_all_interface_short_urns_for_a_resource(const oc_resource_t *resource)
{
	oc_interface_mask_t res_interfaces = OC_IF_NONE;
	oc_resource_get_all_interfaces_for_a_resource(resource, &res_interfaces);

	const unsigned int nr_entries = oc_count_total_interfaces_in_mask(res_interfaces);
	oc_string_array_t interface_list;
	oc_new_string_array(&interface_list, nr_entries);

	oc_put_all_interface_short_urns_from_a_mask_in_string_array(res_interfaces, interface_list);

	oc_rep_set_string_array(root, if, interface_list);

	oc_free_string_array(&interface_list);
}

static void factory_presets_cb(void *data)
{
	(void)data;
}

static void restart_presets_cb(void *data)
{
	knx_restart_handler(data);
}

static void hostname_cb(const oc_string_t host_name, void *data)
{
	(void)data;

	LOG_DBG("host name: %s", oc_string(host_name));
}

static int knx_stack_init_device(void)
{
	const knx_identity_t *id = knx_device_get()->identity;

	oc_knx_version_info_t hw_version = {.major = 0, .minor = 0, .patch = 1};
	oc_knx_version_info_t fw_version = {.major = 0, .minor = 0, .patch = 1};
	oc_knx_version_info_t app_version = {.major = 1, .minor = 0, .patch = 0};

	/* Permanent identity. */
	oc_core_set_device_sn(id->serialnumber);
	oc_core_set_device_hwv(&hw_version);
	oc_core_set_and_store_device_fwv(&fw_version);

	oc_core_set_device_res();

	oc_core_set_device_mid(id->mid);
	oc_core_set_device_hwt(id->hw_type);
	oc_core_set_device_model(id->dev_model);

	oc_core_set_and_store_device_application_version(&app_version);

	char hname[HNAME_SIZE];
	(void)snprintf(hname, HNAME_SIZE, HNAME_TYPE, id->serialnumber);
	oc_core_set_device_hostname(hname);

	return 0;
}

int knx_stack_init(const char *storage_folder_name)
{
	(void)storage_folder_name;

	oc_storage_config(NULL);

	static oc_handler_t handler = {.init = knx_stack_init_device,
				       .signal_event_loop = signal_event_loop,
				       .register_resources = register_resources,
				       .requests_entry = NULL};

	oc_set_hostname_cb(hostname_cb, NULL);
	oc_set_factory_presets_cb(factory_presets_cb, NULL);
	oc_set_restart_cb(restart_presets_cb, NULL);

	return oc_main_init(&handler);
}

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 */

#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <media/cam_defs.h>
#include <media/cam_icp.h>
#include "a5_soc.h"
#include "cam_soc_util.h"
#include "cam_debug_util.h"

static int cam_a5_get_dt_properties(struct cam_hw_soc_info *soc_info)
{
	int rc = 0, i;
	const char *fw_name;
	struct a5_soc_info *a5_soc_info;
	struct device_node *of_node = NULL;
	struct platform_device *pdev = NULL;
	struct a5_ubwc_cfg_ext *ubwc_cfg_ext = NULL;
	int num_ubwc_cfg;

	pdev = soc_info->pdev;
	of_node = pdev->dev.of_node;

	rc = cam_soc_util_get_dt_properties(soc_info);
	if (rc < 0) {
		CAM_ERR(CAM_ICP, "get a5 dt prop is failed");
		return rc;
	}

	a5_soc_info = soc_info->soc_private;
	fw_name = a5_soc_info->fw_name;

	rc = of_property_read_string(of_node, "fw_name", &fw_name);
	if (rc < 0) {
		CAM_ERR(CAM_ICP, "fw_name read failed");
		goto end;
	}

	ubwc_cfg_ext = &a5_soc_info->uconfig.ubwc_cfg_ext;
	num_ubwc_cfg = of_property_count_u32_elems(of_node,
		"ubwc-ipe-fetch-cfg");
	if ((num_ubwc_cfg < 0) || (num_ubwc_cfg > ICP_UBWC_MAX)) {
		CAM_DBG(CAM_ICP, "wrong ubwc_ipe_fetch_cfg: %d", num_ubwc_cfg);
		rc = num_ubwc_cfg;
		goto ubwc_ex_cfg;
	}

	for (i = 0; i < num_ubwc_cfg; i++) {
		rc = of_property_read_u32_index(of_node, "ubwc-ipe-fetch-cfg",
			i, &ubwc_cfg_ext->ubwc_ipe_fetch_cfg[i]);
		if (rc < 0) {
			CAM_ERR(CAM_ICP,
				"unable to read ubwc_ipe_fetch_cfg values");
			goto end;
		}
	}

	num_ubwc_cfg = of_property_count_u32_elems(of_node,
		"ubwc-ipe-write-cfg");
	if ((num_ubwc_cfg < 0) || (num_ubwc_cfg > ICP_UBWC_MAX)) {
		CAM_ERR(CAM_ICP, "wrong ubwc_ipe_write_cfg: %d", num_ubwc_cfg);
		rc = num_ubwc_cfg;
		goto end;
	}

	for (i = 0; i < num_ubwc_cfg; i++) {
		rc = of_property_read_u32_index(of_node, "ubwc-ipe-write-cfg",
				i, &ubwc_cfg_ext->ubwc_ipe_write_cfg[i]);
		if (rc < 0) {
			CAM_ERR(CAM_ICP,
				"unable to read ubwc_ipe_write_cfg values");
			goto end;
		}
	}

	num_ubwc_cfg = of_property_count_u32_elems(of_node,
		"ubwc-bps-fetch-cfg");
	if ((num_ubwc_cfg < 0) || (num_ubwc_cfg > ICP_UBWC_MAX)) {
		CAM_ERR(CAM_ICP, "wrong ubwc_bps_fetch_cfg: %d", num_ubwc_cfg);
		rc = num_ubwc_cfg;
		goto end;
	}

	for (i = 0; i < num_ubwc_cfg; i++) {
		rc = of_property_read_u32_index(of_node, "ubwc-bps-fetch-cfg",
			i, &ubwc_cfg_ext->ubwc_bps_fetch_cfg[i]);
		if (rc < 0) {
			CAM_ERR(CAM_ICP,
				"unable to read ubwc_bps_fetch_cfg values");
			goto end;
		}
	}

	num_ubwc_cfg = of_property_count_u32_elems(of_node,
		"ubwc-bps-write-cfg");
	if ((num_ubwc_cfg < 0) || (num_ubwc_cfg > ICP_UBWC_MAX)) {
		CAM_ERR(CAM_ICP, "wrong ubwc_bps_write_cfg: %d", num_ubwc_cfg);
		rc = num_ubwc_cfg;
		goto end;
	}

	for (i = 0; i < num_ubwc_cfg; i++) {
		rc = of_property_read_u32_index(of_node, "ubwc-bps-write-cfg",
			i, &ubwc_cfg_ext->ubwc_bps_write_cfg[i]);
		if (rc < 0) {
			CAM_ERR(CAM_ICP,
				"unable to read ubwc_bps_write_cfg values");
			goto end;
		}
	}

	a5_soc_info->ubwc_config_ext = true;
	CAM_DBG(CAM_ICP, "read ubwc_cfg_ext for ipe/bps");
	return rc;

ubwc_ex_cfg:
	num_ubwc_cfg = of_property_count_u32_elems(of_node, "ubwc-cfg");
	if ((num_ubwc_cfg < 0) || (num_ubwc_cfg > ICP_UBWC_MAX)) {
		CAM_ERR(CAM_ICP, "wrong ubwc_cfg: %d", num_ubwc_cfg);
		rc = num_ubwc_cfg;
		goto end;
	}

	for (i = 0; i < num_ubwc_cfg; i++) {
		rc = of_property_read_u32_index(of_node, "ubwc-cfg",
			i, &a5_soc_info->uconfig.ubwc_cfg[i]);
		if (rc < 0) {
			CAM_ERR(CAM_ICP, "unable to read ubwc_cfg values");
			break;
		}
	}

end:
	return rc;
}

static int cam_a5_request_platform_resource(
	struct cam_hw_soc_info *soc_info,
	irq_handler_t a5_irq_handler, void *irq_data)
{
	int rc = 0;

	rc = cam_soc_util_request_platform_resource(soc_info, a5_irq_handler,
		irq_data);

	return rc;
}

int cam_a5_init_soc_resources(struct cam_hw_soc_info *soc_info,
	irq_handler_t a5_irq_handler, void *irq_data)
{
	int rc = 0;

	rc = cam_a5_get_dt_properties(soc_info);
	if (rc < 0)
		return rc;

	rc = cam_a5_request_platform_resource(soc_info, a5_irq_handler,
		irq_data);
	if (rc < 0)
		return rc;

	return rc;
}

int cam_a5_enable_soc_resources(struct cam_hw_soc_info *soc_info)
{
	int rc = 0;

	rc = cam_soc_util_enable_platform_resource(soc_info, true,
		CAM_SVS_VOTE, true);
	if (rc)
		CAM_ERR(CAM_ICP, "enable platform failed");

	return rc;
}

int cam_a5_disable_soc_resources(struct cam_hw_soc_info *soc_info)
{
	int rc = 0;

	rc = cam_soc_util_disable_platform_resource(soc_info, true, true);
	if (rc)
		CAM_ERR(CAM_ICP, "disable platform failed");

	return rc;
}

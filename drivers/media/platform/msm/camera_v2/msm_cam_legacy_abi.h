/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 32-bit camera ABI of the msm-3.18 uapi headers.
 *
 * The TB-X704F ships a 32-bit mm-qcamera-daemon built against Lenovo's
 * msm-3.18 headers. Only the structures whose 32-bit layout differs from
 * the 4.9 uapi are described here; the kernel accepts them in addition to
 * the 4.9 variants. The ioctl numbers differ in their size field, so both
 * variants can be told apart by the command alone, except for the CSIPHY
 * parameters which are passed by pointer (see msm_camera_legacy_abi).
 *
 * Differences (ARM32 sizes, 3.18 vs 4.9):
 *  - SUB_MODULE_MAX 14 vs 15 (SUB_MODULE_LASER_LED added):
 *    msm_sensor_info_t 164 vs 172, sensor_init_cfg_data 204 vs 212,
 *    sensorb_cfg_data 168 vs 176.
 *  - msm_sensor_id_info_t 6 vs 28 (reg setting added), vendor_id_info and
 *    vendor_id_need_read added: msm_camera_sensor_slave_info 1568 vs 1608.
 *  - msm_camera_csiphy_params 16 vs 24 (uint64_t data_rate added).
 */

#ifndef MSM_CAM_LEGACY_ABI_H
#define MSM_CAM_LEGACY_ABI_H

#include <linux/types.h>

/*
 * Set once a 3.18 style sensor init ioctl has been seen. Used where the
 * ABI variant can not be derived from the ioctl number.
 */
extern bool msm_camera_legacy_abi;

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#include <media/msm_cam_sensor.h>

#define LEGACY_SUB_MODULE_MAX 14

struct msm_sensor_info_t_legacy {
	char     sensor_name[MAX_SENSOR_NAME];
	uint32_t session_id;
	int32_t  subdev_id[LEGACY_SUB_MODULE_MAX];
	int32_t  subdev_intf[LEGACY_SUB_MODULE_MAX];
	uint8_t  is_mount_angle_valid;
	uint32_t sensor_mount_angle;
	int modes_supported;
	enum camb_position_t position;
};

struct sensor_init_cfg_data32_legacy {
	enum msm_sensor_init_cfg_type_t cfgtype;
	struct msm_sensor_info_t_legacy probed_info;
	char                            entity_name[MAX_SENSOR_NAME];
	union {
		compat_uptr_t setting;
	} cfg;
};

struct sensorb_cfg_data32_legacy {
	int cfgtype;
	union {
		struct msm_sensor_info_t_legacy   sensor_info;
		struct msm_sensor_init_params     sensor_init_params;
		compat_uptr_t                     setting;
		struct msm_sensor_i2c_sync_params sensor_i2c_sync_params;
	} cfg;
};

struct msm_sensor_id_info_t_legacy {
	unsigned short sensor_id_reg_addr;
	unsigned short sensor_id;
	unsigned short sensor_id_mask;
};

struct msm_camera_sensor_slave_info32_legacy {
	char sensor_name[32];
	char eeprom_name[32];
	char actuator_name[32];
	char ois_name[32];
	char flash_name[32];
	enum msm_sensor_camera_id_t camera_id;
	uint16_t slave_addr;
	enum i2c_freq_mode_t i2c_freq_mode;
	enum msm_camera_i2c_reg_addr_type addr_type;
	struct msm_sensor_id_info_t_legacy sensor_id_info;
	struct msm_sensor_power_setting_array32 power_setting_array;
	uint8_t  is_init_params_valid;
	struct msm_sensor_init_params sensor_init_params;
	enum msm_sensor_output_format_t output_format;
	uint8_t bypass_video_node_creation;
};

/* 3.18 msm_camera_csiphy_params ends before data_rate */
#define MSM_CSIPHY_PARAMS_LEGACY_SIZE \
	offsetof(struct msm_camera_csiphy_params, data_rate)

#define VIDIOC_MSM_SENSOR_CFG32_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 1, struct sensorb_cfg_data32_legacy)

#define VIDIOC_MSM_SENSOR_INIT_CFG32_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 10, \
		struct sensor_init_cfg_data32_legacy)

/* Sizes of the stock 32-bit daemon (ioctl numbers 0xc0cc56ca/0xc0a856c1) */
#define MSM_CAM_LEGACY_ABI_CHECK() do { \
	BUILD_BUG_ON(sizeof(struct msm_sensor_info_t_legacy) != 164); \
	BUILD_BUG_ON(sizeof(struct sensor_init_cfg_data32_legacy) != 204); \
	BUILD_BUG_ON(sizeof(struct sensorb_cfg_data32_legacy) != 168); \
	BUILD_BUG_ON(sizeof(struct msm_camera_sensor_slave_info32_legacy) \
		!= 1568); \
	BUILD_BUG_ON(MSM_CSIPHY_PARAMS_LEGACY_SIZE != 16); \
	BUILD_BUG_ON(SUB_MODULE_MAX < LEGACY_SUB_MODULE_MAX); \
} while (0)

static inline void msm_sensor_info_to_legacy(
	struct msm_sensor_info_t_legacy *dst,
	const struct msm_sensor_info_t *src)
{
	int i;

	memcpy(dst->sensor_name, src->sensor_name, sizeof(dst->sensor_name));
	dst->session_id = src->session_id;
	for (i = 0; i < LEGACY_SUB_MODULE_MAX; i++) {
		dst->subdev_id[i] = src->subdev_id[i];
		dst->subdev_intf[i] = src->subdev_intf[i];
	}
	dst->is_mount_angle_valid = src->is_mount_angle_valid;
	dst->sensor_mount_angle = src->sensor_mount_angle;
	dst->modes_supported = src->modes_supported;
	dst->position = src->position;
}

/*
 * ISP (msmb_isp.h): the 3.18 private command enum has CFG_HW_STATE,
 * AHB_CLK_CFG, UPDATE_FE_FRAME_ID and RESTART_FE at other positions and
 * msm_vfe_axi_stream_cfg_cmd carries a trailing enum hw_state (44 vs 48).
 * All other ISP ioctls are identical. Only used when <media/msmb_isp.h>
 * is included.
 */
#ifdef __MSMB_ISP__
struct msm_vfe_axi_stream_cfg_cmd_legacy {
	struct msm_vfe_axi_stream_cfg_cmd cfg;
	uint32_t hw_state;
};

#define VIDIOC_MSM_ISP_CFG_STREAM_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 5, \
		struct msm_vfe_axi_stream_cfg_cmd_legacy)
#define VIDIOC_MSM_ISP_FETCH_ENG_MULTI_PASS_START_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 25, \
		struct msm_vfe_fetch_eng_multi_pass_start)
#define VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 26, \
		struct msm_vfe_fetch_eng_multi_pass_start)
#define VIDIOC_MSM_ISP_CFG_HW_STATE_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 27, \
		struct msm_vfe_axi_stream_cfg_cmd_legacy)
#define VIDIOC_MSM_ISP_AHB_CLK_CFG_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 28, struct msm_isp_ahb_clk_cfg)
#define VIDIOC_MSM_ISP_UPDATE_FE_FRAME_ID_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 29, uint32_t)
#define VIDIOC_MSM_ISP_RESTART_FE_LEGACY \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 30, uint32_t)

/* Stock daemon: 0xc03056c5, 0xc03056db, 0xc00c56dc */
#define MSM_ISP_LEGACY_ABI_CHECK() do { \
	BUILD_BUG_ON(VIDIOC_MSM_ISP_CFG_STREAM_LEGACY != 0xc03056c5); \
	BUILD_BUG_ON(VIDIOC_MSM_ISP_CFG_HW_STATE_LEGACY != 0xc03056db); \
	BUILD_BUG_ON(VIDIOC_MSM_ISP_AHB_CLK_CFG_LEGACY != 0xc00c56dc); \
	BUILD_BUG_ON(_IOC_NR(VIDIOC_MSM_ISP_CFG_STREAM) != \
		BASE_VIDIOC_PRIVATE + 5); \
} while (0)
#endif /* __MSMB_ISP__ */
#endif /* CONFIG_COMPAT */

#endif /* MSM_CAM_LEGACY_ABI_H */

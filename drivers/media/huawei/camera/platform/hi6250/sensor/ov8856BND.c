 

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)

static hwsensor_vtbl_t s_ov8856BND_vtbl;
static bool power_on_status = false;//false: power off, true:power on
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -save -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -specific(-e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715)*/

enum rear_sensor_dvdd_type_t{
	DVDD105V=0,//1.05V
	DVDD120V=1,//1.20V
};

int ov8856BND_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting ov8856BND_power_up_setting[] = {
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 DVDD 1.05V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },


    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
struct sensor_power_setting ov8856BND_power_down_setting[] = {
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 DVDD 1.05V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val = LDO_VOLTAGE_1P05V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

struct sensor_power_setting ov8856BND_power_up_setting_dvdd120[] = {
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val =  LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },


    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
struct sensor_power_setting ov8856BND_power_down_setting_dvdd120[] = {
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD,
        .config_val = LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 DVDD 1.2V
    {
        .seq_type = SENSOR_DVDD2,
        .config_val =  LDO_VOLTAGE_1P2V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
	//MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //disable main camera reset
    {
        .seq_type = SENSOR_SUSPEND,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};
struct mutex ov8856BND_power_lock;
static sensor_t s_ov8856BND =
{
    .intf = { .vtbl = &s_ov8856BND_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(ov8856BND_power_up_setting),
            .power_setting = ov8856BND_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(ov8856BND_power_down_setting),
            .power_setting = ov8856BND_power_down_setting,
    },

};

static const struct of_device_id s_ov8856BND_dt_match[] =
{
    {
        .compatible = "huawei,ov8856BND",
        .data = &s_ov8856BND.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_ov8856BND_dt_match);

static struct platform_driver s_ov8856BND_driver =
{
    .driver =
    {
        .name = "huawei,ov8856BND",
        .owner = THIS_MODULE,
        .of_match_table = s_ov8856BND_dt_match,
    },
};

char const* ov8856BND_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = I2S(si);
    return sensor->board_info->name;
}

int ov8856BND_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    ret = hw_sensor_power_up_config(s_ov8856BND.dev, sensor->board_info);
    if (0 == ret ){
        cam_info("%s. power up config success.", __func__);
    }else{
        cam_err("%s. power up config fail.", __func__);
        return ret;
    }
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power up sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

int
ov8856BND_power_down(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;
    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power down sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power down sensor fail.", __func__);
    }
    hw_sensor_power_down_config(sensor->board_info);
    return ret;
}

int ov8856BND_csi_enable(hwsensor_intf_t* si)
{
    return 0;
}

int ov8856BND_csi_disable(hwsensor_intf_t* si)
{
    return 0;
}

static int ov8856BND_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;

    if (!si || !data){
        cam_err("%s : si or data is null", __func__);
        return -1;
    }

    sensor = I2S(si);
    cdata = (struct sensor_cfg_data *)data;

    cam_info("%s enter.", __func__);

    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static hwsensor_vtbl_t s_ov8856BND_vtbl =
{
    .get_name = ov8856BND_get_name,
    .config = ov8856BND_config,
    .power_up = ov8856BND_power_up,
    .power_down = ov8856BND_power_down,
    .match_id = ov8856BND_match_id,
    .csi_enable = ov8856BND_csi_enable,
    .csi_disable = ov8856BND_csi_disable,
    .match_id = ov8856BND_match_id,
};

int ov8856BND_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data;

    int ret =0;

	if (NULL == si || NULL == argp){
		cam_err("%s : si or argp is null", __func__);
		return -1;
	}

    data = (struct sensor_cfg_data *)argp;
    cam_debug("ov8856BND cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&ov8856BND_power_lock);
            if(false == power_on_status){
            ret = si->vtbl->power_up(si);
                if (ret == 0) {
                    power_on_status = true;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov8856BND_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&ov8856BND_power_lock);
            if(true == power_on_status){
            ret = si->vtbl->power_down(si);
                if (ret == 0) {
                    power_on_status = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&ov8856BND_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_WRITE_REG:
            break;
        case SEN_CONFIG_READ_REG:
            break;
        case SEN_CONFIG_WRITE_REG_SETTINGS:
            break;
        case SEN_CONFIG_READ_REG_SETTINGS:
            break;
        case SEN_CONFIG_ENABLE_CSI:
            //ret = si->vtbl->csi_enable(si);
            break;
        case SEN_CONFIG_DISABLE_CSI:
            //ret = si->vtbl->csi_disable(si);
            break;
        case SEN_CONFIG_MATCH_ID:
            ret = si->vtbl->match_id(si,argp);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static int32_t ov8856BND_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    cam_notice("enter %s",__func__);

    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_ov8856BND);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto ov8856BND_sensor_probe_fail;
        }

		/*CASE 1.2v dvdd, revew;CASE 1.05v use default; Other cases need add*/
		if(DVDD120V == s_ov8856BND.board_info->ldo_diverse){
			cam_info("%s change to relative power setting line %d\n", __func__, __LINE__);
			s_ov8856BND.power_setting_array.size = ARRAY_SIZE(ov8856BND_power_up_setting_dvdd120);
			s_ov8856BND.power_setting_array.power_setting = ov8856BND_power_up_setting_dvdd120;
			s_ov8856BND.power_down_setting_array.size = ARRAY_SIZE(ov8856BND_power_down_setting_dvdd120);
			s_ov8856BND.power_down_setting_array.power_setting = ov8856BND_power_down_setting_dvdd120;
		}
    } else {
        cam_err("%s ov8856BND of_node is NULL.\n", __func__);
        goto ov8856BND_sensor_probe_fail;
    }
	mutex_init(&ov8856BND_power_lock);
    s_ov8856BND.dev = &pdev->dev;
    rc = hwsensor_register(pdev, &s_ov8856BND.intf);
    rc = rpmsg_sensor_register(pdev, (void*)&s_ov8856BND);
ov8856BND_sensor_probe_fail:
    return rc;
}

static int __init ov8856BND_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_ov8856BND_driver,
            ov8856BND_platform_probe);
}

static void __exit ov8856BND_exit_module(void)
{
    hwsensor_unregister(&s_ov8856BND.intf);
    platform_driver_unregister(&s_ov8856BND_driver);
}
/*lint -restore*/
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 +esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -e528 -esym(528,*)*/
module_init(ov8856BND_init_module);
module_exit(ov8856BND_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("ov8856BND");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

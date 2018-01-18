 

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/pinctrl/consumer.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)

static hwsensor_vtbl_t s_hi259FIG_vtbl;
static bool power_on_status = false;//false: power off, true:power on
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -save -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -specific(-e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715)*/

int hi259FIG_config(hwsensor_intf_t* si, void  *argp);

struct sensor_power_setting hi259FIG_power_up_setting[] = {
    //MCAM1 AFVDD 3V
    {
        .seq_type = SENSOR_VCM_AVDD,
        .data = (void*)"cameravcm-vcc",
        .config_val = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM2 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM2 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 0,
    },

    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 2,
    },

    {
        .seq_type = SENSOR_PWDN2,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 5,
    },

    {
        .seq_type = SENSOR_RST3,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
};

struct sensor_power_setting hi259FIG_power_down_setting[] = {
    {
        .seq_type = SENSOR_RST3,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },

    {
        .seq_type = SENSOR_PWDN2,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    {
        .seq_type = SENSOR_MIPI_SW,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //SCAM AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD2,
        .data = (void*)"slave-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM2 AVDD 2.80V
    {
        .seq_type = SENSOR_AVDD,
        .data = (void*)"main-sensor-avdd",
        .config_val = LDO_VOLTAGE_V2P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = 1,
    },
    //MCAM2 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"main-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
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
};

atomic_t volatile hi259FIG_powered = ATOMIC_INIT(0);
struct mutex hi259FIG_power_lock;
static sensor_t s_hi259FIG =
{
    .intf = { .vtbl = &s_hi259FIG_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(hi259FIG_power_up_setting),
            .power_setting = hi259FIG_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(hi259FIG_power_down_setting),
            .power_setting = hi259FIG_power_down_setting,
     },
     .p_atpowercnt = &hi259FIG_powered,
};

static const struct of_device_id s_hi259FIG_dt_match[] =
{
    {
        .compatible = "huawei,hi259FIG",
        .data = &s_hi259FIG.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_hi259FIG_dt_match);

static struct platform_driver s_hi259FIG_driver =
{
    .driver =
    {
        .name = "huawei,hi259FIG",
        .owner = THIS_MODULE,
        .of_match_table = s_hi259FIG_dt_match,
    },
};

char const* hi259FIG_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    return sensor->board_info->name;
}

int hi259FIG_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

    sensor = I2S(si);
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    ret = hw_sensor_power_up_config(s_hi259FIG.dev, sensor->board_info);
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

int hi259FIG_power_down(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

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

int hi259FIG_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int hi259FIG_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int hi259FIG_match_id(hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor              = NULL;
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

static hwsensor_vtbl_t s_hi259FIG_vtbl =
{
    .get_name = hi259FIG_get_name,
    .config = hi259FIG_config,
    .power_up = hi259FIG_power_up,
    .power_down = hi259FIG_power_down,
    .match_id = hi259FIG_match_id,
    .csi_enable = hi259FIG_csi_enable,
    .csi_disable = hi259FIG_csi_disable,
};

int hi259FIG_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data;

    int ret =0;

    if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("hi259FIG cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            if (mutex_lock_interruptible(&hi259FIG_power_lock))
                return -ERESTARTSYS;
            if(false == power_on_status){
                ret = si->vtbl->power_up(si);
                if (0 == ret) {
                    power_on_status = true;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi259FIG_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            if (mutex_lock_interruptible(&hi259FIG_power_lock))
                return -ERESTARTSYS;
            if(true == power_on_status){
                ret = si->vtbl->power_down(si);
                if (0 == ret) {
                    power_on_status = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&hi259FIG_power_lock);
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
        case SEN_CONFIG_RESET_HOLD:
            cam_warn("%s cfgtype(%d) does not support", __func__, data->cfgtype);
            break;
        case SEN_CONFIG_RESET_RELEASE:
            cam_warn("%s cfgtype(%d) does not support", __func__, data->cfgtype);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static int32_t hi259FIG_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    cam_notice("enter %s",__func__);

    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }

    if (pdev->dev.of_node) {
        rc = hw_sensor_get_dt_data(pdev, &s_hi259FIG);
        if (rc < 0) {
            cam_err("%s failed line %d\n", __func__, __LINE__);
            goto hi259FIG_sensor_probe_fail;
        }
    } else {
        cam_err("%s hi259FIG of_node is NULL.\n", __func__);
        goto hi259FIG_sensor_probe_fail;
    }
    mutex_init(&hi259FIG_power_lock);
    s_hi259FIG.dev = &pdev->dev;
	
    rc = hwsensor_register(pdev, &s_hi259FIG.intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto hi259FIG_sensor_probe_fail;
    }

    rc = rpmsg_sensor_register(pdev, (void *)&s_hi259FIG);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(&s_hi259FIG.intf);
        goto hi259FIG_sensor_probe_fail;
    }

hi259FIG_sensor_probe_fail:
    return rc;
}

static int __init hi259FIG_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_hi259FIG_driver,
            hi259FIG_platform_probe);
}

static void __exit hi259FIG_exit_module(void)
{
    rpmsg_sensor_unregister((void*)&s_hi259FIG);
    hwsensor_unregister(&s_hi259FIG.intf);
    platform_driver_unregister(&s_hi259FIG_driver);
}
/*lint -restore*/
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 +esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -e528 -esym(528,*)*/
module_init(hi259FIG_init_module);
module_exit(hi259FIG_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("hi259FIG");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/

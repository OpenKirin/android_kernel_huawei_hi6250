/*
 *  stmvl53l0_module-i2c.c - Linux kernel modules for STM VL53L0 FlightSense TOF
 *							sensor
 *
 *  Copyright (C) 2016 STMicroelectronics Imaging Division.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/media.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_graph.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/videodev2.h>

#include <media/media-entity.h>
//#include <media/vl53l0.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-image-sizes.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-of.h>
#include <media/v4l2-subdev.h>
#include <media/huawei/camera.h>
#include <hwcam_intf.h> 
#include <linux/mutex.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/platform_device.h>
/*
 * power specific includes
 */
#include <linux/pwm.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
/*
 * API includes
 */
#include "vl53l0_api.h"
#include "vl53l0_def.h"
#include "vl53l0_platform.h"
#include "stmvl53l0-i2c.h"
#include "stmvl53l0.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include "../laser_common.h"
#include <media/huawei/laser_cfg.h>
//lint -save -e715 -e578 -e644 -e530 -e774 -e383 -e749 -e838
//lint -save -e826 -e715 -e785 -e64 -e528 -e551 -e753 -e753 -esym(528,*) -esym(753,*) -esym(551,*)
#define DRIVER_NAME "vl53l0_970"
enum{
    XSDN,
    INTR,
    GPIO_MAX,
};

enum{
    LASER_IOVDD,
    LASER_AVDD,
    VDD_MAX,
};

//#define DBG_EN_STUB

/*
 * Global data
 */

extern int laser_remove(struct i2c_client *client);

extern int laser_probe(struct i2c_client *client, const struct i2c_device_id *id);


static void put_intr(struct i2c_data *i2c_data)
{
    if(NULL == i2c_data)
    {
        vl53l0_dbgmsg("i2c_data is NULL");
        return;
    }
    if (i2c_data->io_flag.intr_owned) {
        if (i2c_data->io_flag.intr_started) {
            free_irq((unsigned int)i2c_data->irq, i2c_data);
            i2c_data->io_flag.intr_started = 0;
        }
        vl53l0_dbgmsg("release intr_gpio %d", i2c_data->intr_gpio);
        gpio_free((unsigned int)i2c_data->intr_gpio);
        i2c_data->io_flag.intr_owned = 0;
    }
    i2c_data->intr_gpio = -1;
}




static void put_xsdn(struct i2c_data *i2c_data)
{
    if(NULL == i2c_data)
    {
        vl53l0_dbgmsg("i2c_data is NULL");
        return;
    }
    if (i2c_data->io_flag.xsdn_owned) {
        vl53l0_dbgmsg("release xsdn_gpio %d", i2c_data->xsdn_gpio);
        gpio_free((unsigned int)i2c_data->xsdn_gpio);
        i2c_data->io_flag.xsdn_owned = 0;
        i2c_data->xsdn_gpio = -1;
    }
    i2c_data->xsdn_gpio = -1;
}

#ifdef DBG_EN_STUB
static int get_xsdn_stub(struct device *dev, struct i2c_data *i2c_data)
{
    vl53l0_dbgmsg("xsdn stub");
    return 0;
}
static int get_intr_stub(struct device *dev, struct i2c_data *i2c_data)
{
    vl53l0_dbgmsg("irq stub!!!");
    return 0;
}
#else
static int get_xsdn(struct device *dev, struct i2c_data *i2c_data)
{
    int rc = 0;

    if(NULL == i2c_data)
    {
        vl53l0_errmsg("i2c_data is NULL");
        return -EINVAL;
    }
    i2c_data->io_flag.xsdn_owned = 0;
    if (i2c_data->xsdn_gpio == -1) {
        vl53l0_errmsg("reset gpio is required");
        rc = -ENODEV;
        goto no_gpio;
    }

    vl53l0_dbgmsg("request xsdn_gpio %d", i2c_data->xsdn_gpio);
    rc = gpio_request((unsigned int)(i2c_data->xsdn_gpio), "vl53l0_xsdn");
    if (rc) {
        vl53l0_errmsg("fail to acquire xsdn %d", rc);
        goto request_failed;
    }

    rc = gpio_direction_output((unsigned int)(i2c_data->xsdn_gpio), 0);
    if (rc) {
        vl53l0_errmsg("fail to configure xsdn as output %d", rc);
        goto direction_failed;
    }
    i2c_data->io_flag.xsdn_owned = 1;

    return rc;

direction_failed:
    gpio_free((unsigned int)(i2c_data->xsdn_gpio));

request_failed:
no_gpio:
    return rc;
}

static int get_intr(struct device *dev, struct i2c_data *i2c_data)
{
    int rc = 0;
    if(NULL == i2c_data)
    {
        vl53l0_errmsg("i2c_data is NULL");
        return -EINVAL;
    }
    i2c_data->io_flag.intr_owned = 0;
    if (i2c_data->intr_gpio == -1) {
        vl53l0_errmsg("no interrupt gpio");
        goto no_gpio;
    }

    vl53l0_dbgmsg("request intr_gpio %d", i2c_data->intr_gpio);
    rc = gpio_request((unsigned int)(i2c_data->intr_gpio), "vl53l0_intr");
    if (rc) {
        vl53l0_errmsg("fail to acquire intr %d", rc);
        goto request_failed;
    }

    rc = gpio_direction_input((unsigned int)(i2c_data->intr_gpio));
    if (rc) {
        vl53l0_errmsg("fail to configure intr as input %d", rc);
        goto direction_failed;
    }

    i2c_data->irq = gpio_to_irq((unsigned int)(i2c_data->intr_gpio));
    if (i2c_data->irq < 0) {
        vl53l0_errmsg("fail to map GPIO: %d to interrupt:%d\n",
                i2c_data->intr_gpio, i2c_data->irq);
        goto irq_failed;
    }
    i2c_data->io_flag.intr_owned = 1;

    return rc;

irq_failed:
direction_failed:
    gpio_free((unsigned int)(i2c_data->intr_gpio));

request_failed:
no_gpio:
    return rc;
}

#endif

int stmvl53l0_power_on_off(struct stmvl53l0_data *data, int is_on)
{
    int rc = 0;
    struct i2c_data *i2cdata = NULL;
    if(NULL != data && NULL != data->client_object)
    {
        i2cdata = (struct i2c_data *) data->client_object;
        vl53l0_dbgmsg("Enter\n");

        gpio_direction_output((unsigned int)(i2cdata->xsdn_gpio), is_on);

        #if 0
        if(is_on)
        {
            rc = VL53L0_WaitDeviceBooted(data);
        }

        if (rc)
            vl53l0_errmsg("boot fail with error %d", rc);
        #endif
        vl53l0_dbgmsg("End\n");
    }
    else
    {
        vl53l0_errmsg("param is NULL");
        rc = -EINVAL;
    }

    return rc;
}

static int stmvl53l0_parse_tree(struct device *dev, struct i2c_data *i2c_data)
{
    struct device_node *of_node;
    int rc = 0;
    char *gpio_tag = NULL;
    const char *gpio_ctrl_types[GPIO_MAX] = {"xsdn","intr"};
    int gpio[GPIO_MAX];
    int index = 0;
    int i = 0;
    int gpio_num;
    if(dev == NULL) {
        return -EFAULT;
    }
    of_node = dev->of_node;
    vl53l0_dbgmsg("E");
    i2c_data->ldo_avdd.supply = "avdd";
    rc = devm_regulator_bulk_get(dev, 1, &(i2c_data->ldo_avdd));
    if (rc < 0) {
        vl53l0_dbgmsg("%s failed %d\n", __func__, __LINE__);
        goto fail;
    }

    gpio_num = of_gpio_count(of_node);
    if(gpio_num < 0 ) {
        vl53l0_dbgmsg("%s failed %d, ret is %d", __func__, __LINE__, gpio_num);
        goto fail;
    }
    vl53l0_dbgmsg("laser gpio num = %d", gpio_num);
    for(index = 0;index < gpio_num;index++)
    {
        rc = of_property_read_string_index(of_node, "huawei,gpio-ctrl-types", index, (const char **)&gpio_tag);
        if(rc < 0) {
        vl53l0_dbgmsg("%s failed %d", __func__, __LINE__);
        goto fail;
        }
        for(i = 0; i < GPIO_MAX; i++)
        {
            if(!strcmp(gpio_ctrl_types[i], gpio_tag))
            gpio[index] = of_get_gpio(of_node, index);
        }
        vl53l0_dbgmsg("gpio ctrl types: %s gpio = %d", gpio_tag, gpio[index]);
    }

    i2c_data->pinctrl = devm_pinctrl_get(dev);
    if (IS_ERR(i2c_data->pinctrl)) {
        vl53l0_dbgmsg("could not get pinctrl");
        goto fail;
    }
    i2c_data->pins_default = pinctrl_lookup_state(i2c_data->pinctrl, PINCTRL_STATE_DEFAULT);
    if (IS_ERR(i2c_data->pins_default))
        vl53l0_dbgmsg("could not get default pinstate");
    i2c_data->pins_idle = pinctrl_lookup_state(i2c_data->pinctrl, PINCTRL_STATE_IDLE);
    if (IS_ERR(i2c_data->pins_idle))
        vl53l0_dbgmsg("could not get idle pinstate");
   
    i2c_data->xsdn_gpio = gpio[XSDN];
    i2c_data->intr_gpio = gpio[INTR];
#ifndef DBG_EN_STUB
    rc = get_xsdn(dev, i2c_data);
#else
    rc = get_xsdn_stub(dev, i2c_data);
#endif
    if (rc)
        goto no_xsdn;
#ifndef DBG_EN_STUB
    rc = get_intr(dev, i2c_data);
#else
    rc = get_intr_stub(dev, i2c_data);
#endif
    if (rc)
        goto no_intr;
    return rc;

fail:
    vl53l0_dbgmsg("%s can not read laser info exit.", __func__);
    return rc;
no_intr:
    put_intr(i2c_data);
no_xsdn:
    put_xsdn(i2c_data);
return rc;
}


int stmvl53l0_match_id(struct stmvl53l0_data * data)
{
    VL53L0_Error Status;
    const unsigned timeout = 5;
    unsigned int count;
    const unsigned mdelay = 2;
    VL53L0_DeviceInfo_t DeviceInfo;
    int rc = 0;
    struct i2c_data *i2cdata = NULL;
    count = timeout;

    if(NULL == data ||NULL == data->client_object)
        return -EINVAL;
    vl53l0_dbgmsg("Call of VL53L0_DataInit Status");


    i2cdata = (struct i2c_data *) data->client_object;

    rc = regulator_set_voltage(i2cdata->ldo_avdd.consumer, LDO_AVDD_3P3V, LDO_AVDD_3P3V);
    if(rc < 0){
         vl53l0_errmsg("set ldo_avdd error %d", rc);
    }
    rc =  regulator_bulk_enable(1, &i2cdata->ldo_avdd);
    if (rc) {
        vl53l0_errmsg("failed to enable ldo_avdd %d\n", rc);
    }

    do {
        Status = VL53L0_DataInit(data);
        vl53l0_dbgmsg("Call of VL53L0_DataInit Status = %d", Status);
        msleep(mdelay);
    } while ((Status != VL53L0_ERROR_NONE) && (--count));
    if(Status == VL53L0_ERROR_NONE)
    {
        Status = VL53L0_GetDeviceInfo(data, &DeviceInfo);
        vl53l0_dbgmsg("Call of VL53L0_GetDeviceInfo Status = %d", Status);
        if(Status == VL53L0_ERROR_NONE)
        {
            vl53l0_dbgmsg("VL53L0_GetDeviceInfo:");
            vl53l0_dbgmsg("Device Name : %s", DeviceInfo.Name);
            vl53l0_dbgmsg("Device Type : %s", DeviceInfo.Type);
            vl53l0_dbgmsg("Device ID : %s", DeviceInfo.ProductId);
            vl53l0_dbgmsg("ProductRevisionMajor : %d", DeviceInfo.ProductRevisionMajor);
            vl53l0_dbgmsg("ProductRevisionMinor : %d", DeviceInfo.ProductRevisionMinor);
            #ifdef CONFIG_HUAWEI_HW_DEV_DCT
            /*if match id success*/
            set_hw_dev_flag(DEV_I2C_LASER);
            #endif
        }
    }
    return (int)Status;
}


static int stmvl53l0_detect(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	int rc = 0;
	struct stmvl53l0_data *vl53l0_data;
	struct i2c_data *i2c_object = NULL;
    hw_laser_ctrl_t *ctrl;
    if(client == NULL || id == NULL)
            return -EINVAL;

    ctrl = (hw_laser_ctrl_t*)id->driver_data;
	vl53l0_dbgmsg("Enter\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE)) {
		rc = -EIO;
		return rc;
	}

	vl53l0_data = kzalloc(sizeof(struct stmvl53l0_data), GFP_KERNEL);
	if (!vl53l0_data) {
		rc = -ENOMEM;
		return rc;
	}
	if (vl53l0_data != NULL) {
		vl53l0_data->client_object =
		    kzalloc(sizeof(struct i2c_data), GFP_KERNEL);
        if(!vl53l0_data->client_object) {
             rc = -ENOMEM;
            goto free_data;
        }
		i2c_object = (struct i2c_data *)vl53l0_data->client_object;
	}


	i2c_object->client = client;

	/* setup bus type */
	vl53l0_data->bus_type = I2C_BUS;

	/* setup regulator */
    rc = stmvl53l0_parse_tree(&i2c_object->client->dev, i2c_object);
    if (rc) {
        vl53l0_errmsg("vl53l0 parse device tree error");
        goto free_client;
	}
	/* setup device name */
	vl53l0_data->dev_name = dev_name(&client->dev);
        vl53l0_data->irq = i2c_object->irq;
	/* setup device data */
	/* setup other stuff */
	rc = stmvl53l0_setup(vl53l0_data);
    if(rc) {
        vl53l0_errmsg("vl53l0 setup fail");
        goto free_client;
    }

    stmvl53l0_power_on_off(vl53l0_data, 1);
    msleep(2);
//we do not really match id in this stage
    rc = stmvl53l0_match_id(vl53l0_data);
    if(rc) {
        vl53l0_errmsg("vl53l0 match id fail");
        goto free_client;
    }
    stmvl53l0_power_on_off(vl53l0_data, 0);
	/* init default value */
	i2c_object->power_up = 0;
    ctrl->data = (void *)vl53l0_data;

	vl53l0_dbgmsg("End\n");
	return rc;
free_client:
    kfree(i2c_object);
free_data:
    kfree(vl53l0_data);
    return rc;
}



static int stmvl53l0_remove(void *p)
{
	struct stmvl53l0_data *data = (struct stmvl53l0_data *)p;

	vl53l0_dbgmsg("Enter\n");

	/* Power down the device */
	stmvl53l0_cleanup(data);

	kfree(data->client_object);
	kfree(data);
	vl53l0_dbgmsg("End\n");
	return 0;
}

hw_laser_fn_t vl53l0_fn = {
    .laser_ioctl = stmvl53l0_laser_ioctl,
};

static hw_laser_ctrl_t vl53l0_ctrl = {
    .func_tbl = &vl53l0_fn,
    .data = NULL,
};


static const struct i2c_device_id vl53l0_id[] = {
	{ "vl53l0_970",  (unsigned long)&vl53l0_ctrl},
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(i2c, vl53l0_id);

static const struct of_device_id vl53l0_of_match[] = {
	{ .compatible = "huawei,vl53l0_970", },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, vl53l0_of_match);

static int vl53l0_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
    int rc;
    rc = stmvl53l0_detect(client, id);
    if(0 == rc)
    {
        rc = laser_probe(client, id);
    }
    return rc;
}

static int vl53l0_remove(struct i2c_client *client)
{
    int rc;
    rc = stmvl53l0_remove(client);
    if(0 == rc)
    {
        rc = laser_remove(client);
    }
    return rc;
}

static struct i2c_driver vl53l0_i2c_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.of_match_table = of_match_ptr(vl53l0_of_match),
	},
	.probe		= vl53l0_probe,
	.remove		= vl53l0_remove,
	.id_table	= vl53l0_id,
};

module_i2c_driver(vl53l0_i2c_driver);



MODULE_AUTHOR("h00382658");
MODULE_DESCRIPTION("hw laser driver for vl53l0 970");
MODULE_LICENSE("GPL v2");

//lint -restore

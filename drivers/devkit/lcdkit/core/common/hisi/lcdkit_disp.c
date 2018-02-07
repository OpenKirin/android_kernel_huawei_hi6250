#include "hisi_fb.h"
#include <huawei_ts_kit.h>
#include <linux/hisi/hw_cmdline_parse.h> //for runmode_is_factory
#include "lcdkit_effect.h"
#include "lcdkit_panel.h"
#include "lcdkit_disp.h"
#include "lcdkit_dbg.h"
#include <huawei_platform/log/log_jank.h>
#include "lcdkit_btb_check.h"

#if defined(CONFIG_LCDKIT_DRIVER)
#include "lcdkit_fb_util.h"
#endif

extern struct ts_kit_platform_data g_ts_kit_platform_data;
struct platform_device *g_hisi_pdev=NULL;
struct lcdkit_private_info g_lcdkit_pri_info;

void lcdkit_set_pdev(struct platform_device *pdev)
{
    g_hisi_pdev = pdev;
    return ;
}

void lcdkit_get_pdev(struct platform_device **pdev)
{
    *pdev = g_hisi_pdev;
    return ;
}

static void lcdkit_display_on_cmds_record_time(void)
{
    do_gettimeofday(&lcdkit_info.panel_infos.dis_on_cmds_record_tv);
    LCDKIT_INFO("display on at %lu seconds %lu mil seconds\n",
        lcdkit_info.panel_infos.dis_on_cmds_record_tv.tv_sec,
        lcdkit_info.panel_infos.dis_on_cmds_record_tv.tv_usec);
    lcdkit_info.panel_infos.dis_on_cmds_panel_on_tag = true;
    return;
}
static void lcdkit_display_on_cmds_check_delay(void)
{
    u32 delta_time_backlight_to_panel_on = 0;
    u32 delay_margin = 0;
    struct timeval tv;
    memset(&tv, 0, sizeof(struct timeval));
    do_gettimeofday(&tv);
    LCDKIT_INFO("set backlight at %lu seconds %lu mil seconds\n",
        tv.tv_sec, tv.tv_usec);
    delta_time_backlight_to_panel_on = (tv.tv_sec - lcdkit_info.panel_infos.dis_on_cmds_record_tv.tv_sec)*1000000
        + tv.tv_usec - lcdkit_info.panel_infos.dis_on_cmds_record_tv.tv_usec;
    delta_time_backlight_to_panel_on /= 1000;
    if (delta_time_backlight_to_panel_on >= lcdkit_info.panel_infos.dis_on_cmds_delay_margin_time)
    {
        LCDKIT_INFO("%lu > %lu, no need delay\n",
            delta_time_backlight_to_panel_on,
            lcdkit_info.panel_infos.dis_on_cmds_delay_margin_time);
        goto CHECK_DELAY_END;
    }
    delay_margin = lcdkit_info.panel_infos.dis_on_cmds_delay_margin_time
                                 - delta_time_backlight_to_panel_on;
    if (delay_margin > 200)
    {
        LCDKIT_INFO("something maybe error");
        goto CHECK_DELAY_END;
    }
    mdelay(delay_margin);
CHECK_DELAY_END:
    lcdkit_info.panel_infos.dis_on_cmds_panel_on_tag = false;
    return;
}
static void lcdkit_display_on_new_seq_set_delay_time(void)
{
    static bool read_once = false;

    if (read_once){
        LCDKIT_INFO("display on new seq already set\n");
        return ;
    }

    if (synaptics_tddi_new_seq()){
        LCDKIT_INFO("panel display on new seq for synaptics DDIC\n");

        /* set reset H delay 20 ms */
        lcdkit_gpio_reset_normal_cmds[2].wait = 20;
        lcdkit_info.panel_infos.display_on_cmds.cmds[lcdkit_info.panel_infos.display_on_cmds.cmd_cnt-1].wait = 80;
    }
    read_once = true;
    LCDKIT_INFO("set diaplay on new seq in first panel on\n");
    return ;
}

/*
*name:lcdkit_get_id
*function:power on panel
*@pdev:platform device
*/
int lcdkit_get_id(struct platform_device* pdev)
{
    int pulldown_value = 0;
    int pullup_value = 0;
    int lcd_status = 0;
    int lcd_id0 = 0;
    int lcd_id1 = 0;

    /*set gpio direction to out, set id0 to low*/
    gpio_cmds_tx(lcdkit_gpio_id0_low_cmds, \
                 ARRAY_SIZE(lcdkit_gpio_id0_low_cmds));
    /*set gpio direction to input*/
    gpio_cmds_tx(lcdkit_gpio_id0_input_cmds, \
                 ARRAY_SIZE(lcdkit_gpio_id0_input_cmds));
    /*read id0 value*/
    pulldown_value = gpio_get_value(lcdkit_info.panel_infos.gpio_lcd_id0);

    /*set gpio direction to out, set id0 to high*/
    gpio_cmds_tx(lcdkit_gpio_id0_high_cmds, \
                 ARRAY_SIZE(lcdkit_gpio_id0_high_cmds));
    /*set gpio direction to input*/
    gpio_cmds_tx(lcdkit_gpio_id0_input_cmds, \
                 ARRAY_SIZE(lcdkit_gpio_id0_input_cmds));
    /*read id0 value*/
    pullup_value = gpio_get_value(lcdkit_info.panel_infos.gpio_lcd_id0);

    if (pulldown_value != pullup_value)
    {
        lcd_id0 = 2; //floating
    }
    else
    {
        lcd_id0 = pulldown_value; //high or low
    }

    /*set gpio direction to out, set id1 to low*/
    gpio_cmds_tx(lcdkit_gpio_id1_low_cmds, \
                 ARRAY_SIZE(lcdkit_gpio_id1_low_cmds));
    /*set gpio direction to input*/
    gpio_cmds_tx(lcdkit_gpio_id1_input_cmds, \
                 ARRAY_SIZE(lcdkit_gpio_id1_input_cmds));
    /*read id1 value*/
    pulldown_value = gpio_get_value(lcdkit_info.panel_infos.gpio_lcd_id1);

    /*set gpio direction to out, set id1 to low*/
    gpio_cmds_tx(lcdkit_gpio_id1_high_cmds, \
                 ARRAY_SIZE(lcdkit_gpio_id1_high_cmds));
    /*set gpio direction to input*/
    gpio_cmds_tx(lcdkit_gpio_id1_input_cmds, \
                 ARRAY_SIZE(lcdkit_gpio_id1_input_cmds));
    /*read id1 value*/
    pullup_value = gpio_get_value(lcdkit_info.panel_infos.gpio_lcd_id1);

    if (pulldown_value != pullup_value)
    {
        lcd_id1 = 2; //floating
    }
    else
    {
        lcd_id1 = pulldown_value; //high or low
    }

    lcd_status = (lcd_id0 | (lcd_id1 << 2));
    LCDKIT_INFO("lcd_id0:%d, lcd_id1:%d, lcd_status = 0x%x.\n", lcd_id0, lcd_id1, lcd_status);
    return lcd_status;
}

/*
*name:lcdkit_vcc_init
*function:init lcd vcc parameter
*@cmds:vcc cmds
*@cnt:vcc number
*/
static void lcdkit_power_init(struct vcc_desc* cmds , int cnt)
{
    int i = 0;
    struct vcc_desc* cm = NULL;
    cm = cmds;

    for (i = 0; i < cnt; i++)
    {
        if (cm->dtype == DTYPE_VCC_SET_VOLTAGE)
        {
            if (0 == strncmp(cm->id, LCDKIT_VCC_LCDANALOG_NAME, strlen(cm->id)))
            {
                cm->min_uV = lcdkit_info.panel_infos.lcdanalog_vcc;
                cm->max_uV = lcdkit_info.panel_infos.lcdanalog_vcc;
            }
            else if (0 == strncmp(cm->id, LCDKIT_VCC_LCDIO_NAME, strlen(cm->id)))
            {
                cm->min_uV = lcdkit_info.panel_infos.lcdio_vcc;
                cm->max_uV = lcdkit_info.panel_infos.lcdio_vcc;
            }
            else if (0 == strncmp(cm->id, VCC_LCDKIT_BIAS_NAME, strlen(cm->id)))
            {
                cm->min_uV = lcdkit_info.panel_infos.lcd_bias;
                cm->max_uV = lcdkit_info.panel_infos.lcd_bias;
            }
            else if (0 == strncmp(cm->id, VCC_LCDKIT_VSP_NAME, strlen(cm->id)))
            {
                cm->min_uV = lcdkit_info.panel_infos.lcd_vsp;
                cm->max_uV = lcdkit_info.panel_infos.lcd_vsp;
            }
            else if (0 == strncmp(cm->id, VCC_LCDKIT_VSN_NAME, strlen(cm->id)))
            {
                cm->min_uV = lcdkit_info.panel_infos.lcd_vsn;
                cm->max_uV = lcdkit_info.panel_infos.lcd_vsn;
            }
        }

        cm++;
    }
}

static __maybe_unused int lcdkit_enter_ulps(struct hisi_fb_data_type* hisifd)
{
    BUG_ON(hisifd == NULL);

    /* switch to cmd mode */
    set_reg(hisifd->mipi_dsi0_base + MIPIDSI_MODE_CFG_OFFSET, 0x1, 1, 0);
    /* cmd mode: low power mode */
    set_reg(hisifd->mipi_dsi0_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x7f, 7, 8);
    set_reg(hisifd->mipi_dsi0_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0xf, 4, 16);
    set_reg(hisifd->mipi_dsi0_base + MIPIDSI_CMD_MODE_CFG_OFFSET, 0x1, 1, 24);

    /* disable generate High Speed clock */
    set_reg(hisifd->mipi_dsi0_base + MIPIDSI_LPCLK_CTRL_OFFSET, 0x0, 1, 0);
    mipi_dsi_ulps_cfg(hisifd, 0);
    udelay(10);
    return 0;
}

static int lcdkit_is_enter_sleep_mode(void)
{
   return (ts_kit_gesture_func || g_tskit_pt_station_flag);
}

static void lcdkit_bias_init(struct platform_device* pdev)
{
    ssize_t ret = 0;
    if (lcdkit_is_lcd_panel())
    {
        if (lcdkit_bias_is_regulator_ctrl_power())
        {
            /* lcd scharger vcc get*/
            ret = vcc_cmds_tx(pdev, lcdkit_scharger_bias_get_cmds, ARRAY_SIZE(lcdkit_scharger_bias_get_cmds));
            if (ret != 0)
            {
                LCDKIT_ERR("LCD scharger vcc get failed!\n");
            }
            /*init bias/vsp/vsn*/
            lcdkit_power_init(lcdkit_scharger_bias_set_cmds, ARRAY_SIZE(lcdkit_scharger_bias_set_cmds));
        }
    }
}

static ssize_t lcdkit_vcc_init(struct platform_device* pdev)
{
    ssize_t ret = 0;

    if (lcdkit_iovcc_is_regulator_ctrl_power())
    {
        lcdkit_power_init(lcdkit_io_vcc_init_cmds, ARRAY_SIZE(lcdkit_io_vcc_init_cmds));
        /*init lcdio vcc*/
        ret = vcc_cmds_tx(pdev, lcdkit_io_vcc_init_cmds,  ARRAY_SIZE(lcdkit_io_vcc_init_cmds));

        if (ret != 0)
        {
            LCDKIT_ERR("LCD vcc init failed!\n");
            return ret;
        }
    }

    /*init lcdanalog vcc*/
     if ( HYBRID == g_tskit_ic_type )
     {
        lcdkit_power_init(lcdkit_vci_init_cmds,ARRAY_SIZE(lcdkit_vci_init_cmds));
        if(lcdkit_vci_is_regulator_ctrl_power())
        {
            ret = vcc_cmds_tx(pdev, lcdkit_vci_init_cmds, ARRAY_SIZE(lcdkit_vci_init_cmds));

            if (ret != 0)
            {
                LCDKIT_ERR("LCD vcc init failed!\n");
                return ret;
            }
        }
    }

    return ret;
}

static void lcdkit_reset_delay_init(struct gpio_desc* cmds)
{
    struct gpio_desc* cm = NULL;

    cm = cmds;
    cm->wait = lcdkit_info.panel_infos.reset_step1_H;
    cm++;
    cm->wait = lcdkit_info.panel_infos.reset_L;
    cm++;
    cm->wait = lcdkit_info.panel_infos.reset_step2_H;
}

static void lcdkit_bias_on_delay_init_regulator(struct vcc_desc* cmds)
{
    struct vcc_desc* cm = NULL;

    cm = cmds;
    cm->wait = lcdkit_info.panel_infos.delay_af_bias_on;
    cm++;
    cm->wait = lcdkit_info.panel_infos.delay_af_vsp_on;
    cm++;
    cm->wait = lcdkit_info.panel_infos.delay_af_vsn_on;
}

static void lcdkit_bias_off_delay_init_regulator(struct vcc_desc* cmds)
{
    struct vcc_desc* cm = NULL;

    cm = cmds;
    cm->wait = lcdkit_info.panel_infos.delay_af_vsn_off;
    cm++;
    cm->wait = lcdkit_info.panel_infos.delay_af_vsp_off;
    cm++;
    cm->wait = lcdkit_info.panel_infos.delay_af_bias_off;
}

static void lcdkit_bias_on_delay_init_gpio(struct gpio_desc* cmds)
{
    struct gpio_desc* cm = NULL;

    cm = cmds;
    cm->wait = lcdkit_info.panel_infos.delay_af_vsp_on;
    cm++;
    cm->wait = lcdkit_info.panel_infos.delay_af_vsn_on;
}

static void lcdkit_bias_off_delay_init_gpio(struct gpio_desc* cmds)
{
    struct gpio_desc* cm = NULL;

    cm = cmds;
    cm->wait = lcdkit_info.panel_infos.delay_af_vsn_off;
    cm++;
    cm->wait = lcdkit_info.panel_infos.delay_af_vsp_off;
}
static void lcdkit_gpio_request(void)
{
    /*id0 && id1 gpio request*/
    gpio_cmds_tx(lcdkit_gpio_id_request_cmds, ARRAY_SIZE(lcdkit_gpio_id_request_cmds));

    /*bias request*/
    if (lcdkit_bias_is_gpio_ctrl_power())
    {
        gpio_cmds_tx(lcdkit_bias_request_cmds, ARRAY_SIZE(lcdkit_bias_request_cmds));
    }
}

int lcdkit_get_vsp_voltage(void)
{
    int i = 0;

    for(i = 0;i < sizeof(voltage_table) / sizeof(struct lcdkit_vsp_vsn_voltage);i ++) {
        if(voltage_table[i].voltage == lcdkit_info.panel_infos.lcd_vsp) {
            LCDKIT_INFO("vsp voltage:%ld\n",voltage_table[i].voltage);
            return (voltage_table[i].value);
        }
    }

    if (i >= sizeof(voltage_table) / sizeof(struct lcdkit_vsp_vsn_voltage)) {
        LCDKIT_ERR("not found vsp voltage, use default voltage:TPS65132_VOL_55\n");
    }
    return TPS65132_VOL_55;
}

int lcdkit_get_vsn_voltage(void)
{
    int i = 0;

    for(i = 0;i < sizeof(voltage_table) / sizeof(struct lcdkit_vsp_vsn_voltage);i ++) {
        if(voltage_table[i].voltage == lcdkit_info.panel_infos.lcd_vsn) {
            LCDKIT_INFO("vsn voltage:%ld\n",voltage_table[i].voltage);
            return (voltage_table[i].value);
        }
    }

    if (i >= sizeof(voltage_table) / sizeof(struct lcdkit_vsp_vsn_voltage)) {
        LCDKIT_ERR("not found vsn voltage, use default voltage:TPS65132_VOL_55\n");
    }
    return TPS65132_VOL_55;
}

static ssize_t lcdkit_pinctrl_init(struct platform_device* pdev)
{
    ssize_t ret = 0;
    // lcd pinctrl init
    ret = pinctrl_cmds_tx(pdev, lcdkit_pinctrl_init_cmds, ARRAY_SIZE(lcdkit_pinctrl_init_cmds));

    if (ret != 0)
    {
        LCDKIT_ERR("Init pinctrl failed, defer\n");
    }

    return ret;
}

/*
*name:lcdkit_on
*function:power on panel
*@pdev:platform device
*/
static int lcdkit_on(struct platform_device* pdev)
{
    struct hisi_fb_data_type* hisifd = NULL;
    struct hisi_panel_info* pinfo = NULL;
    char __iomem* mipi_dsi0_base = NULL;
    ssize_t ret = 0, i = 0;
    ssize_t esd_recovery_times = 0;
#if defined (CONFIG_HUAWEI_DSM)
    static struct lcd_reg_read_t lcd_status_reg[] = {
        {0x0A, 0x9C, 0xFF, "lcd power state"},
    };
#endif
    LCDKIT_INFO("enter!\n");

    if (NULL == pdev)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    hisifd = platform_get_drvdata(pdev);

    if (NULL == hisifd)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    pinfo = &(hisifd->panel_info);
    mipi_dsi0_base = hisifd->mipi_dsi0_base;

    /*for mipi clk debug*/
    if (is_lcdkit_mipiclk_enable())
    {
        hisifd->panel_info.mipi.dsi_bit_clk = get_lcdkit_mipiclk_dbg();
        hisifd->panel_info.mipi.dsi_bit_clk_upt = get_lcdkit_mipiclk_dbg();
    }

    if (lcdkit_is_lcd_panel())
    {
        /* for vsp vsn debug */
        if (is_lcdkit_vsp_vsn_enable() && lcdkit_bias_is_regulator_ctrl_power())
        {
            lcdkit_debug_set_vsp_vsn(lcdkit_scharger_bias_set_cmds, ARRAY_SIZE(lcdkit_scharger_bias_set_cmds));
            vcc_cmds_tx(NULL, lcdkit_scharger_bias_set_cmds, ARRAY_SIZE(lcdkit_scharger_bias_set_cmds));
        }
    }

    if (pinfo->lcd_init_step == LCD_INIT_POWER_ON)
    {
      if(CHECKED_ERROR == mipi_lcdkit_btb_check()) {
        LCDKIT_ERR("btb checked failed!");
      }
      if (lcdkit_info.panel_infos.panel_display_on_new_seq)
        lcdkit_display_on_new_seq_set_delay_time();
      LOG_JANK_D(JLID_KERNEL_LCD_POWER_ON, "%s", "LCD_POWER_ON");
      if(!lcdkit_is_enter_sleep_mode())
      {
        /*for incell panel, lcd ctrl tp VCI power*/
        if ( HYBRID == g_tskit_ic_type )
         {
               if(lcdkit_vci_is_regulator_ctrl_power())
               	{
                vcc_cmds_tx(pdev, lcdkit_regulator_vci_enable_cmds,
                            ARRAY_SIZE(lcdkit_regulator_vci_enable_cmds));
            }
            else if ( lcdkit_vci_is_gpio_ctrl_power())
            {
                gpio_cmds_tx(lcdkit_vci_request_cmds, ARRAY_SIZE(lcdkit_vci_request_cmds));
                gpio_cmds_tx(lcdkit_vci_enable_cmds, ARRAY_SIZE(lcdkit_vci_enable_cmds));

            }
        }

        mdelay(lcdkit_info.panel_infos.delay_af_vci_on);

        /*1.8V*/
        if ( lcdkit_iovcc_is_regulator_ctrl_power())
        {
            vcc_cmds_tx(pdev, lcdkit_io_vcc_enable_cmds,
                        ARRAY_SIZE(lcdkit_io_vcc_enable_cmds));
        }
        else if (lcdkit_iovcc_is_gpio_ctrl_power())
        {
            gpio_cmds_tx(lcdkit_iovcc_request_cmds, ARRAY_SIZE(lcdkit_iovcc_request_cmds));
            gpio_cmds_tx(lcdkit_iovcc_enable_cmds, ARRAY_SIZE(lcdkit_iovcc_enable_cmds));
        }

        mdelay(lcdkit_info.panel_infos.delay_af_iovcc_on);

        if (lcdkit_vbat_is_gpio_ctrl_power())
        {
            gpio_cmds_tx(lcdkit_vbat_request_cmds, ARRAY_SIZE(lcdkit_vbat_request_cmds));
            gpio_cmds_tx(lcdkit_vbat_enable_cmds, ARRAY_SIZE(lcdkit_vbat_enable_cmds));
        }
        mdelay(lcdkit_info.panel_infos.delay_af_vbat_on);

        // lcd pinctrl normal
        pinctrl_cmds_tx(pdev, lcdkit_pinctrl_normal_cmds,
                        ARRAY_SIZE(lcdkit_pinctrl_normal_cmds));

        // lcd reset gpio request
        gpio_cmds_tx(lcdkit_gpio_reset_request_cmds, \
                     ARRAY_SIZE(lcdkit_gpio_reset_request_cmds));

        if (lcdkit_is_oled_panel())
        {
            //vdd3 vddr enable
            gpio_cmds_tx(lcdkit_vdd_normal_cmds, \
                         ARRAY_SIZE(lcdkit_vdd_normal_cmds));
        }

        if (lcdkit_info.panel_infos.first_reset)
        {
            //the first reset HLH
            gpio_cmds_tx(lcdkit_gpio_reset_normal_cmds, \
                         ARRAY_SIZE(lcdkit_gpio_reset_normal_cmds));
        }

        if (lcdkit_bias_is_gpio_ctrl_power())
        {
            LCDKIT_INFO("power ctrl by gpio!\n");
            gpio_cmds_tx(lcdkit_bias_request_cmds, \
                         ARRAY_SIZE(lcdkit_bias_request_cmds));
            gpio_cmds_tx(lcdkit_bias_enable_cmds, \
                         ARRAY_SIZE(lcdkit_bias_enable_cmds));
        }

        if (lcdkit_is_lcd_panel() && lcdkit_bias_is_regulator_ctrl_power())
        {
            //for lcd, vsp/vsn enable
            LCDKIT_INFO("power ctrl by regulator and the panel is lcd!\n");
            vcc_cmds_tx(NULL, lcdkit_scharger_bias_enable_cmds, \
                        ARRAY_SIZE(lcdkit_scharger_bias_enable_cmds));
        }
         }else
        {
             // lcd pinctrl normal
             pinctrl_cmds_tx(pdev, lcdkit_pinctrl_normal_cmds,
                        ARRAY_SIZE(lcdkit_pinctrl_normal_cmds));

            // lcd reset gpio request
            gpio_cmds_tx(lcdkit_gpio_reset_request_cmds, \
                     ARRAY_SIZE(lcdkit_gpio_reset_request_cmds));
        }
        pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
    }
    else if (pinfo->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE)
    {
        mdelay(lcdkit_info.panel_infos.delay_af_LP11);

lp_sequence_restart:
        // lcd gpio normal
        if (lcdkit_info.panel_infos.second_reset)
        {
            //the second reset hlh
            gpio_cmds_tx(lcdkit_gpio_reset_normal_cmds, \
                         ARRAY_SIZE(lcdkit_gpio_reset_normal_cmds));
        }

        if ( g_tskit_ic_type && !lcdkit_info.panel_infos.ts_resume_ctrl_mode)
        {
            LCDKIT_INFO("call ts resume default\n");
            if(lcdkit_info.panel_infos.tp_resume_no_sync)
            {
                ret = ts_kit_power_control_notify(TS_RESUME_DEVICE, NO_SYNC_TIMEOUT);
            }
            else
            {
                ret = ts_kit_power_control_notify(TS_RESUME_DEVICE, SHORT_SYNC_TIMEOUT);
            }

            if (ret)
            {
                LCDKIT_ERR("ts resume device err\n");
            }

            mdelay(lcdkit_info.panel_infos.delay_af_tp_reset);
        }

        if (runmode_is_factory())
        {
            if (!strncmp(lcdkit_info.panel_infos.panel_name, "JDI_NT35696 5.1' CMD TFT 1920 x 1080", strlen(lcdkit_info.panel_infos.panel_name)))
            {
                ret = lcdkit_jdi_nt35696_5p5_gamma_reg_read(hisifd);
                if (ret)
                {
                    LCDKIT_ERR("read gamma reg err\n");
                }
            }
        }

        //for debug for inital_code
        if (is_lcdkit_initcode_enable())
        {
            LCDKIT_INFO("init code debug enter!\n");
            lcdkit_dsi_tx(hisifd, &lcdkit_info.panel_infos.g_panel_cmds);
        }
        else
        {
            lcdkit_info.lcdkit_on_cmd(hisifd, &lcdkit_info.panel_infos.display_on_cmds);
        }

        if ( g_tskit_ic_type && (lcdkit_info.panel_infos.ts_resume_ctrl_mode & LCDKIT_TS_RESUME_AFTER_DIS_ON))
        {
            LCDKIT_INFO("call ts resume after display on\n");
            if(lcdkit_info.panel_infos.tp_resume_no_sync)
            {
                ret = ts_kit_power_control_notify(TS_RESUME_DEVICE, NO_SYNC_TIMEOUT);
            }
            else
            {
                ret = ts_kit_power_control_notify(TS_RESUME_DEVICE, SHORT_SYNC_TIMEOUT);
            }

            if (ret)
            {
                LCDKIT_ERR("ts resume device err\n");
            }

            mdelay(lcdkit_info.panel_infos.delay_af_tp_reset);
        }

       if (lcdkit_info.panel_infos.dis_on_cmds_delay_margin_support)
        {
            lcdkit_display_on_cmds_record_time();
        }
        if(lcdkit_info.panel_infos.display_effect_on_support)
        {
            LCDKIT_INFO("display on effect is support!\n");
            lcdkit_dsi_tx(hisifd,&lcdkit_info.panel_infos.display_effect_on_cmds);
        }

        if (lcdkit_info.panel_infos.display_on_in_backlight)
        {
            lcdkit_info.panel_infos.display_on_need_send = true;
        }

        lcdkit_info.panel_infos.cabc_mode = CABC_UI_MODE;
        lcdkit_info.panel_infos.ce_mode = CE_SRGB_MODE;
        if(lcdkit_info.panel_infos.check_reg_on_support)
        {
            if (ESD_RECOVER_STATE_START != hisifd->esd_recover_state){
#if defined (CONFIG_HUAWEI_DSM)
                for(i = 0; i < ARRAY_SIZE(lcd_status_reg); i++)
                {
                   lcd_status_reg[i].reg_addr = lcdkit_info.panel_infos.check_reg_on_value.buf[i];
                   lcd_status_reg[i].expected_value = lcdkit_info.panel_infos.check_reg_expect_value.buf[i];
                   lcd_status_reg[i].read_mask = lcdkit_info.panel_infos.check_reg_mask_value.buf[i];
                   LCDKIT_INFO("check_reg_on_value:%02x, check_reg_expect_value:%02x, check_reg_mask_value:%02x\n",
                        lcdkit_info.panel_infos.check_reg_on_value.buf[i],
                        lcdkit_info.panel_infos.check_reg_expect_value.buf[i],
                        lcdkit_info.panel_infos.check_reg_mask_value.buf[i]);
                 }
                panel_check_status_and_report_by_dsm(lcd_status_reg, \
                ARRAY_SIZE(lcd_status_reg), mipi_dsi0_base);
                LCDKIT_INFO("recovery:%u, esd_recovery_times:%u\n", lcd_status_reg[0].recovery, esd_recovery_times);
                if (lcd_status_reg[0].recovery && esd_recovery_times < 3){
                    LCDKIT_ERR("==>0A error detect, go to restart lp sequence!!!\n");
                    esd_recovery_times++;
                    goto lp_sequence_restart;
                }
#endif
            }
        }

        pinfo->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
        LCDKIT_INFO("lcd name = %s.\n", lcdkit_info.panel_infos.panel_name);
    }
    else if (pinfo->lcd_init_step == LCD_INIT_MIPI_HS_SEND_SEQUENCE)
    {
       if(lcdkit_info.panel_infos.lp2hs_mipi_check_support)
       {
           lcdkit_lp2hs_mipi_test(hisifd);
    }

        if ( g_tskit_ic_type)
        {
            ret = ts_kit_power_control_notify(TS_AFTER_RESUME, NO_SYNC_TIMEOUT);

            if (ret)
            { LCDKIT_ERR("ts after resume err\n"); }
        }
    }
    else
    {
        LCDKIT_ERR("failed to init lcd!\n");
    }

    // backlight on
    hisi_lcd_backlight_on(pdev);

    if (lcdkit_is_oled_panel())
    {
        //	amoled_irq_enable();
    }

    LCDKIT_INFO("fb%d, -!\n", hisifd->index);

    return 0;
}

/*
*name:lcdkit_off
*function:power off panel
*@pdev:platform device
*/
static int lcdkit_off(struct platform_device* pdev)
{
    struct hisi_fb_data_type* hisifd = NULL;
    struct hisi_panel_info* pinfo = NULL;
    ssize_t ret = 0;

    if (NULL == pdev)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    hisifd = platform_get_drvdata(pdev);

    if (NULL == hisifd)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    if (NULL == hisifd->mipi_dsi0_base)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    pinfo = &(hisifd->panel_info);

    if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_HS_SEND_SEQUENCE)
    {
        LOG_JANK_D(JLID_KERNEL_LCD_POWER_OFF, "%s", "LCD_POWER_OFF");
        // backlight off
        hisi_lcd_backlight_off(pdev);
        lcdkit_info.lcdkit_off_cmd(hisifd, &lcdkit_info.panel_infos.display_off_cmds);

        if (lcdkit_info.panel_infos.display_on_in_backlight)
        {
            lcdkit_info.panel_infos.display_on_need_send = false;
        }

        if ( g_tskit_ic_type && !hisifd->fb_shutdown)
        {
            ret = ts_kit_power_control_notify(TS_BEFORE_SUSPEND, SHORT_SYNC_TIMEOUT);

            if (ret)
            {
                LCDKIT_ERR("ts before suspend err\n");
            }

            ret = ts_kit_power_control_notify(TS_SUSPEND_DEVICE, NO_SYNC_TIMEOUT);

            if (ret)
            {
                LCDKIT_ERR("ts suspend device err\n");
            }
        }

        pinfo->lcd_uninit_step = LCD_UNINIT_MIPI_LP_SEND_SEQUENCE;
    }
    else if (pinfo->lcd_uninit_step == LCD_UNINIT_MIPI_LP_SEND_SEQUENCE)
    {
        if(lcdkit_info.panel_infos.panel_ulps_support){
            lcdkit_enter_ulps(hisifd);
            LCDKIT_INFO("enter ulps mode!\n");
        }
        pinfo->lcd_uninit_step = LCD_UNINIT_POWER_OFF;
    }
    else if (pinfo->lcd_uninit_step == LCD_UNINIT_POWER_OFF)
    {
    mdelay(5); 

    if(!lcdkit_is_enter_sleep_mode())
    {
        if(g_ts_kit_platform_data.chip_data->is_parade_solution)
        {
            //ts_kit_check_bootup_upgrade();
        }

        // lcd reset gpio low
        gpio_cmds_tx(lcdkit_gpio_reset_low_cmds, \
                    ARRAY_SIZE(lcdkit_gpio_reset_low_cmds));

        // lcd reset gpio free
        gpio_cmds_tx(lcdkit_gpio_reset_free_cmds, \
                     ARRAY_SIZE(lcdkit_gpio_reset_free_cmds));

        // lcd pinctrl low
        pinctrl_cmds_tx(pdev, lcdkit_pinctrl_low_cmds,
                        ARRAY_SIZE(lcdkit_pinctrl_low_cmds));

        if (lcdkit_bias_is_gpio_ctrl_power())
        {
            LCDKIT_INFO("power is ctrol by gpio!\n");
            gpio_cmds_tx(lcdkit_bias_disable_cmds, \
                         ARRAY_SIZE(lcdkit_bias_disable_cmds));
                gpio_cmds_tx(lcdkit_bias_free_cmds, ARRAY_SIZE(lcdkit_bias_free_cmds));
            }

        if (lcdkit_bias_is_regulator_ctrl_power())
        {
            //vsp/vsn disable
            vcc_cmds_tx(NULL, lcdkit_scharger_bias_disable_cmds, \
                        ARRAY_SIZE(lcdkit_scharger_bias_disable_cmds));
            LCDKIT_INFO("power is ctrol by regulator and the panel is lcd!\n");
        }

        if ( lcdkit_vbat_is_gpio_ctrl_power())
        {
            gpio_cmds_tx(lcdkit_vbat_disable_cmds, \
                         ARRAY_SIZE(lcdkit_vbat_disable_cmds));
            gpio_cmds_tx(lcdkit_vbat_free_cmds, ARRAY_SIZE(lcdkit_vbat_free_cmds)); 
        }
        mdelay(lcdkit_info.panel_infos.delay_af_vbat_off);

        //lcd vcc disable
        if (lcdkit_iovcc_is_regulator_ctrl_power())
        {
            vcc_cmds_tx(pdev, lcdkit_io_vcc_disable_cmds,
                        ARRAY_SIZE(lcdkit_io_vcc_disable_cmds));
        }
        else if ( lcdkit_iovcc_is_gpio_ctrl_power())
        {
            gpio_cmds_tx(lcdkit_iovcc_disable_cmds, \
                         ARRAY_SIZE(lcdkit_iovcc_disable_cmds));
            gpio_cmds_tx(lcdkit_iovcc_free_cmds, ARRAY_SIZE(lcdkit_iovcc_free_cmds)); 
        }
        mdelay(lcdkit_info.panel_infos.delay_af_iovcc_off);

        if ( HYBRID ==  g_tskit_ic_type)
        {
            if (lcdkit_vci_is_regulator_ctrl_power())
            {
                vcc_cmds_tx(pdev, lcdkit_regulator_vci_disable_cmds,
                            ARRAY_SIZE(lcdkit_regulator_vci_disable_cmds));
            }
            else if (lcdkit_vci_is_gpio_ctrl_power())
            {
                gpio_cmds_tx(lcdkit_vci_disable_cmds, \
                             ARRAY_SIZE(lcdkit_vci_disable_cmds));
                gpio_cmds_tx(lcdkit_vci_free_cmds, ARRAY_SIZE(lcdkit_vci_free_cmds));
            }

        }

        mdelay(lcdkit_info.panel_infos.delay_af_vci_off);

        }else
        {
            // lcd reset gpio free
            gpio_cmds_tx(lcdkit_gpio_reset_free_cmds, \
                ARRAY_SIZE(lcdkit_gpio_reset_free_cmds));

            // lcd pinctrl low
            pinctrl_cmds_tx(pdev, lcdkit_pinctrl_low_cmds,
                ARRAY_SIZE(lcdkit_pinctrl_low_cmds));

        }
        if (hisifd->fb_shutdown)
        {
            ts_kit_thread_stop_notify();
        }
        if (lcdkit_info.panel_infos.fps_func_switch && pinfo->fps_updt_support){
            LCDKIT_INFO("fps_updt_support = 0, fps_updt_panel_only = 0\n");
            pinfo->fps_updt_support = 0;
            pinfo->fps_updt_panel_only = 0;
        }
    }
    else
    {
        LCDKIT_ERR("failed to uninit lcd!\n");
    }

    //if(lcdkit_is_oled_panel())
    //  amoled_irq_disable();
    LCDKIT_INFO("fb%d, -!\n", hisifd->index);
    return 0;
}

/*
*name:lcdkit_remove
*function:panel remove
*@pdev:platform device
*/
static int lcdkit_remove(struct platform_device* pdev)
{
    struct hisi_fb_data_type* hisifd = NULL;

    if (NULL == pdev)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    hisifd = platform_get_drvdata(pdev);

    if (!hisifd)
    {
        LCDKIT_ERR("hisifd is NULL Point!\n");
        return 0;
    }

    LCDKIT_DEBUG("fb%d, +.\n", hisifd->index);

    // lcd vcc finit
    vcc_cmds_tx(pdev, lcdkit_vcc_finit_cmds,
                ARRAY_SIZE(lcdkit_vcc_finit_cmds));

    if (lcdkit_is_lcd_panel())
    {
        ssize_t ret = 0;
        // scharger vcc finit
        ret = vcc_cmds_tx(pdev, lcdkit_scharger_bias_put_cmds,
                    ARRAY_SIZE(lcdkit_scharger_bias_put_cmds));
        if (ret != 0)
        {
            LCDKIT_ERR("LCD scharger vcc failed!\n");
        }
    }

    // lcd pinctrl finit
    pinctrl_cmds_tx(pdev, lcdkit_pinctrl_finit_cmds,
                    ARRAY_SIZE(lcdkit_pinctrl_finit_cmds));

    LCDKIT_DEBUG("fb%d, -.\n", hisifd->index);

    return 0;
}

/*
*name:lcdkit_set_backlight
*function:set backlight level
*@pdev:platform device
*/
static int lcdkit_set_backlight(struct platform_device* pdev, uint32_t bl_level)
{
    int ret = 0;
    struct hisi_fb_data_type* hisifd = NULL;
    static uint32_t last_bl_level = 255;
    static uint32_t jank_last_bl_level = 0;

    if (NULL == pdev)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    hisifd = platform_get_drvdata(pdev);

    if (NULL == hisifd)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    if(lcdkit_info.panel_infos.display_on_in_backlight && lcdkit_info.panel_infos.display_on_need_send)
    { 
        LCDKIT_INFO("Set display on before backlight set\n");
        hisifb_activate_vsync(hisifd);
        lcdkit_dsi_tx(hisifd, &lcdkit_info.panel_infos.display_on_in_backlight_cmds);
        hisifb_deactivate_vsync(hisifd);
        lcdkit_info.panel_infos.display_on_need_send = false;
    }

    if (lcdkit_info.panel_infos.dis_on_cmds_delay_margin_support
        && lcdkit_info.panel_infos.dis_on_cmds_panel_on_tag)
    {
        lcdkit_display_on_cmds_check_delay();
    }
    LCDKIT_DEBUG("fb%d, bl_level=%d.\n", hisifd->index, bl_level);
    if(jank_last_bl_level == 0 && bl_level != 0)
    {
        LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_ON, "LCD_BACKLIGHT_ON,%u", bl_level);
        jank_last_bl_level = bl_level;
    }
    else if (bl_level == 0 && jank_last_bl_level != 0)
    {
        LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_OFF, "LCD_BACKLIGHT_OFF");
        jank_last_bl_level = bl_level;
    }

    if (hisifd->panel_info.bl_set_type & BL_SET_BY_PWM)
    {
        ret = hisi_pwm_set_backlight(hisifd, bl_level);
    }
    else if (hisifd->panel_info.bl_set_type & BL_SET_BY_BLPWM)
    {
        ret = hisi_blpwm_set_backlight(hisifd, bl_level);
    }
    else if (hisifd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM)
    {
        ret = hisi_sh_blpwm_set_backlight(hisifd, bl_level);
    }
    else if ((hisifd->panel_info.bl_set_type & BL_SET_BY_MIPI) && (hisifd->panel_info.bl_max > 0))
    {
        bl_level = (bl_level < hisifd->panel_info.bl_max) ? bl_level : hisifd->panel_info.bl_max;
        LCDKIT_DEBUG("bl_steps =:  %d!\n",hisifd->panel_info.bl_steps);
        switch(hisifd->panel_info.bl_steps){
            case LCD_PANEL_BACKLIGHT_STEP_255:
                lcdkit_info.panel_infos.backlight_cmds.cmds[0].payload[1] = bl_level;
                break;
            case LCD_PANEL_BACKLIGHT_STEP_1024:
                /*follow spec send high 8 bit data to lcd ic DBV[11:4]*/
                lcdkit_info.panel_infos.backlight_cmds.cmds[0].payload[1] = (bl_level & 0x3fc) >> 2;
                /*follow spec low 2 bit data send to DBV[3:2], 0x00 send to DBV[1:0]*/
                lcdkit_info.panel_infos.backlight_cmds.cmds[0].payload[2] = (bl_level & 0x003) << 2;
                break;
            case LCD_PANEL_BACKLIGHT_STEP_4096:
                /*follow spec send high 4 bit data to DBV[11:8]*/
                lcdkit_info.panel_infos.backlight_cmds.cmds[0].payload[1] = (bl_level & 0xf00) >>8;
                /*follow spec send low 8 bit data to lcd ic DBV[7:0]*/
                lcdkit_info.panel_infos.backlight_cmds.cmds[0].payload[2] = bl_level & 0x0ff;
                break;
            default:
                break;
        }
        lcdkit_dsi_tx(hisifd, &lcdkit_info.panel_infos.backlight_cmds);

        if (lcdkit_info.panel_infos.panel_type & PANEL_TYPE_LCD)
        {
            /*enable/disable backlight*/
            down(&lcdkit_info.panel_infos.bl_sem);
            if (bl_level == 0 && last_bl_level != 0)
            {
                vcc_cmds_tx(NULL, lcdkit_scharger_bl_disable_cmds, \
                            ARRAY_SIZE(lcdkit_scharger_bl_disable_cmds));
            }
            else if (last_bl_level == 0 && bl_level != 0)
            {
                vcc_cmds_tx(NULL, lcdkit_scharger_bl_enable_cmds, \
                            ARRAY_SIZE(lcdkit_scharger_bl_enable_cmds));
                LCDKIT_INFO("bl_level = %d!\n", bl_level);
            }

            last_bl_level = bl_level;
            up(&lcdkit_info.panel_infos.bl_sem);
        }
    }
    else
    {
        LCDKIT_ERR("fb%d, not support this bl_set_type(%d)!\n",
                   hisifd->index, hisifd->panel_info.bl_set_type);
    }

    LCDKIT_DEBUG("fb%d, -.\n", hisifd->index);

    return ret;
}

static int lcdkit_esd_check(struct platform_device* pdev)
{
    struct hisi_fb_data_type* hisifd = NULL;
    int ret = 0;
    uint32_t temp = 0;

    if (NULL == pdev)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    hisifd = platform_get_drvdata(pdev);

    if (NULL == hisifd)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    /*check platform esd*/
    if (g_lcdkit_pri_info.platform_esd_support)
    {
        temp = inp32(hisifd->mipi_dsi0_base + g_lcdkit_pri_info.platform_esd_reg);
        if (temp & g_lcdkit_pri_info.platform_esd_value)
        {
            LCDKIT_ERR("Platform open esd err, reg:0x%x, value:0x%x\n", g_lcdkit_pri_info.platform_esd_reg, temp);
            hisifd->esd_recover_state = ESD_RECOVER_STATE_START;
            return 1;
        }
        LCDKIT_INFO("Platform esd check, reg:0x%x, value:0x%x\n", g_lcdkit_pri_info.platform_esd_reg, temp);
    }

    ret = lcdkit_info.lcdkit_check_esd(hisifd);
    return ret;
}

static void lcdkit_snd_cmd_before_frame(struct platform_device* pdev)
{
    struct hisi_fb_data_type* hisifd = NULL;
    int ret = 0;

    LCDKIT_DEBUG("+\n");
    if (NULL == pdev)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    hisifd = platform_get_drvdata(pdev);

    if (NULL == hisifd)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    lcdkit_dsi_tx(hisifd, &lcdkit_info.panel_infos.snd_cmd_before_frame_cmds);

    LCDKIT_DEBUG("-\n");
    return ;
}

/*
*name:lcdkit_set_fastboot
*function:set fastboot display
*@pdev:platform device
*/
static int lcdkit_set_fastboot(struct platform_device* pdev)
{
    struct hisi_fb_data_type* hisifd = NULL;

    if (NULL == pdev)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    hisifd = platform_get_drvdata(pdev);

    if (NULL == hisifd)
    {
        LCDKIT_ERR("NULL Pointer\n");
        return -EINVAL;
    }

    if(CHECKED_ERROR == mipi_lcdkit_btb_check()) {
        LCDKIT_ERR("btb checked failed!");
    }

    if (lcdkit_is_lcd_panel())
    {
        if ( lcdkit_bias_is_regulator_ctrl_power())
        {
            /*set scharger vcc*/
            vcc_cmds_tx(NULL, lcdkit_scharger_bias_set_cmds, \
                        ARRAY_SIZE(lcdkit_scharger_bias_set_cmds));

            /*scharger vcc enable*/
            vcc_cmds_tx(NULL, lcdkit_scharger_bias_enable_cmds, \
                        ARRAY_SIZE(lcdkit_scharger_bias_enable_cmds));

            /*scharger bl enable*/
            vcc_cmds_tx(NULL, lcdkit_scharger_bl_enable_cmds, \
                        ARRAY_SIZE(lcdkit_scharger_bl_enable_cmds));
        }
    }

    // lcd pinctrl normal
    pinctrl_cmds_tx(pdev, lcdkit_pinctrl_normal_cmds,
                    ARRAY_SIZE(lcdkit_pinctrl_normal_cmds));

    // lcd reset gpio request
    gpio_cmds_tx(lcdkit_gpio_reset_request_cmds,
                 ARRAY_SIZE(lcdkit_gpio_reset_request_cmds));

    // backlight on
    hisi_lcd_backlight_on(pdev);

    return 0;
}


static int lcdkit_set_display_region(struct platform_device* pdev, struct dss_rect* dirty)
{
    struct hisi_fb_data_type *hisifd = NULL;

    if (NULL == pdev)
    {
        LCDKIT_ERR("pdev NULL Pointer!\n");
        return -1;
    }

    hisifd = platform_get_drvdata(pdev);
    if (NULL == hisifd)
    {
        LCDKIT_ERR("NULL Pointer!\n");
        return -1;
    }

    if (!lcdkit_info.panel_infos.display_region_support)
    {
      LCDKIT_DEBUG("fps updt is not support!\n");
      return -1;
    }

    return lcdkit_info.lcdkit_set_display_region(hisifd, dirty);

}

static int lcdkit_fps_scence_handle(struct platform_device* pdev, uint32_t scence)
{
    if (!lcdkit_info.panel_infos.fps_func_switch)
    {
      LCDKIT_DEBUG("fps updt is not support!\n");
      return 0;
    }

    return lcdkit_info.lcdkit_fps_scence_handle(pdev, scence);

}

static int lcdkit_fps_updt_handle(void* pdata)
{
    int ret = 0;

    if (lcdkit_info.panel_infos.fps_func_switch)
    {
        LCDKIT_DEBUG("fps updt is support!\n");
        ret = lcdkit_info.lcdkit_fps_updt_handle(pdata);
    }
    else
    {
        LCDKIT_ERR("fps updt is not support!\n");
    }

    return ret;
}

static int lcdkit_ce_mode_show(void* pdata, char* buf)
{
    int ret = 0;

    if (lcdkit_info.panel_infos.ce_support)
    {
        LCDKIT_INFO("ce is support!\n");
        ret = lcdkit_info.lcdkit_ce_mode_show(pdata,buf);
    }
    else
    {
        LCDKIT_INFO("ce is not support!\n");
    }

    return ret;
}

static int lcdkit_ce_mode_store(void* pdata, const char* buf, size_t count)
{
    int ret = 0;
    unsigned long val = 0;
    struct hisi_fb_data_type *hisifd = NULL;
    struct platform_device *pdev = NULL;


    pdev = (struct platform_device *)pdata;
    hisifd = platform_get_drvdata(pdev);

    if (NULL == hisifd)
    {
        LCDKIT_ERR("NULL Pointer!\n");
        return -1;
    }

    ret = strict_strtoul(buf, 0, &val);
    if (ret)
        return ret;

    hisifd->user_scene_mode = (int)val;

    if (lcdkit_info.panel_infos.ce_support)
    {
        LCDKIT_INFO("ce is support!\n");
        ret = lcdkit_info.lcdkit_ce_mode_store(hisifd, buf, count);
    }
    else
    {
        LCDKIT_INFO("ce updt is not support!\n");
    }

    return ret;
}

/*******************************************************************************
**
*/
static struct hisi_panel_info lcdkit_panel_info = {0};
static struct hisi_fb_panel_data lcdkit_data =
{
    .panel_info = &lcdkit_panel_info,
    .set_fastboot = lcdkit_set_fastboot,
    .on = lcdkit_on,
    .off = lcdkit_off,
    .remove = lcdkit_remove,
    .set_backlight = lcdkit_set_backlight,
    .esd_handle = lcdkit_esd_check,
    .set_display_region = lcdkit_set_display_region,
    .lcd_fps_scence_handle = lcdkit_fps_scence_handle,
    .lcd_fps_updt_handle = lcdkit_fps_updt_handle,
    .lcd_ce_mode_show = lcdkit_ce_mode_show,
    .lcd_ce_mode_store = lcdkit_ce_mode_store,
    .snd_cmd_before_frame = lcdkit_snd_cmd_before_frame,
};

/*
*name:lcdkit_probe
*function:panel driver probe
*@pdev:platform device
*/
static int __init lcdkit_probe(struct platform_device* pdev)
{
    struct hisi_panel_info* pinfo = NULL;
    struct device_node* np = NULL;
    int ret = 0;
    struct platform_device* hisi_pdev=NULL;

    np = of_find_compatible_node(NULL, NULL, lcdkit_info.panel_infos.lcd_compatible);
    if (!np)
    {
        LCDKIT_ERR("NOT FOUND device node %s!\n", lcdkit_info.panel_infos.lcd_compatible);
        return -EINVAL;
    }

    pinfo = lcdkit_data.panel_info;
    memset(pinfo, 0, sizeof(struct hisi_panel_info));
    lcdkit_init(np, pinfo);

    if (hisi_fb_device_probe_defer(lcdkit_info.panel_infos.lcd_disp_type, lcdkit_info.panel_infos.bl_type))
    {
        goto err_probe_defer;
    }

    /*init sem*/
    sema_init(&lcdkit_info.panel_infos.bl_sem, 1);
    pdev->id = 1;

    if (runmode_is_factory())
    {
        pinfo->esd_enable = 0;
        pinfo->dirty_region_updt_support = 0;
        pinfo->prefix_ce_support = 0;
        pinfo->prefix_sharpness1D_support = 0;
        pinfo->prefix_sharpness2D_support = 0;
        pinfo->sbl_support = 0;
        pinfo->acm_support = 0;
        pinfo->acm_ce_support = 0;
        pinfo->esd_enable = 0;
        pinfo->blpwm_precision_type = BLPWM_PRECISION_DEFAULT_TYPE;
        pinfo->comform_mode_support = 0;
        pinfo->color_temp_rectify_support = 0;
        lcdkit_info.panel_infos.fps_func_switch = 0;
        if(lcdkit_info.panel_infos.dynamic_gamma_support)
        {
#if defined (CONFIG_HISI_FB_3660)
             hisifb_update_gm_from_reserved_mem(pinfo->gamma_lut_table_R,
                            pinfo->gamma_lut_table_G,
                            pinfo->gamma_lut_table_B,
                            pinfo->igm_lut_table_R,
                            pinfo->igm_lut_table_G,
                            pinfo->igm_lut_table_B);
#endif
        }
        pinfo->hiace_support = 0;
        pinfo->arsr1p_sharpness_support = 0;
        pinfo->blpwm_input_ena = 0;
        lcdkit_info.panel_infos.display_effect_on_support = 0;
        lcdkit_info.panel_infos.effect_support_mode = 0;
        pinfo->gmp_support = 0;
    }

    if (lcdkit_info.panel_infos.fps_func_switch){
        lcdkit_info.lcdkit_fps_timer_init();
    }

    /*effect init*/
    lcdkit_effect_get_data(lcdkit_panel_init(lcdkit_info.panel_infos.product_id, lcdkit_info.panel_infos.lcd_compatible), pinfo);
    /*vsp/vsn bl init*/
    lcdkit_bias_init(pdev);

    /*1.8 & 3.1 init*/
    ret = lcdkit_vcc_init(pdev);

    if (ret)
    {
        LCDKIT_ERR("vcc init fail!\n");
        goto err_return;
    }

    /*delay init*/
    lcdkit_reset_delay_init(lcdkit_gpio_reset_normal_cmds);
    lcdkit_bias_on_delay_init_regulator(lcdkit_scharger_bias_enable_cmds);
    lcdkit_bias_off_delay_init_regulator(lcdkit_scharger_bias_disable_cmds);
    lcdkit_bias_on_delay_init_gpio(lcdkit_bias_enable_cmds);
    lcdkit_bias_off_delay_init_gpio(lcdkit_bias_disable_cmds);

    ret = lcdkit_pinctrl_init(pdev);

    if (ret)
    {
        LCDKIT_ERR("pinctrl init fail!\n");
        goto err_return;
    }

    // lcd vcc enable
    if (is_fastboot_display_enable())
    {
       if ( HYBRID == g_tskit_ic_type)
        {
            if (lcdkit_vci_is_regulator_ctrl_power())
            {
                vcc_cmds_tx(pdev, lcdkit_regulator_vci_enable_cmds,
                            ARRAY_SIZE(lcdkit_regulator_vci_enable_cmds));
            }
        }

        if (lcdkit_iovcc_is_regulator_ctrl_power())
        {
            vcc_cmds_tx(pdev, lcdkit_io_vcc_enable_cmds,
                        ARRAY_SIZE(lcdkit_io_vcc_enable_cmds));
        }
    }

    lcdkit_gpio_request();

    // alloc panel device data
    ret = platform_device_add_data(pdev, &lcdkit_data, sizeof(struct hisi_fb_panel_data));

    if (ret)
    {
        LCDKIT_ERR("platform_device_add_data failed!\n");
        goto err_device_put;
    }

    hisi_pdev = hisi_fb_add_device(pdev);
    lcdkit_debugfs_init();
    lcdkit_set_pdev(hisi_pdev);
    LCDKIT_INFO("exit succ!!!!\n");

    return 0;

err_device_put:
    platform_device_put(pdev);
err_return:
    return ret;
err_probe_defer:
    return -EPROBE_DEFER;

    return ret;
}

/***********************************************************
*platform driver definition
***********************************************************/
/*
*probe match table
*/
static struct of_device_id lcdkit_match_table[] =
{
    {
        .compatible = "auo_otm1901a_5p2_1080p_video",
        .data = NULL,
    },
    {},
};

/*
*panel platform driver
*/
static struct platform_driver lcdkit_driver =
{
    .probe = lcdkit_probe,
    .remove = NULL,
    .suspend = NULL,
    .resume = NULL,
    .shutdown = NULL,
    .driver = {
        .name = "lcdkit_mipi_panel",
        .of_match_table = lcdkit_match_table,
    },
};

/*
*name:lcdkit_init
*function:panel init
*/
static int __init lcdkit_disp_init(void)
{
    int ret = 0, len = 0;
    struct device_node* np = NULL;

#if defined(CONFIG_LCDKIT_DRIVER)
    if(!get_lcdkit_support())
    {
       HISI_FB_INFO("lcdkit is not support!\n");
       return ret;
    }
#endif

    np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCDKIT_PANEL_TYPE);

    if (!np)
    {
        LCDKIT_ERR("NOT FOUND device node %s!\n", DTS_COMP_LCDKIT_PANEL_TYPE);
        ret = -1;
        return ret;
    }

    memset(&lcdkit_info.panel_infos , 0 , sizeof(struct lcdkit_panel_infos));

    OF_PROPERTY_READ_U32_RETURN(np, "product_id", &lcdkit_info.panel_infos.product_id);
    LCDKIT_INFO("lcdkit_info.panel_infos.product_id = %d", lcdkit_info.panel_infos.product_id);

    lcdkit_info.panel_infos.lcd_compatible = (char*)of_get_property(np, "lcd_panel_type", NULL);

    lcdkit_info.panel_infos.isbulcked = false;
    if (!lcdkit_info.panel_infos.lcd_compatible)
    {
        LCDKIT_ERR("Is not normal lcd and return\n");
        return ret;
    }
    else
    {
        if(!strncmp(lcdkit_info.panel_infos.lcd_compatible, "auo_otm1901a_5p2_1080p_video_default",strlen("auo_otm1901a_5p2_1080p_video_default")))
        {
           LCDKIT_INFO("the panel is not buckled! \n");
           return ret;
        }
        lcdkit_info.panel_infos.isbulcked = true;
        len = strlen(lcdkit_info.panel_infos.lcd_compatible);
        memset( (char *)lcdkit_driver.driver.of_match_table->compatible, 0, LCDKIT_PANEL_COMP_LENGTH);
        strncpy( (char *)lcdkit_driver.driver.of_match_table->compatible, lcdkit_info.panel_infos.lcd_compatible,
			len > (LCDKIT_PANEL_COMP_LENGTH - 1) ? (LCDKIT_PANEL_COMP_LENGTH - 1) : len);
        LCDKIT_INFO("g_lcd_type=%s, len = %u!\n", lcdkit_info.panel_infos.lcd_compatible, strlen(lcdkit_info.panel_infos.lcd_compatible));
    }

    ret = platform_driver_register(&lcdkit_driver);

    if (ret)
    {
        LCDKIT_ERR("platform_driver_register failed, error=%d!\n", ret);
    }

    return ret;
}

module_init(lcdkit_disp_init);

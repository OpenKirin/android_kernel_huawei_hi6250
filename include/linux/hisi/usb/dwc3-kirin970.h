enum phy_change_type {
	PHY_MODE_CHANGE_BEGIN,
	PHY_MODE_CHANGE_END,
};
int kirin970_usb31_phy_notify(enum phy_change_type type);
void dwc3_misc_ctrl_get(void);
void dwc3_misc_ctrl_put(void);
volatile unsigned int hisi_dwc3_usbcore_read(u32 offset);
void hisi_dwc3_usbcore_write(u32 offset, u32 value);

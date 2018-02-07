ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)),)
obj-$(CONFIG_INPUTHUB) += default/
else
obj-$(CONFIG_INPUTHUB) += $(TARGET_BOARD_PLATFORM)/
endif


#
# FLP PDR configuration
#

ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)/
endif

obj-$(CONFIG_HISI_FLP)	+=hisi_softtimer.o hisi_flp.o hisi_flp_dt.o

#
#
#================================================================
#   
#   
#   文件名称：user.mk
#   创 建 者：肖飞
#   创建日期：2019年10月25日 星期五 13时04分38秒
#   修改日期：2021年02月04日 星期四 10时52分25秒
#   描    述：
#
#================================================================

USER_C_INCLUDES += -Iapps
USER_C_INCLUDES += -Iapps/modules
USER_C_INCLUDES += -Iapps/modules/os
USER_C_INCLUDES += -Iapps/modules/drivers
USER_C_INCLUDES += -Iapps/modules/hardware
USER_C_INCLUDES += -Iapps/modules/app
USER_C_INCLUDES += -Iapps/modules/app/vfs_disk
USER_C_INCLUDES += -Iapps/modules/app/ftpd
USER_C_INCLUDES += -Iapps/modules/tests

C_INCLUDES += $(USER_C_INCLUDES)

USER_C_SOURCES += apps/app.c
USER_C_SOURCES += apps/probe_tool_handler.c
USER_C_SOURCES += apps/uart_debug_handler.c
USER_C_SOURCES += apps/os_memory.c
USER_C_SOURCES += apps/early_sys_callback.c
USER_C_SOURCES += apps/usbh_user_callback.c

USER_C_SOURCES += apps/modules/app/eeprom_config.c
USER_C_SOURCES += apps/modules/app/poll_loop.c
USER_C_SOURCES += apps/modules/app/probe_tool.c
USER_C_SOURCES += apps/modules/app/uart_debug.c
USER_C_SOURCES += apps/modules/app/file_log.c
USER_C_SOURCES += apps/modules/app/request.c
USER_C_SOURCES += apps/modules/app/net_client.c
USER_C_SOURCES += apps/modules/app/net_protocol_udp.c
USER_C_SOURCES += apps/modules/app/net_protocol_tcp.c
USER_C_SOURCES += apps/modules/app/net_protocol_ws.c
USER_C_SOURCES += apps/modules/app/request_default.c
USER_C_SOURCES += apps/modules/app/https.c
USER_C_SOURCES += apps/modules/app/request_ws.c
USER_C_SOURCES += apps/modules/app/ftp_client.c
USER_C_SOURCES += apps/modules/app/net_callback.c
USER_C_SOURCES += apps/modules/app/vfs_disk/vfs.c
##C_SOURCES := $(filter-out Middlewares/Third_Party/FatFs/src/diskio.c ,$(C_SOURCES))
##USER_C_SOURCES += apps/modules/app/vfs_ramdisk/pseudo_disk_io.c
#USER_C_SOURCES += apps/modules/app/ftpd/ftpd.c
USER_C_SOURCES += apps/modules/app/ftpd/ftpd_rtt.c
USER_C_SOURCES += apps/modules/app/ftpd/ftpd_file_rtt.c
USER_C_SOURCES += apps/modules/app/mt_file.c
USER_C_SOURCES += apps/modules/app/can_data_task.c
USER_C_SOURCES += apps/modules/app/can_config.c
USER_C_SOURCES += apps/modules/app/duty_cycle_pattern.c
USER_C_SOURCES += apps/modules/app/usbh_user_callback.c
USER_C_SOURCES += apps/modules/app/early_sys_callback.c
USER_C_SOURCES += apps/modules/app/usb_upgrade.c
USER_C_SOURCES += apps/modules/hardware/flash.c
USER_C_SOURCES += apps/modules/hardware/eeprom.c
USER_C_SOURCES += apps/modules/drivers/spi_txrx.c
USER_C_SOURCES += apps/modules/drivers/can_txrx.c
USER_C_SOURCES += apps/modules/drivers/usart_txrx.c
USER_C_SOURCES += apps/modules/os/event_helper.c
USER_C_SOURCES += apps/modules/os/callback_chain.c
USER_C_SOURCES += apps/modules/os/bitmap_ops.c
USER_C_SOURCES += apps/modules/os/iap.c
USER_C_SOURCES += apps/modules/os/os_utils.c
USER_C_SOURCES += apps/modules/os/net_utils.c
USER_C_SOURCES += apps/modules/os/cpu_utils.c
USER_C_SOURCES += apps/modules/os/soft_timer.c
USER_C_SOURCES += apps/modules/os/log.c
USER_C_SOURCES += apps/modules/os/object_class.c
USER_C_SOURCES += apps/modules/tests/test_serial.c
USER_C_SOURCES += apps/modules/tests/test_event.c
USER_C_SOURCES += apps/modules/tests/test_can.c
USER_C_SOURCES += apps/modules/tests/test_soft_timer.c
USER_C_SOURCES += apps/modules/tests/test_object_class.c

C_SOURCES += $(USER_C_SOURCES)

USER_CFLAGS += -DtraceTASK_SWITCHED_IN=StartIdleMonitor -DtraceTASK_SWITCHED_OUT=EndIdleMonitor

CFLAGS += $(USER_CFLAGS)
LDFLAGS += -u _printf_float

IAP_FILE := apps/modules/os/iap.h

define update-iap-include
	if [ -f $(IAP_FILE) ]; then
		touch $(IAP_FILE);
	fi
endef

ifeq ("$(origin APP)", "command line")
build-type := .app.stamps
build-type-invalid := .bootloader.stamps
CFLAGS += -DUSER_APP
LDSCRIPT = STM32F407VETx_FLASH_APP.ld
#$(info $(shell $(update-iap-include)))
$(info "build app!")
else
build-type := .bootloader.stamps
build-type-invalid := .app.stamps
LDSCRIPT = STM32F407VETx_FLASH.ld
#$(info $(shell $(update-iap-include)))
$(info "build bootloader!")
endif

default: all

all : $(build-type)

$(build-type) :
	-rm $(build-type-invalid)
	$(shell $(update-iap-include))
	touch $@

cscope: all
	rm cscope e_cs -rf
	mkdir -p cscope
	#$(silent)tags.sh prepare;
	$(silent)touch dep_files;
	$(silent)for f in $$(find . -type f -name "*.d" 2>/dev/null); do \
		for i in $$(cat "$$f" | sed 's/^.*://g' | sed 's/[\\ ]/\n/g' | sort -h | uniq); do \
			if test "$${i:0:1}" = "/";then \
				echo "$$i" >> dep_files; \
			else \
				readlink -f "$$i" >> dep_files; \
			fi; \
		done; \
	done;
	$(silent)cat dep_files | sort | uniq | sed 's/^\(.*\)$$/\"\1\"/g' >> cscope/cscope.files;
	$(silent)cat dep_files | sort | uniq >> cscope/ctags.files;
	$(silent)rm dep_files
	$(silent)tags.sh cscope;
	$(silent)tags.sh tags;
	$(silent)tags.sh env;

clean: clean-cscope
clean-cscope:
	rm cscope e_cs -rf



/*================================================================
 *
 *
 *   文件名称：app.c
 *   创 建 者：肖飞
 *   创建日期：2019年10月11日 星期五 16时54分03秒
 *   修改日期：2021年03月23日 星期二 10时22分38秒
 *   描    述：
 *
 *================================================================*/
#include "app.h"

#include <string.h>

#include "app_platform.h"
#include "cmsis_os.h"

#include "iwdg.h"

#include "os_utils.h"
#include "eeprom_layout.h"
#include "test_serial.h"
#include "test_event.h"
#include "test_can.h"
#include "test_soft_timer.h"
#include "test_object_class.h"
#include "usart_txrx.h"
#include "probe_tool.h"
#include "file_log.h"
#include "uart_debug.h"
#include "net_client.h"
#include "ftp_client.h"
#include "ftpd/ftpd_rtt.h"
#include "usb_upgrade.h"

#include "log.h"

#include "duty_cycle_pattern.h"

extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern SPI_HandleTypeDef hspi3;

static app_info_t *app_info = NULL;
static eeprom_info_t *eeprom_info = NULL;
static os_signal_t app_event = NULL;

app_info_t *get_app_info(void)
{
	return app_info;
}

static int app_load_config(void)
{
	eeprom_layout_t *eeprom_layout = get_eeprom_layout();
	size_t offset = (size_t)&eeprom_layout->mechine_info.mechine;
	debug("offset:%d", offset);
	return eeprom_load_config_item(eeprom_info, "eva", &app_info->mechine, sizeof(mechine_info_t), offset);
}

int app_save_config(void)
{
	eeprom_layout_t *eeprom_layout = get_eeprom_layout();
	size_t offset = (size_t)&eeprom_layout->mechine_info.mechine;
	debug("offset:%d", offset);
	return eeprom_save_config_item(eeprom_info, "eva", &app_info->mechine, sizeof(mechine_info_t), offset);
}

void app_init(void)
{
	app_event = signal_create(1);
}

void send_app_event(app_event_t event)
{
	signal_send(app_event, event, 0);
}

void app(void const *argument)
{

	poll_loop_t *poll_loop;
	add_log_handler((log_fn_t)log_uart_data);
	add_log_handler((log_fn_t)log_udp_data);
	add_log_handler((log_fn_t)log_file_data);

	{
		uart_info_t *uart_info = get_or_alloc_uart_info(&huart1);

		if(uart_info == NULL) {
			app_panic();
		}

		set_log_uart_info(uart_info);

		osThreadDef(uart_debug, task_uart_debug, osPriorityNormal, 0, 128 * 2 * 2);
		osThreadCreate(osThread(uart_debug), uart_info);
	}

	poll_loop = get_or_alloc_poll_loop(0);

	if(poll_loop == NULL) {
		app_panic();
	}

	probe_broadcast_add_poll_loop(poll_loop);
	probe_server_add_poll_loop(poll_loop);

	//while(is_log_server_valid() == 0) {
	//	osDelay(1);
	//}

	debug("===========================================start app============================================");

	app_info = (app_info_t *)os_calloc(1, sizeof(app_info_t));

	OS_ASSERT(app_info != NULL);

	eeprom_info = get_or_alloc_eeprom_info(get_or_alloc_spi_info(&hspi3),
	                                       spi3_cs_GPIO_Port,
	                                       spi3_cs_Pin,
	                                       spi3_wp_GPIO_Port,
	                                       spi3_wp_Pin);

	OS_ASSERT(eeprom_info != NULL);

	if(app_load_config() == 0) {
		debug("app_load_config successful!");
		debug("device id:\'%s\', server host:\'%s\', server port:\'%s\'!", app_info->mechine.device_id, app_info->mechine.host, app_info->mechine.port);
		app_info->available = 1;
	} else {
		debug("app_load_config failed!");
		snprintf(app_info->mechine.device_id, sizeof(app_info->mechine.device_id), "%s", "0000000000");
		snprintf(app_info->mechine.host, sizeof(app_info->mechine.host), "%s", "112.74.40.227");
		snprintf(app_info->mechine.port, sizeof(app_info->mechine.port), "%s", "12345");
		snprintf(app_info->mechine.path, sizeof(app_info->mechine.path), "%s", "");
		debug("device id:\'%s\', server host:\'%s\', server port:\'%s\'!", app_info->mechine.device_id, app_info->mechine.host, app_info->mechine.port);
		app_info->mechine.upgrade_enable = 0;
		app_save_config();
		app_info->available = 1;
	}

	//net_client_add_poll_loop(poll_loop);
	//ftp_client_add_poll_loop(poll_loop);

	//ftpd_init();
	start_ftpd(NULL);

	//test_event();
	//test_sys_class();
	//test_can();
	//test_object_class();
	//test_soft_timer();
	//test_serial(&huart1);
	//test_serial(&huart3);

	while(1) {
		uint32_t event;
		int ret = signal_wait(app_event, &event, 1000);

		if(ret == 0) {
			switch(event) {
				case APP_EVENT_USB: {
					start_usb_upgrade();
				}
				break;

				default: {
				}
				break;
			}
		}

		//handle_open_log();
		handle_usb_upgrade();
	}
}

static pattern_state_t work_pattern_state = {
	.type = PWM_COMPARE_COUNT_UP,
	.duty_cycle = 0,
};

static void update_work_led(void)
{
	//计数值小于duty_cycle,输出1;大于duty_cycle输出0
	uint16_t duty_cycle = get_duty_cycle_pattern(&work_pattern_state, 1000, 0, 20);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, duty_cycle);
}

void idle(void const *argument)
{
	MX_IWDG_Init();
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);

	while(1) {
		HAL_IWDG_Refresh(&hiwdg);
		update_work_led();
		osDelay(10);
	}
}

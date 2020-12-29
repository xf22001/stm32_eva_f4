

/*================================================================
 *
 *
 *   文件名称：app.c
 *   创 建 者：肖飞
 *   创建日期：2019年10月11日 星期五 16时54分03秒
 *   修改日期：2020年12月29日 星期二 17时17分26秒
 *   描    述：
 *
 *================================================================*/
#include "app.h"

#include <string.h>

#include "app_platform.h"
#include "cmsis_os.h"

#include "iwdg.h"

#include "os_utils.h"
#include "test_serial.h"
#include "test_event.h"
#include "test_map_utils.h"
#include "test_sys_class.h"
#include "probe_tool.h"
#include "net_client.h"
#include "ftp_client.h"
#include "ftpd/ftpd.h"
#include "ftpd/ftpd_rtt.h"

#include "config_list.h"

#include "log.h"

#include "file_log.h"

extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart3;
extern SPI_HandleTypeDef hspi3;

static app_info_t *app_info = NULL;
static eeprom_info_t *eeprom_info = NULL;

app_info_t *get_app_info(void)
{
	return app_info;
}

static int app_load_config(void)
{
	return eeprom_load_config_item(eeprom_info, "eva", &app_info->mechine, sizeof(mechine_info_t), 0);
}

int app_save_config(void)
{
	return eeprom_save_config_item(eeprom_info, "eva", &app_info->mechine, sizeof(mechine_info_t), 0);
}

#define test_config_get(cls, key) do { \
	debug("get cls:%u, key:%u, value:%p\n", cls, key, config_get_class_key_value(cls, key)); \
} while(0)

void test_config(void)
{
	config_init();

	config_set_class_key_value(0, 1, (void *)1);
	config_dump_class_key_value();
	config_set_class_key_value(0, 2, (void *)1);
	config_dump_class_key_value();
	config_set_class_key_value(2, 1, (void *)1);
	config_dump_class_key_value();
	config_set_class_key_value(2, 2, (void *)1);
	config_dump_class_key_value();
	test_config_get(0, 1);
	test_config_get(0, 2);
	test_config_get(0, 3);
	test_config_get(1, 3);
	test_config_get(2, 2);
}

void app(void const *argument)
{

	poll_loop_t *poll_loop;
	config_init();
	mt_file_environment_init();

	{
		uart_info_t *uart_info = get_or_alloc_uart_info(&huart3);

		if(uart_info == NULL) {
			app_panic();
		}

		set_log_uart_info(uart_info);

		osThreadDef(uart_debug, task_uart_debug, osPriorityNormal, 0, 128 * 2 * 2);
		osThreadCreate(osThread(uart_debug), uart_info);
	}

	//{
	//	uart_info_t *uart_info = get_or_alloc_uart_info(&huart3);

	//	if(uart_info == NULL) {
	//		app_panic();
	//	}

	//	osThreadDef(task_test_serial, task_test_serial, osPriorityNormal, 0, 128);
	//	osThreadCreate(osThread(task_test_serial), uart_info);
	//}

	poll_loop = get_or_alloc_poll_loop(0);

	if(poll_loop == NULL) {
		app_panic();
	}

	osThreadDef(poll_loop, task_poll_loop, osPriorityNormal, 0, 128 * 2 * 8);
	osThreadCreate(osThread(poll_loop), poll_loop);

	probe_broadcast_add_poll_loop(poll_loop);
	probe_server_add_poll_loop(poll_loop);

	//while(is_log_server_valid() == 0) {
	//	osDelay(1);
	//}

	debug("===========================================start app============================================\n");

	app_info = (app_info_t *)os_alloc(sizeof(app_info_t));

	if(app_info == NULL) {
		app_panic();
	}

	memset(app_info, 0, sizeof(app_info_t));

	eeprom_info = get_or_alloc_eeprom_info(&hspi3,
	                                       spi3_cs_GPIO_Port,
	                                       spi3_cs_Pin,
	                                       spi3_wp_GPIO_Port,
	                                       spi3_wp_Pin);

	if(eeprom_info == NULL) {
		app_panic();
	}

	if(app_load_config() == 0) {
		debug("app_load_config successful!\n");
		debug("device id:\'%s\', server host:\'%s\', server port:\'%s\'!\n", app_info->mechine.device_id, app_info->mechine.host, app_info->mechine.port);
		app_info->available = 1;
	} else {
		debug("app_load_config failed!\n");
		snprintf(app_info->mechine.device_id, sizeof(app_info->mechine.device_id), "%s", "0000000000");
		snprintf(app_info->mechine.host, sizeof(app_info->mechine.host), "%s", "112.74.40.227");
		snprintf(app_info->mechine.port, sizeof(app_info->mechine.port), "%s", "12345");
		snprintf(app_info->mechine.path, sizeof(app_info->mechine.path), "%s", "");
		debug("device id:\'%s\', server host:\'%s\', server port:\'%s\'!\n", app_info->mechine.device_id, app_info->mechine.host, app_info->mechine.port);
		app_save_config();
		app_info->available = 1;
	}

	//net_client_add_poll_loop(poll_loop);
	//ftp_client_add_poll_loop(poll_loop);

	//ftpd_init();
	start_ftpd(NULL);

	//test_config();
	
	test_event();
	//test_map_utils();
	//test_sys_class();

	while(1) {
		handle_open_log();
		osDelay(1000);
	}
}

typedef enum {
	PWM_COMPARE_COUNT_UP = 0,
	PWM_COMPARE_COUNT_DOWN,
	PWM_COMPARE_COUNT_KEEP,
} compare_count_type_t;

static void update_work_led(void)
{
	static compare_count_type_t type = PWM_COMPARE_COUNT_UP;
	static uint16_t duty_cycle = 0;
	static uint16_t keep_count = 0;
	//计数值小于duty_cycle,输出1;大于duty_cycle输出0

	switch(type) {
		case PWM_COMPARE_COUNT_UP: {

			if(duty_cycle < 1000) {
				duty_cycle += 5;
			} else {
				type = PWM_COMPARE_COUNT_KEEP;
			}
		}
		break;

		case PWM_COMPARE_COUNT_DOWN: {
			if(duty_cycle > 0) {
				duty_cycle -= 5;
			} else {
				type = PWM_COMPARE_COUNT_UP;
			}

		}
		break;

		case PWM_COMPARE_COUNT_KEEP: {
			if(keep_count < duty_cycle) {
				keep_count += 10;
			} else {
				keep_count = 0;
				type = PWM_COMPARE_COUNT_DOWN;
			}

		}
		break;

		default:
			break;
	}

	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, duty_cycle);
}

void idle(void const *argument)
{
	MX_IWDG_Init();
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);

	while(1) {
		HAL_IWDG_Refresh(&hiwdg);
		update_work_led();
		osDelay(1);
	}
}

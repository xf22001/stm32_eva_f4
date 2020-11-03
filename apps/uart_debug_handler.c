

/*================================================================
 *
 *
 *   文件名称：uart_debug_handler.c
 *   创 建 者：肖飞
 *   创建日期：2020年05月13日 星期三 13时18分00秒
 *   修改日期：2020年05月15日 星期五 08时29分20秒
 *   描    述：
 *
 *================================================================*/
#include "uart_debug_handler.h"

#define LOG_UART
#include "log.h"

static void fn1(char *arguments)
{
	_printf("%s:%s:%d arguments:\'%s\'\n", __FILE__, __func__, __LINE__, arguments);
}

static void fn5(char *arguments)
{
	int size = xPortGetFreeHeapSize();
	uint8_t *os_thread_info;
	uint8_t is_app = 0;
	uint32_t ticks = osKernelSysTick();

#if defined(USER_APP)
	is_app = 1;
#endif

	_printf("free heap size:%d\n", size);
	_printf("current ticks:%lu\n", ticks);
	_printf("%lu day %lu hour %lu min %lu sec\n",
	                ticks / (1000 * 60 * 60 * 24),//day
	                (ticks % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60),//hour
	                (ticks % (1000 * 60 * 60)) / (1000 * 60),//min
	                (ticks % (1000 * 60)) / (1000)//sec
	               );

	if(size < 4 * 1024) {
		return;
	}

	size = 1024;

	os_thread_info = (uint8_t *)os_alloc(size);

	if(os_thread_info == NULL) {
		return;
	}

	osThreadList(os_thread_info);

	_puts((const char *)os_thread_info);

	os_free(os_thread_info);

	if(is_app) {
		_printf("in app!\n");
	} else {
		_printf("in bootloader!\n");
	}
}

static uart_fn_item_t uart_fn_map[] = {
	{1, fn1},
	{5, fn5},
};

uart_fn_map_info_t uart_fn_map_info = {
	.uart_fn_map = uart_fn_map,
	.uart_fn_map_size = sizeof(uart_fn_map) / sizeof(uart_fn_item_t),
};

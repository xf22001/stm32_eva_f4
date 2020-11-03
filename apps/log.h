

/*================================================================
 *   
 *   
 *   文件名称：log.h
 *   创 建 者：肖飞
 *   创建日期：2020年05月14日 星期四 14时09分56秒
 *   修改日期：2020年08月21日 星期五 13时13分31秒
 *   描    述：
 *
 *================================================================*/
#ifndef _LOG_H
#define _LOG_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "os_utils.h"
#include "probe_tool.h"
#include "uart_debug.h"

#ifdef __cplusplus
}
#endif

#if defined(LOG_NONE)
#elif defined(LOG_UDP)
#elif defined(LOG_UART)
#elif defined(LOG_ALL)
#else
//#define LOG_NONE
//#define LOG_UDP
//#define LOG_UART
#define LOG_ALL
#endif

#if defined(LOG_NONE)
#define _printf(fmt, ...)
#define _hexdump(label, data, len)
#define _puts(s)
#elif defined(LOG_UDP)
#define _printf(fmt, ...) log_printf((log_fn_t)log_udp_data, fmt, ## __VA_ARGS__)
#define _hexdump(label, data, len) log_hexdump((log_fn_t)log_udp_data, label, data, len)
#define _puts(s) log_puts((log_fn_t)log_udp_data, s)
#elif defined(LOG_UART)
#define _printf(fmt, ...) log_printf((log_fn_t)log_uart_data, fmt, ## __VA_ARGS__)
#define _hexdump(label, data, len) log_hexdump((log_fn_t)log_uart_data, label, data, len)
#define _puts(s) log_puts((log_fn_t)log_uart_data, s)
#elif defined(LOG_ALL)
#define _printf(fmt, ...) do { \
	log_printf((log_fn_t)log_udp_data, fmt, ## __VA_ARGS__); \
	log_printf((log_fn_t)log_uart_data, fmt, ## __VA_ARGS__); \
} while(0)
#define _hexdump(label, data, len) do { \
	log_hexdump((log_fn_t)log_udp_data, label, data, len); \
	log_hexdump((log_fn_t)log_uart_data, label, data, len); \
} while(0)
#define _puts(s) do { \
	log_puts((log_fn_t)log_udp_data, s); \
	log_puts((log_fn_t)log_uart_data, s); \
} while(0)
#endif

#define debug(fmt, ...) _printf("[%s:%s:%d] " fmt, __FILE__, __func__, __LINE__, ## __VA_ARGS__)

#endif //_LOG_H



/*================================================================
 *   
 *   
 *   文件名称：log.h
 *   创 建 者：肖飞
 *   创建日期：2020年05月14日 星期四 14时09分56秒
 *   修改日期：2020年11月20日 星期五 11时33分38秒
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
#include "file_log.h"

#ifdef __cplusplus
}
#endif

#if !defined(LOG_NONE)
#if !defined(LOG_UDP) && !defined(LOG_UART) && !defined(LOG_FILE)
#define LOG_UART
#endif
#endif

#if defined(LOG_UDP)
#define udp_log_printf(fmt, ...) log_printf((log_fn_t)log_udp_data, fmt, ## __VA_ARGS__)
#define udp_log_hexdump(label, data, len) log_hexdump((log_fn_t)log_udp_data, label, data, len)
#define udp_log_puts(s) log_puts((log_fn_t)log_udp_data, s)
#else
#define udp_log_printf(fmt, ...)
#define udp_log_hexdump(label, data, len)
#define udp_log_puts(s)
#endif

#if defined(LOG_UART)
#define uart_log_printf(fmt, ...) log_printf((log_fn_t)log_uart_data, fmt, ## __VA_ARGS__)
#define uart_log_hexdump(label, data, len) log_hexdump((log_fn_t)log_uart_data, label, data, len)
#define uart_log_puts(s) log_puts((log_fn_t)log_uart_data, s)
#else
#define uart_log_printf(fmt, ...)
#define uart_log_hexdump(label, data, len)
#define uart_log_puts(s)
#endif

#if defined(LOG_FILE)
#define file_log_printf(fmt, ...) log_printf((log_fn_t)log_file_data, fmt, ## __VA_ARGS__)
#define file_log_hexdump(label, data, len) log_hexdump((log_fn_t)log_file_data, label, data, len)
#define file_log_puts(s) log_puts((log_fn_t)log_file_data, s)
#else
#define file_log_printf(fmt, ...)
#define file_log_hexdump(label, data, len)
#define file_log_puts(s)
#endif

#define _printf(fmt, ...) do { \
	udp_log_printf(fmt, ## __VA_ARGS__); \
	uart_log_printf(fmt, ## __VA_ARGS__); \
	file_log_printf(fmt, ## __VA_ARGS__); \
} while(0)
#define _hexdump(label, data, len) do { \
	udp_log_hexdump(label, data, len); \
	uart_log_hexdump(label, data, len); \
	file_log_hexdump(label, data, len); \
} while(0)
#define _puts(s) do { \
	udp_log_puts(s); \
	uart_log_puts(s); \
	file_log_puts(s); \
} while(0)

#define debug(fmt, ...) _printf("[%s:%s:%d] " fmt, __FILE__, __func__, __LINE__, ## __VA_ARGS__)

#endif //_LOG_H

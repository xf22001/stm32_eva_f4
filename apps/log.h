

/*================================================================
 *   
 *   
 *   文件名称：log.h
 *   创 建 者：肖飞
 *   创建日期：2020年05月14日 星期四 14时09分56秒
 *   修改日期：2020年11月04日 星期三 13时46分32秒
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
#define LOG_UDP
#define LOG_UART
#endif

#if defined(LOG_UDP)
#define LOG_UDP_DATA log_udp_data
#else
#define LOG_UDP_DATA NULL 
#endif

#if defined(LOG_UART)
#define LOG_UART_DATA log_uart_data
#else
#define LOG_UART_DATA NULL
#endif

#if defined(LOG_FILE)
#define LOG_FILE_DATA log_file_data
#else
#define LOG_FILE_DATA NULL
#endif

#if defined(LOG_NONE)
#define _printf(fmt, ...)
#define _hexdump(label, data, len)
#define _puts(s)
#else
#define _printf(fmt, ...) do { \
	log_printf((log_fn_t)LOG_UDP_DATA, fmt, ## __VA_ARGS__); \
	log_printf((log_fn_t)LOG_UART_DATA, fmt, ## __VA_ARGS__); \
	log_printf((log_fn_t)LOG_FILE_DATA, fmt, ## __VA_ARGS__); \
} while(0)
#define _hexdump(label, data, len) do { \
	log_hexdump((log_fn_t)LOG_UDP_DATA, label, data, len); \
	log_hexdump((log_fn_t)LOG_UART_DATA, label, data, len); \
	log_hexdump((log_fn_t)LOG_FILE_DATA, label, data, len); \
} while(0)
#define _puts(s) do { \
	log_puts((log_fn_t)LOG_UDP_DATA, s); \
	log_puts((log_fn_t)LOG_UART_DATA, s); \
	log_puts((log_fn_t)LOG_FILE_DATA, s); \
} while(0)
#endif

#define debug(fmt, ...) _printf("[%s:%s:%d] " fmt, __FILE__, __func__, __LINE__, ## __VA_ARGS__)

#endif //_LOG_H

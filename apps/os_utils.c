

/*================================================================
 *
 *
 *   文件名称：os_utils.c
 *   创 建 者：肖飞
 *   创建日期：2019年11月13日 星期三 11时13分17秒
 *   修改日期：2020年12月17日 星期四 14时35分38秒
 *   描    述：
 *
 *================================================================*/
#include "os_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cmsis_os.h"
#include "list_utils.h"
#include "main.h"
#include "app_platform.h"

#include "log.h"

typedef struct {
	size_t size;
	struct list_head list;
} mem_node_info_t;

typedef struct {
	uint8_t init;
	osMutexId os_utils_mutex;
	size_t size;
	size_t count;
	size_t max_size;
	struct list_head mem_info_list;
} mem_info_t;

#define LOG_BUFFER_SIZE (1024)

#if(configAPPLICATION_ALLOCATED_HEAP == 1)
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] CCMRAM;
#endif

static mem_info_t mem_info = {
	.init = 0,
	.os_utils_mutex = NULL,
};

static int init_mem_info(void)
{
	int ret = -1;
	osMutexDef(os_utils_mutex);
	osStatus os_status;

	if(mem_info.os_utils_mutex == NULL) {
		mem_info.os_utils_mutex = osMutexCreate(osMutex(os_utils_mutex));

		if(mem_info.os_utils_mutex == NULL) {
			return ret;
		}

		os_status = osMutexWait(mem_info.os_utils_mutex, osWaitForever);

		if(os_status != osOK) {
		}

		mem_info.size = 0;
		mem_info.count = 0;
		mem_info.max_size = 0;
		INIT_LIST_HEAD(&mem_info.mem_info_list);

		mem_info.init = 1;
		ret = 0;

		os_status = osMutexRelease(mem_info.os_utils_mutex);

		if(os_status != osOK) {
		}
	}

	return ret;
}

static void *xmalloc(size_t size)
{
	osStatus os_status;
	mem_node_info_t *mem_node_info;

	if(mem_info.init == 0) {
		if(init_mem_info() != 0) {
			return NULL;
		}
	}

	os_status = osMutexWait(mem_info.os_utils_mutex, osWaitForever);

	if(os_status != osOK) {
	}

	mem_node_info = (mem_node_info_t *)malloc(sizeof(mem_node_info_t) + size);

	if(mem_node_info != NULL) {
		mem_info.size += size;
		mem_info.count += 1;

		if(mem_info.size > mem_info.max_size) {
			mem_info.max_size = mem_info.size;
		}

		mem_node_info->size = size;
		list_add_tail(&mem_node_info->list, &mem_info.mem_info_list);
	}

	os_status = osMutexRelease(mem_info.os_utils_mutex);

	if(os_status != osOK) {
	}

	return (mem_node_info != NULL) ? (mem_node_info + 1) : NULL;
}

static void xfree(void *p)
{
	osStatus os_status;

	if(mem_info.init == 0) {
		if(init_mem_info() != 0) {
			return;
		}
	}

	os_status = osMutexWait(mem_info.os_utils_mutex, osWaitForever);

	if(os_status != osOK) {
	}

	if(p != NULL) {
		mem_node_info_t *mem_node_info = (mem_node_info_t *)p;

		mem_node_info--;

		mem_info.size -= mem_node_info->size;
		mem_info.count -= 1;

		list_del(&mem_node_info->list);
		free(mem_node_info);
	}

	os_status = osMutexRelease(mem_info.os_utils_mutex);

	if(os_status != osOK) {
	}
}

void get_mem_info(size_t *size, size_t *count, size_t *max_size)
{
	osStatus os_status;
	mem_node_info_t *mem_node_info;
	struct list_head *head;

	*size = 0;
	*count = 0;
	*max_size = 0;

	if(mem_info.init == 0) {
		if(init_mem_info() != 0) {
			return;
		}
	}

	os_status = osMutexWait(mem_info.os_utils_mutex, osWaitForever);

	if(os_status != osOK) {
	}

	*size = mem_info.size;
	*count = mem_info.count;
	*max_size = mem_info.max_size;

	head = &mem_info.mem_info_list;

	list_for_each_entry(mem_node_info, head, mem_node_info_t, list) {
	}

	os_status = osMutexRelease(mem_info.os_utils_mutex);

	if(os_status != osOK) {
	}
}

extern uint32_t _Min_Heap_Size;
uint32_t get_min_heap_size(void)
{
	return (uint32_t)&_Min_Heap_Size;
}

void *os_alloc(size_t size)
{
	void *p;

	p = xmalloc(size);

	return p;
}

void os_free(void *p)
{
	xfree(p);
}

int log_printf(log_fn_t log_fn, const char *fmt, ...)
{
	va_list ap;
	int ret = -1;
	char *log_buffer = (char *)os_alloc(LOG_BUFFER_SIZE);

	if(log_buffer == NULL) {
		return ret;
	}

	va_start(ap, fmt);
	ret = vsnprintf(log_buffer, LOG_BUFFER_SIZE, fmt, ap);
	va_end(ap);

	if(ret > LOG_BUFFER_SIZE) {
		ret = LOG_BUFFER_SIZE;
	}

	if(log_fn != NULL) {
		ret = log_fn(log_buffer, ret);
	}

	os_free(log_buffer);

	return ret;
}

static int32_t my_isprint(int32_t c)
{
	if(((uint8_t)c >= 0x20) && ((uint8_t)c <= 0x7e)) {
		return 0x4000;
	} else {
		return 0;
	}
}

#define BUFFER_LEN 80
void log_hexdump(log_fn_t log_fn, const char *label, const char *data, int len)
{
	int ret = 0;
	char *buffer = (char *)os_alloc(BUFFER_LEN);
	const char *start = data;
	const char *end = start + len;
	int c;
	int puts(const char *s);
	char *buffer_start = buffer;
	int i;
	long offset = 0;
	int bytes_per_line = 16;

	if(buffer == NULL) {
		return;
	}

	if(label != NULL) {
		ret = snprintf(buffer, BUFFER_LEN, "%s:\n", label);

		if(ret > BUFFER_LEN) {
			ret = BUFFER_LEN;
		}

		if(log_fn != NULL) {
			ret = log_fn(buffer, ret);
		}
	}

	while(start < end) {
		int left = BUFFER_LEN;
		long address = start - data;

		buffer_start = buffer;

		c = end - start;

		if(c > bytes_per_line) {
			c = bytes_per_line;
		}

		ret = snprintf(buffer_start, left, "%08lx", offset + address);
		buffer_start += ret;

		if(ret >= left) {
			left = 0;
			goto out;
		} else {
			left -= ret;
		}

		ret = snprintf(buffer_start, left, " ");
		buffer_start += ret;

		if(ret >= left) {
			left = 0;
			goto out;
		} else {
			left -= ret;
		}

		for(i = 0; i < c; i++) {
			if(i % 8 == 0) {
				ret = snprintf(buffer_start, left, " ");
				buffer_start += ret;

				if(ret >= left) {
					left = 0;
					goto out;
				} else {
					left -= ret;
				}
			}

			ret = snprintf(buffer_start, left, "%02x ", (unsigned char)start[i]);
			buffer_start += ret;

			if(ret >= left) {
				left = 0;
				goto out;
			} else {
				left -= ret;
			}
		}

		for(i = c; i < bytes_per_line; i++) {
			if(i % 8 == 0) {
				ret = snprintf(buffer_start, left, " ");
				buffer_start += ret;

				if(ret >= left) {
					left = 0;
					goto out;
				} else {
					left -= ret;
				}
			}

			ret = snprintf(buffer_start, left, "%2s ", " ");
			buffer_start += ret;

			if(ret >= left) {
				left = 0;
				goto out;
			} else {
				left -= ret;
			}
		}

		ret = snprintf(buffer_start, left, "|");
		buffer_start += ret;

		if(ret >= left) {
			left = 0;
			goto out;
		} else {
			left -= ret;
		}

		for(i = 0; i < c; i++) {
			ret = snprintf(buffer_start, left, "%c", my_isprint(start[i]) ? start[i] : '.');
			buffer_start += ret;

			if(ret >= left) {
				left = 0;
				goto out;
			} else {
				left -= ret;
			}
		}

		ret = snprintf(buffer_start, left, "|");
		buffer_start += ret;

		if(ret >= left) {
			left = 0;
			goto out;
		} else {
			left -= ret;
		}

		ret = snprintf(buffer_start, left, "\n");
		buffer_start += ret;

		if(ret >= left) {
			left = 0;
			goto out;
		} else {
			left -= ret;
		}

	out:

		if(log_fn != NULL) {
			ret = log_fn(buffer, BUFFER_LEN - left);
		}

		start += c;
	}

	os_free(buffer);
}

int log_puts(log_fn_t log_fn, const char *s)
{
	int ret = 0;
	ret = strlen(s);

	if(log_fn != NULL) {
		if(ret > (1024 - 1)) {
			log_hexdump(log_fn, NULL, s, ret);
		} else {
			ret = log_fn((void *)s, ret);
		}
	}

	return ret;
}

void app_panic(void)
{
	while(1);
}

unsigned char mem_is_set(char *values, size_t size, char value)
{
	unsigned char ret = 1;
	int i;

	for(i = 0; i < size; i++) {
		if(values[i] != value) {
			ret = 0;
			break;
		}
	}

	return ret;
}

unsigned int str_hash(const char *s)
{
	unsigned int hash = 0;
	const char *p = NULL;

	p = s;
	while(*p != 0) {
		hash = (31 * hash) + tolower(*p);
		p++;
	}

	return hash;
}

unsigned char calc_crc8(const void *data, size_t size)
{
	unsigned char crc = 0;
	unsigned char *p = (unsigned char *)data;

	while(size > 0) {
		crc += *p;

		p++;
		size--;
	}

	return crc;
}

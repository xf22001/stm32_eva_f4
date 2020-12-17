

/*================================================================
 *   
 *   
 *   文件名称：app.h
 *   创 建 者：肖飞
 *   创建日期：2019年10月11日 星期五 16时56分29秒
 *   修改日期：2020年12月17日 星期四 12时41分26秒
 *   描    述：
 *
 *================================================================*/
#ifndef _APP_H
#define _APP_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#ifdef __cplusplus
}
#endif
#define VER_MAJOR 0
#define VER_MINOR 0
#if defined(USER_APP)
#define VER_REV 1
#else
#define VER_REV 0
#endif
#define VER_BUILD 0

#pragma pack(push, 1)

typedef struct {
	char device_id[32];
	char host[256];
	char port[8];
	char path[256];
} mechine_info_t;

typedef struct {
	unsigned char available;
	mechine_info_t mechine;
} app_info_t;

typedef struct {
	uint32_t crc;
	uint16_t payload_size;
} eeprom_app_head_t;

typedef struct {
	eeprom_app_head_t head;
	mechine_info_t mechine;
} eeprom_app_info_t;

#pragma pack(pop)

app_info_t *get_app_info(void);
int app_save_config(void);
void app(void const *argument);
void idle(void const *argument);
#endif //_APP_H

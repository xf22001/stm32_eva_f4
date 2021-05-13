

/*================================================================
 *   
 *   
 *   文件名称：app.h
 *   创 建 者：肖飞
 *   创建日期：2019年10月11日 星期五 16时56分29秒
 *   修改日期：2021年05月08日 星期六 22时52分18秒
 *   描    述：
 *
 *================================================================*/
#ifndef _APP_H
#define _APP_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "app_platform.h"
#include "cmsis_os.h"

#ifdef __cplusplus
}
#endif
#define VER_MAJOR 0
#define VER_MINOR 0
#define VER_REV 0
#define VER_BUILD 0

#pragma pack(push, 1)

typedef struct {
	char device_id[32];
	char host[256];
	char port[8];
	char path[256];
	uint8_t upgrade_enable;
} mechine_info_t;

typedef struct {
	unsigned char available;
	mechine_info_t mechine;
} app_info_t;

#pragma pack(pop)

typedef enum {
	APP_EVENT_NONE = 0,
	APP_EVENT_USB,
} app_event_t;

app_info_t *get_app_info(void);
int app_save_config(void);
void app_init(void);
void send_app_event(app_event_t event);
void app(void const *argument);
void idle(void const *argument);
#endif //_APP_H

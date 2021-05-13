

/*================================================================
 *   
 *   
 *   文件名称：eeprom_layout.h
 *   创 建 者：肖飞
 *   创建日期：2021年03月30日 星期二 16时36分37秒
 *   修改日期：2021年05月13日 星期四 15时25分16秒
 *   描    述：
 *
 *================================================================*/
#ifndef _EEPROM_LAYOUT_H
#define _EEPROM_LAYOUT_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "app_platform.h"
#include "cmsis_os.h"

#include "eeprom_config.h"
#include "app.h"

#ifdef __cplusplus
}
#endif


#pragma pack(push, 1)


typedef struct {
	eeprom_config_item_head_t head;
	mechine_info_t mechine;
} eeprom_mechine_info_t;

typedef struct {
	eeprom_mechine_info_t mechine_info;
} eeprom_layout_t;

static inline eeprom_layout_t *get_eeprom_layout(void)
{
	return (eeprom_layout_t *)0;
}

#pragma pack(pop)

#endif //_EEPROM_LAYOUT_H



/*================================================================
 *
 *
 *   文件名称：usbh_user_callback.c
 *   创 建 者：肖飞
 *   创建日期：2021年03月25日 星期四 14时15分22秒
 *   修改日期：2021年05月08日 星期六 21时08分51秒
 *   描    述：
 *
 *================================================================*/
#include "usbh_user_callback.h"

#include "mt_file.h"
#include "file_log.h"
#include "vfs.h"
#include "app.h"

void usbh_user_callback(USBH_HandleTypeDef *phost, uint8_t id)
{
	switch(id) {
		case HOST_USER_CONNECTION: {
		}
		break;

		case HOST_USER_DISCONNECTION: {
			try_to_close_log();

			if(mt_f_mount(0, "", 0) != FR_OK) {
			}
		}
		break;

		case HOST_USER_CLASS_ACTIVE: {
			if(mt_f_mount(get_vfs_fs(), "", 0) == FR_OK) {
				send_app_event(APP_EVENT_USB);
			}
		}
		break;

		default:
			break;
	}
}

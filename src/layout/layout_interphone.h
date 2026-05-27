
#ifndef _LAYOUT_INTERPHONE_H_
#define _LAYOUT_INTERPHONE_H_

typedef enum
{
	INTERPHONE_STATUS_IDLE,//空闲
	/*已经呼出，等待应答*/
	INTERPHONE_STATUS_PUBLISH,
	INTERPHONE_STATUS_OUT,//呼出
	INTERPHONE_STATUS_IN,//被呼叫
	INTERPHONE_STATUS_TALK//接通谈话中
}interphone_status_enum;

extern interphone_status_enum interphone_status;
void layout_motion_init(void);

void layout_interphone_init(void);

void interphone_call_event_extern_func(unsigned long arg1,unsigned long arg2);
#endif


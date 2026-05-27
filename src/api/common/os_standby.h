
#include <stdbool.h>


bool standby_timer_open(int timeout,void(*timeout_callback)(void));

bool standby_timer_close(void);


bool standby_timer_check(void);

bool standby_timer_reset(void);




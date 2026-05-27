#include "stdio.h"
static int watch_timerout = 0;
int watch_dog_fd = -1;

int watch_dog_start(void)
{
    watch_dog_fd = 0;
    watch_timerout = 0;
    return 1;
}

int watch_dog_push(void)
{
    watch_timerout = 0;
    return 1;
}

int watch_dog_wait(void)
{
    if(watch_dog_fd < 0)
    {
        return 0;
    }
    return watch_timerout++;;
}


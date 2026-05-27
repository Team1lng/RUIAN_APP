#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include "stdbool.h"

#include "ak_thread.h"

typedef struct
{
	char* head;
	char* tail;

	char* r_addr;
	char* w_addr;

	int cache_len;
	int ring_len ;

	ak_mutex_t* mutex;
}ring_buffer;




bool ring_buffer_init(ring_buffer* ring,int size,ak_mutex_t* mutex);

bool ring_buffer_write(ring_buffer* ring,char* data,int size);

int ring_buffer_read(ring_buffer* ring,char*data ,int size);

bool ring_buffer_release(ring_buffer*ring);

#endif


#ifndef __H264_UVC_TESTAP_H_
#define __H264_UVC_TESTAP_H_

#include <inttypes.h>

struct buffer {
	void *         start;
	size_t         length;
};


extern int capturing;
extern FILE *rec_fp1;
void Init_264camera(void);
void * cap_video (void *arg);
int read_buf(void * opaque,uint8_t *buf, int buf_size);

#endif
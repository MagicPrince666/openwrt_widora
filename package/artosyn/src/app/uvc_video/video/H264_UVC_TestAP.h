#ifndef __H264_UVC_TESTAP_H_
#define __H264_UVC_TESTAP_H_

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
	void *         start;
	size_t         length;
};


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif 

void Init_264camera(const char *dev);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 
//void * cap_video (void *arg);

#endif
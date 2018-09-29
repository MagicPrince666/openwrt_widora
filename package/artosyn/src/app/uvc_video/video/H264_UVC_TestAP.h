#ifndef __H264_UVC_TESTAP_H_
#define __H264_UVC_TESTAP_H_

extern int capturing;
extern FILE *rec_fp1;
extern int Set_BitRate;
void Init_264camera(const char *dev);
void * cap_video (void *arg);

#endif
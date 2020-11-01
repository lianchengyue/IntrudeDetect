#ifndef CONVERT_H
#define CONVERT_H

void yuyv_to_rgb(unsigned char *yuv_buffer,unsigned char *rgb_buffer,int iWidth,int iHeight);
void NV21_YUV420P(const unsigned char* image_src, unsigned char* image_dst,int image_width, int image_height);
int yuyv_to_yuv420p(const unsigned char *in, unsigned char *out, unsigned int width, unsigned int height);
//YV12
void YUV420P_to_RGB24(unsigned char *data, unsigned char *rgb, int width, int height);
//NV211
void NV21_TO_RGB24(unsigned char *yuyv, unsigned char *rgb, int width, int height);
//
void I420_TO_RGB24(unsigned char *yuv420p, unsigned char *rgb24, int width, int height);
unsigned char clip_value(unsigned char x,unsigned char min_val,unsigned char  max_val);
bool RGB24_TO_YUV420(unsigned char *RgbBuf,int w,int h,unsigned char *yuvBuf);


#endif // CONVERT_H

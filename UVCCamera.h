#ifndef _UVCCAMERA_H
#define _UVCCAMERA_H
#include "H264Encoder.h"

class UVCCamera
{
public:
    UVCCamera();
    virtual ~UVCCamera();

    void init_mmap(void);
    void init_camera(void);
    void open_camera(char *dev_name);

    void close_camera(void);
    void read_one_frame(unsigned char *frame);
    int getnextframe(unsigned char *frame, long &frame_idx);
    void start_capture(void);
    void stop_capture(void);
    int  camera_able_read(void);
    void Init(char *dev_video);
    void intoloop();
    void Destory();
    int v4l2_ioctl_supported_framesize(int fd);

private:
    int fd;
    unsigned int n_buffer;
    Encoder en;
    BUFTYPE *usr_buf;
};
#endif  //_UVCCAMERA_H

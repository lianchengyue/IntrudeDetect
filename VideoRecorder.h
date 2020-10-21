#ifndef _VIDEORECORDER_H
#define _VIDEORECORDER_H

#include "H264Encoder.h"

class VideoRecorder
{
public:
    VideoRecorder();
    virtual ~VideoRecorder();
    //初始化 - 申请空间
    int InitRecorder();
    //准备录像 - 设置文件信息和编码器参数
    int PrepareRecord();
    //录像 - 编码一帧图像
    int DoRecord(unsigned char *in_frame);
    int StopRecord();
    int ReleaseRecorder();

private:
    int input_frame_len;
    int h264_frame_len;
    unsigned char *h264_buf;
    unsigned int n_buffer;
    Encoder en;
    class H264Encoder h264encoder;
};

#endif  //_VIDEORECORDER_H

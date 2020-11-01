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
    //准备录像 - optional 1:封面缩略图
    int CoverThumbnail(unsigned char *in_frame);
    //录像 - 编码一帧图像
    int DoRecord(unsigned char *in_frame, long current_encode_index);
    //录像 - optional 2:N帧之后保存为short video
    //int CutAndSave();
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

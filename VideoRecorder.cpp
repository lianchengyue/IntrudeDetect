#include <stdlib.h>
#include "VideoRecorder.h"
#include "Macro.h"

VideoRecorder::VideoRecorder()
{
    h264_buf = NULL;
}

VideoRecorder::~VideoRecorder()
{
    ReleaseRecorder();
}

//1:初始化 - 申请空间
int VideoRecorder::InitRecorder()
{
    input_frame_len = WIDTH * HEIGHT * 2;

    //alloc buf
    h264_buf = (unsigned char *) malloc(WIDTH * HEIGHT * 2);

    return 0;
}

//2:准备录像 - 设置文件信息和编码器参数
int VideoRecorder::PrepareRecord()
{
    //按时间更新文件名
    h264encoder.updateH264Info();

    //H264 Encoder
    h264encoder.init_encoder();  //move here
    return 0;
}

//2.1:准备录像 - 封面缩略图
int VideoRecorder::CoverThumbnail(unsigned char *in_frame)
{
    //按时间更新文件名
    char *name = h264encoder.getH264Info();
    //sprintf();

    return 0;
}

//3:录像 - 编码一帧图像
int VideoRecorder::DoRecord(unsigned char *in_frame)
{
    //逐帧送进编码器
    h264_frame_len = h264encoder.encode_frame(in_frame, input_frame_len, (unsigned char*)h264_buf/*, h264_frame_len*/);

    return 0;
}

//4:结束录像 - 关闭编码器
int VideoRecorder::StopRecord()
{
    h264encoder.close_encoder();
    return 0;
}

int VideoRecorder::ReleaseRecorder()
{
    //H264 Encoder
    //h264encoder.close_encoder();
    //h264_buf
    free(h264_buf);
    h264_buf = NULL;
    return 0;
}

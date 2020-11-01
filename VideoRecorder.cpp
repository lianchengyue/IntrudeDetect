#include <stdlib.h>
#include "VideoRecorder.h"
#include "Macro.h"
//dump jpeg
#include "tiny_jpeg.h"
#include "H264Encoder.h"
#include "Convert.h"
//dump jpeg end

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
    int width = WIDTH;
    int height = HEIGHT;
    int num_components = 3;
    char *name = h264encoder.getCurrentH264Info();
    char *thumbnail_name = new char[VIDEO_NAME_MAX];
    unsigned char *rgb24_buffer = new unsigned char[width*height*3];

    memset(rgb24_buffer, 0, width*height*3);
    memset(thumbnail_name, 0, VIDEO_NAME_MAX);

    sprintf(thumbnail_name, "%s.jpg", name);

    //YUYV to RGB24
    yuyv_to_rgb(in_frame, rgb24_buffer, width, height);

    //encode
    if ( !tje_encode_to_file(thumbnail_name, width, height, num_components, rgb24_buffer) ) {
        fprintf(stderr, "Could not write JPEG\n");
        return EXIT_FAILURE;
    }

    free(thumbnail_name);
    thumbnail_name = NULL;
    free(rgb24_buffer);
    rgb24_buffer = NULL;

    return 0;
}

//3:录像 - 编码一帧图像
int VideoRecorder::DoRecord(unsigned char *in_frame, long current_encode_index)
{
    //逐帧送进编码器
    h264_frame_len = h264encoder.encode_frame(in_frame, input_frame_len, (unsigned char*)h264_buf/*, h264_frame_len*/, current_encode_index);

    return 0;
}

//3.1:录像 - N帧之后保存为short video
//int VideoRecorder::CutAndSave()
//{
//    //保存为Short Video
//
//    //加入suffix
//
//    return 0;
//}

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

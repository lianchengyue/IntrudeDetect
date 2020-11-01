#ifndef _H264ENCODER_H
#define _H264ENCODER_H

#include <stdio.h>
#include <string.h>
#include "encoder_define.h"
#include "Utils.h"
#include "Macro.h"

class H264Encoder
{
public:
    H264Encoder();
    virtual ~H264Encoder();

    int updateH264Info();
    char* getCurrentH264Info();
    void init_encoder(void);
    void compress_begin(Encoder *en, int width, int height);
    int  compress_frame(Encoder *en, int type, unsigned char *in, int len, unsigned char *out);
    void compress_end(Encoder *en);
    void Destory();
    void close_encoder();
    size_t encode_frame(unsigned char* yuv_frame, size_t yuv_length, unsigned char* h264_buf/*, unsigned int* h264_length*/, long current_encode_idx);

    BUFTYPE *usr_buf;

    int lumaPlaneSize;
    int chromaPlaneSize;

private:
    Encoder en;
#ifdef DUMP_H264
    //char *date_buf;
    char* h264_prefix_name;
    char* h264_file_name;
    char* h264_limit_file_name;
    FILE *h264_fp;
    FILE *h264_limit_fp;
#endif
};

#endif // _H264ENCODER_H

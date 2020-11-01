#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ctime>

#include "H264Encoder.h"

H264Encoder::H264Encoder()
{
#ifdef DUMP_H264
    //date_buf
    h264_prefix_name = (char *) malloc(VIDEO_NAME_MAX);
    h264_file_name = (char *) malloc(VIDEO_NAME_MAX);
    h264_limit_file_name = (char *) malloc(VIDEO_NAME_MAX);
#endif

#ifdef YUYV
#elif defined NV21
    lumaPlaneSize = WIDTH * HEIGHT;
    chromaPlaneSize = WIDTH * HEIGHT >> 2;
#else
    lumaPlaneSize = WIDTH * HEIGHT;
    chromaPlaneSize = WIDTH * HEIGHT > 1;
#endif

    h264_fp = NULL;
}

H264Encoder::~H264Encoder()
{
    Destory();

    h264_fp = NULL;

    free(h264_prefix_name);
    h264_prefix_name = NULL;

    free(h264_file_name);
    h264_file_name = NULL;

    free(h264_limit_file_name);
    h264_limit_file_name = NULL;
}

//TQQ
void H264Encoder::init_encoder(void)
{
    compress_begin(&en, WIDTH, HEIGHT);
}


void H264Encoder::compress_begin(Encoder *en, int width, int height)
{
    en->param = (x264_param_t *) malloc(sizeof(x264_param_t));
    en->picture = (x264_picture_t *) malloc(sizeof(x264_picture_t));
    x264_param_default(en->param); //set default param

    //3种可变码率VBR
    //#define    X264_RC_CQP                  0--->恒定质量  //完全不需要预测所以它会运行得更快
    //#define    X264_RC_CRF                  1--->恒定码率
    //#define    X264_RC_ABR                  2--->平均码率

    //typedef struct
    //{
    //    int     i_csp;       //色彩空间参数 ，X264只支持I420
    //    int     i_stride[4]; //对应于各个色彩分量的跨度
    //    uint8_t *plane[4];   //对应于各个色彩分量的数据
    //} x264_image_t;

    //typedef struct
    //{
    //    int i_ref_idc;    //指该NAL单元的优先级
    //    int i_type;       //指该NAL单元的类型
    //    int b_long_startcode; // 是否采用长前缀码0x00000001
    //    int i_first_mb; /* If this NAL is a slice, the index of the first MB in the slice. */
    //    int i_last_mb;  /* If this NAL is a slice, the index of the last MB in the slice. */
    //    int i_payload;     //该nal单元包含的字节数
    //    uint8_t *p_payload;//该NAL单元存储数据的开始地
    //} x264_nal_t;

#if 1
    //zerolatency场景: i_lookahead=0, i_threads = 1
    en->param->i_frame_reference=3;  //表示P帧可以使用其之前的多少帧作为参考帧
    en->param->rc.i_rc_method=X264_RC_CRF;
    //en->param->rc.i_vbv_buffer_size = pCodecParam->encoder.iMaxBitrate; //码率控制缓冲区的大小，单位kbit，默认0
    en->param->b_cabac =0;
    en->param->b_interlaced=0;
    en->param->i_level_idc=30;
    en->param->i_keyint_max=en->param->i_fps_num*1.5;  //最大IDR帧间隔,建议设置为帧速率的10倍
    en->param->i_keyint_min=1;
    //i_threads的值 1:不使用并行编码    N:使用N个并行单元编码     0:或不设置:不配置并行单元数，由程序根据当前CPU性能决定N值
    en->param->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
    en->param->b_sliced_threads  = 1;
    //zerolatency场景下设置param.rc.i_lookahead=0; 那么编码器来一帧编码一帧，无并行、无延时。
    //如果没有设置i_lookahead=0，编码器会延时40帧（程序缺省值），再开始编码，这是为了做码率控制而统计帧信息
    en->param->rc.i_lookahead = 0;  //最大缓冲帧数
    // 指定处理线程，如果不为1，slice设置将失效
    //en->param->i_slice_count = 4;
    en->param->i_slice_count = 1;
    en->param->i_width = WIDTH;
    en->param->i_height = HEIGHT;
    en->param->i_frame_total = 0;
    //en->param->i_bframe = 10;
    en->param->b_open_gop = 0;
    en->param->i_bframe_pyramid = 0;  //是否允许将B帧当作参考帧
    //自适应B帧策略,该策略决定使用P帧还是B帧
    //0=X264_B_ADAPT_NONE（总是使用B帧）
    //1=X264_B_ADAPT_FAST（快速算法）
    //2=X264_B_ADAPT_TRELLIS（最佳算法）
    en->param->i_bframe_adaptive = X264_B_ADAPT_NONE;
//    en->param->rc.i_bitrate = 1024*10;//1024 * 10;  //平均码率 ,abr must
    en->param->rc.i_qp_min = 6;
    en->param->rc.i_qp_max = 51;
    en->param->i_fps_num = 5;  //10
    en->param->i_fps_den = 1;
#else
    int m_nFps = 5;
    int iMaxBitrate;
    //en->param->i_csp                = X264_CSP_I420;// 设置帧数据格式为420
    en->param->i_width			 = WIDTH;		 // 设置帧宽度
    en->param->i_height			 = HEIGHT;		 // 设置帧高度
    en->param->rc.i_lookahead		 = 0;

    en->param->i_fps_num			 = m_nFps;       // 设置帧率（分子）
    en->param->i_fps_den			 = 1;            // 设置帧率时间1s（分母）

    en->param->rc.i_bitrate	     = 1024*m_nFps;   // 设置码率
    en->param->rc.i_vbv_max_bitrate = 2048; // 设置平均码率模式下，最大瞬时码率
    en->param->i_keyint_max         = m_nFps * 2;   // 设置GOP最大长度
    en->param->i_keyint_min         = m_nFps * 2;   // 设置GOP最小长度

    //en->param->pf_log = x264_log;	                 // 设置打印日志回调
    en->param->p_log_private = NULL;
    en->param->i_log_level = X264_LOG_WARNING;
    en->param->b_vfr_input = 0;                     // 1:使用timebase和时间戳做码率控制 0：只使用fps做码率控制

    en->param->i_timebase_num = 1;                  // timebase（分子）
    en->param->i_timebase_den = 1000;               // timebase（分母）

    en->param->b_repeat_headers = 0;                // 1:在每个关键帧前面放sps和pps
    en->param->i_threads = 1;                       // 并行编码多个帧线程数
    en->param->rc.i_rc_method       = X264_RC_ABR;  // 码率控制方法，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
    en->param->rc.i_vbv_buffer_size = 0; //码率控制缓冲区的大小，单位kbit，默认0
#endif

#ifdef YUYV
    //设置帧数据格式为422
    en->param->i_csp = X264_CSP_I422;
    x264_param_apply_profile(en->param, x264_profile_names[4]);

    if ((en->handle = x264_encoder_open(en->param)) == 0) {
        return;
    }

    x264_picture_alloc(en->picture, X264_CSP_I422, en->param->i_width,
            en->param->i_height);
#elif defined NV21
    //设置帧数据格式为420
    en->param->i_csp = X264_CSP_I422;
    x264_param_apply_profile(en->param, x264_profile_names[4]);

    if ((en->handle = x264_encoder_open(en->param)) == 0) {
        return;
    }

    x264_picture_alloc(en->picture, X264_CSP_I422, en->param->i_width,
            en->param->i_height);
#else
    //设置帧数据格式为420
    en->param->i_csp = X264_CSP_I420;
    x264_param_apply_profile(en->param, x264_profile_names[4]);

    if ((en->handle = x264_encoder_open(en->param)) == 0) {
        return;
    }

    x264_picture_alloc(en->picture, X264_CSP_I420, en->param->i_width,
            en->param->i_height);
#endif
}

int H264Encoder::compress_frame(Encoder *en, int type, unsigned char *in, int len, unsigned char *out)
{
    x264_picture_t pic_out;
    int index_y, index_u, index_v;
    int num;
    int nNal = -1;
    int result = 0;
    int i = 0;
    unsigned char *p_out = out;
    char *y = (char*)en->picture->img.plane[0];
    char *u = (char*)en->picture->img.plane[1];
    char *v = (char*)en->picture->img.plane[2];

    index_y = 0;
    index_u = 0;
    index_v = 0;
#ifdef YUYV
    num = WIDTH * HEIGHT * 2 - 4  ;

    for(i=0; i<num; i=i+4)
    {
        *(y + (index_y++)) = *(in + i);
        *(u + (index_u++)) = *(in + i + 1);
        *(y + (index_y++)) = *(in + i + 2);
        *(v + (index_v++)) = *(in + i + 3);
     }
#elif defined NV21
    int srcstep = WIDTH;
    int h,w = 0;
    memcpy(y, in, lumaPlaneSize);

    // 取出 NV21 数据中交替存储的 VU 数据
    // V 在前 ( 偶数位置 ), U 在后 ( 奇数位置 ), 交替存储
    for(int i = 0; i < chromaPlaneSize; i++){
        // U 色相 / 色彩值数据, 存储在 lumaPlaneSize 后的奇数索引位置
        //*(u + i) = *(in + lumaPlaneSize + i * 2 + 1);
        // V 色彩饱和度数据, 存储在 lumaPlaneSize 后的偶数索引位置
        //*(v + i) = *(in + lumaPlaneSize + i * 2);

#if 0
        // U 色相 / 色彩值数据, 存储在 lumaPlaneSize 后的奇数索引位置
        *(u + i) = *(in + lumaPlaneSize + i * 2 + 0);
        *(u + i+1) = *(in + lumaPlaneSize + i * 2 + 0);
        // V 色彩饱和度数据, 存储在 lumaPlaneSize 后的偶数索引位置
        *(v + i) = *(in + lumaPlaneSize + i * 2 + 1);
        *(v + i+1) = *(in + lumaPlaneSize + i * 2 + 1);
#else
        h = i / srcstep;
        w = i % srcstep;
        // U 色相 / 色彩值数据, 存储在 lumaPlaneSize 后的奇数索引位置
        *(u + 2*h*srcstep + w) = *(in + lumaPlaneSize + i * 2 + 0);
        *(u + 2*h*srcstep + w + srcstep) = *(in + lumaPlaneSize + i * 2 + 0);
        // V 色彩饱和度数据, 存储在 lumaPlaneSize 后的偶数索引位置
        *(v + 2*h*srcstep + w) = *(in + lumaPlaneSize + i * 2 + 1);
        *(v + 2*h*srcstep + w + srcstep) = *(in + lumaPlaneSize + i * 2 + 1);
#endif
    }
#else
    //I420
    memcpy(y, in, lumaPlaneSize);
    memcpy(u, in + lumaPlaneSize, chromaPlaneSize);
    memcpy(v, in + lumaPlaneSize + chromaPlaneSize, chromaPlaneSize);
#endif

    switch (type) {
    case 0:
        en->picture->i_type = X264_TYPE_P;
        break;
    case 1:
        en->picture->i_type = X264_TYPE_IDR;
        break;
    case 2:
        en->picture->i_type = X264_TYPE_I;
        break;
    default:
        en->picture->i_type = X264_TYPE_AUTO;
        break;
    }

    en->picture->i_pts ++;

    //① x264_t * 参数 : x264 视频编码器
    //② x264_nal_t **pp_nal 参数 : 编码后的帧数据, 可能是 1 帧, 也可能是 3 帧
    //③ int *pi_nal 参数 : 编码后的帧数, 1 或 3
    //④ x264_picture_t *pic_in 参数 : 输入的 NV21 格式的图片数据
    //⑤ x264_picture_t *pic_out 参数 : 输出的图片数据
    if (x264_encoder_encode(en->handle, &(en->nal), &nNal, en->picture,
            &pic_out) < 0) {
        return -1;
    }

    for (i = 0; i < nNal; i++) {
        memcpy(p_out, en->nal[i].p_payload, en->nal[i].i_payload);
        p_out += en->nal[i].i_payload;
        result += en->nal[i].i_payload;
    }

    return result;
}

void H264Encoder::compress_end(Encoder *en)
{
    if (en->handle) {
        x264_encoder_close(en->handle);
    }
    if (en->picture) {
        x264_picture_clean(en->picture);
        free(en->picture);
        en->picture = 0;
    }
    if (en->param) {
        free(en->param);
        en->param = 0;
    }
}


void H264Encoder::Destory()
{
    //close_encoder();
}

void H264Encoder::close_encoder()
{
    compress_end(&en);
#ifdef DUMP_H264
    if(NULL != h264_fp)
    {
        fclose(h264_fp);
        h264_fp = NULL;
    }

    if(NULL != h264_limit_fp)
    {
        fclose(h264_limit_fp);
        h264_limit_fp = NULL;
    }
#endif
}

int H264Encoder::updateH264Info()
{
    time_t rawtime;  //8Byte
    struct tm *timeinfo;  //56Byte
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(h264_prefix_name, VIDEO_NAME_MAX*sizeof(char), "%Y-%m-%d_%H-%M-%S", timeinfo);
    sprintf(h264_file_name, "%s.h264", h264_prefix_name);
    //std::string str(h264_file_name);
    sprintf(h264_limit_file_name, "%s_limit.h264", h264_prefix_name);
    //std::string str2(h264_limit_file_name);

#ifdef DUMP_H264
    h264_fp = fopen(h264_file_name, "wa+");
    h264_limit_fp = fopen(h264_limit_file_name, "wa+");
#endif
}

char* H264Encoder::getCurrentH264Info()
{
    return h264_prefix_name;
}

//H264Processor
size_t H264Encoder::encode_frame(unsigned char* input_frame, size_t input_frame_length, unsigned char* h264_buf/*, unsigned int* h264_length*/, long current_encode_idx)
{
    int h264_length = 0;
    static int count = 0;

    h264_length = compress_frame(&en, -1, input_frame, input_frame_length, h264_buf);
    if ((h264_length > 0) && (NULL != h264_buf))
    {
        if(NULL == h264_buf)
        {
            printf("h264_buf = NULL!!\n");
        }
#ifdef DUMP_H264
        //前200帧率，双码流
        if(current_encode_idx < FRAME_CNT_LIMIT)
        {
            if((fwrite(h264_buf, h264_length, 1, h264_fp)>0) && (fwrite(h264_buf, h264_length, 1, h264_limit_fp)>0))
            {
                #ifdef LOG_H264_DEBUG
                printf("double stream encode_frame num = %d, size =%d\n",count++, h264_length);
                #endif
            }
            else
            {
                perror("double stream encode_frame fwrite err\n");
            }
        }
        //超过200帧率，单码流
        else
        {
            if(fwrite(h264_buf, h264_length, 1, h264_fp)>0)
            {
                #ifdef LOG_H264_DEBUG
                printf("single stream encode_frame num = %d, size =%d\n",count++, h264_length);
                #endif
            }
            else
            {
                perror("single stream encode_frame fwrite err\n");
            }
        }
#endif

    }

    return h264_length;
}

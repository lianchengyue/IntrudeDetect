#ifndef _IDP_H
#define _IDP_H

#include "UVCCamera.h"
#include "VideoRecorder.h"
#include "IntrudeDetector.h"
#include "SceneDetector.h"
#include "RingBuffer.h"

class IDP
{
public:
    IDP(char* args);
    virtual ~IDP();

    int RunIDP();
    int ExitIDP();

private:
    //1: scene场景检测
    int SceneDetectFunc();
    //2: foregound检测
    int ForeDetectFunc();
    //3: 上报信息
    int ReportFunc(int messageType);
    //4: 本地保存h264
    int RecordFunc(int recType);

    UVCCamera Camera;
    VideoRecorder *m_pRecorder;
    IntrudeDetector *m_pIntrudeDetector;
    SceneDetector *m_pSceneDetector;
    unsigned char *inFrame;
    long m_lFrameIdx;

    //前景信息(contour信息)
    Ringbuff *m_pContourBuff;
    float contour_total_size;
    float contour_average_size;
    float m_iLastContourNum;
    //一次positive前景判断后的dumb帧数
    int m_iIntrudeDumbCnt;

    //场景信息(brightness信息)
    Ringbuff *m_pBrightnessBuff;
    float brightness_median;
    float brightness_mean;
    float previous_brightness_mean;
    int m_iLastBrightness;
    //一次positive开关灯判断后的dumb帧数
    int m_iBrightnessDumbCnt;

    int buff0[INTRUDE_BUFFER_NUM];
    int buff1[SCENE_BUFFER_NUM];

    int readBuff0[INTRUDE_BUFFER_NUM];
    int readBuff1[SCENE_BUFFER_NUM];

    bool m_start_record_flag;  //开始录像
    bool m_first_record_frame_coming_flag;  //创建文件fd
    bool m_stop_record_flag;  //停止录像

    //optional 2:
    long m_lRecordIdx;
#ifdef OUTPUT_PRINT
    unsigned char *m_sOutputContent;  //输出打印
#endif
};

#endif // _IDP_H

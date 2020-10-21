#include <stdlib.h>

#include "IDP.h"
#include "Utils.h"
#include "VideoRecorder.h"

//构造与析构函数
IDP::IDP(char* args)
{
    //uvc camera
    Camera.Init(args);

    #ifdef RECORD_FUNC
    //h264 recorder
    m_pRecorder = new VideoRecorder();
    m_pRecorder->InitRecorder();  //初始化encoder
    #endif

    //IntruDetector
    m_pIntrudeDetector = new IntrudeDetector();
    //SceneDetector
    m_pSceneDetector = new SceneDetector();

    //DUMB时，跳过
    m_start_record_flag = false;
    m_first_record_frame_coming_flag = false;
    m_stop_record_flag = false;

    //帧信息
    inFrame = new unsigned char[WIDTH * HEIGHT * 2];
    memset(inFrame, 0, WIDTH * HEIGHT);
    m_lFrameIdx = 0;

    //前景信息
    m_pContourBuff = (Ringbuff *)malloc(/*INTRUDE_BUFFER_NUM * */sizeof(Ringbuff));
    m_iLastContourNum = 0;
    contour_total_size = 0;
    contour_average_size = 0;
    //m_iIntrudeDumbCnt = DUMB_FORE_CNT;

    //场景信息
    m_pBrightnessBuff = (Ringbuff *)malloc(/*SCENE_BUFFER_NUM * */sizeof(Ringbuff));
    brightness_median = 0;
    brightness_mean = 0;
    m_iLastBrightness = 0;
    m_iBrightnessDumbCnt = DUMB_SCENE_CNT;

    //初始化ring buffer
    memset(buff0, 0 ,sizeof(buff0));
    memset(buff1, 0 ,sizeof(buff1));

    rb_init (m_pContourBuff, buff0, INTRUDE_BUFFER_NUM);  //sizeof(buff0): 120
    rb_init (m_pBrightnessBuff, buff1, SCENE_BUFFER_NUM);  //sizeof(buff1)

#ifdef OUTPUT_PRINT
    m_sOutputContent = new unsigned char[12];
    memset(m_sOutputContent, 0 ,12);
#endif
}

IDP::~IDP()
{
    delete m_pIntrudeDetector;
    delete m_pRecorder;
    delete m_pSceneDetector;
    delete m_pContourBuff;
    delete m_pBrightnessBuff;

#ifdef OUTPUT_PRINT
    free(m_sOutputContent);
#endif
}

//1: scene场景检测
int IDP::SceneDetectFunc()
#if 0
{
    float accum  = 0.0;

    int value;

    value = round(m_pSceneDetector->GetAvg(inFrame));

    #if 1
    int r_res=0;
    //r_res = rb_read(m_pContourBuff, readBuff0, 4);
    //printf("buff0=%p\n", buff0);
    printf("buff1[0]=%d\n",buff1[0]);
    printf("buff1[1]=%d\n",buff1[1]);
    printf("buff1[2]=%d\n",buff1[2]);
    printf("buff1[3]=%d\n",buff1[3]);
    printf("buff1[4]=%d\n",buff1[4]);
    printf("buff1[5]=%d\n",buff1[5]);
    printf("buff1[6]=%d\n",buff1[6]);
    printf("buff1[7]=%d\n",buff1[7]);
    printf("buff1[8]=%d\n",buff1[8]);
    printf("buff1[9]=%d\n\n",buff1[9]);
    #endif

    rb_write(m_pBrightnessBuff, &value, 1/*4 or 16*/);

    if((0 != m_lFrameIdx) && (0 == m_lFrameIdx % SCENE_BUFFER_NUM))
    {
        //求均值
        for(int ii=0; ii<SCENE_BUFFER_NUM; ii++) {
            brightness_median += (float)buff1[ii];
        }
        brightness_median = brightness_median / SCENE_BUFFER_NUM;

        //求方差
        for(int jj=0; jj<SCENE_BUFFER_NUM; jj++)
        {
            accum += ((float)buff1[jj] - brightness_median) * ((float)buff1[jj] - brightness_median);
        }
        brightness_mean = sqrt(accum / SCENE_BUFFER_NUM);

#ifdef LOG_SCENE_DEBUG
        printf("brightness_median=%f, brightness_mean=%f\n", brightness_median, brightness_mean);
#endif

        if((brightness_mean < 0.8*previous_brightness_mean)
                ||(brightness_mean > 1.2*previous_brightness_mean)  )
        {
            //上报信息
            ReportFunc(SM_EVT_REINIT);
            //判断场景切换,(数帧之后),reset
            printf("\n场景切换!!!!!!!!!!!\n\n");
            m_pIntrudeDetector->resetReferentialFrame(inFrame);
        }

        //保存上次的计算结果
        previous_brightness_mean = brightness_mean;
    }

    return 0;
}
#else
{
    std::vector<int> valueVec;
    int value;

    if(DUMB_SCENE_CNT-1 == m_lFrameIdx)
    {
        m_iLastBrightness = round(m_pSceneDetector->GetAvg(inFrame));
    }

    if(m_iBrightnessDumbCnt != 0)
    {
        //printf("m_iBrightnessDumbCnt=%d\n",m_iBrightnessDumbCnt);
        m_iBrightnessDumbCnt--;
        return 10;
    }

    value = round(m_pSceneDetector->GetAvg(inFrame));
    //printf("亮度: brightness value=%d\n",value);

    //开灯
    if(/*(m_lFrameIdx > 30) && */(value > 1.35*m_iLastBrightness))  //1.8
    {
        m_iBrightnessDumbCnt = DUMB_SCENE_CNT;
        ReportFunc(SM_EVT_LIGHT_ON);
        printf("\开灯!!!!!!!!!!!\n\n");

    }
    //关灯
    else if (/*(m_lFrameIdx > 30) && */(value < 0.74*m_iLastBrightness))  //0.7
    {
        m_iBrightnessDumbCnt = DUMB_SCENE_CNT;
        ReportFunc(SM_EVT_LIGHT_OFF);
        printf("\关灯!!!!!!!!!!!\n\n");
    }
    else{
        //未达门限
    }

    m_iLastBrightness = value;

    return 0;
}
#endif

//2: foregound检测
int IDP::ForeDetectFunc()
{
    int value = m_pIntrudeDetector->procesUVCFrame(inFrame, m_lFrameIdx);
    rb_write(m_pContourBuff, &value, 1);

    #if 0
    printf("buff0[0~9]=%d, ",buff0[0]);
    printf("%d, ",buff0[1]);
    printf("%d, ",buff0[2]);
    printf("%d, ",buff0[3]);
    printf("%d, ",buff0[4]);
    printf("%d, ",buff0[5]);
    printf("%d, ",buff0[6]);
    printf("%d, ",buff0[7]);
    printf("%d, ",buff0[8]);
    printf("%d\n",buff0[9]);
    #endif

    //1: 判断门限(前30帧不做判断)
    if((0 != m_lFrameIdx) && (0 == m_lFrameIdx%INTRUDE_BUFFER_NUM))
    {
        //重置0
        contour_total_size = 0;
        contour_average_size = 0;

        for(int ii=0; ii<INTRUDE_BUFFER_NUM; ii++)  {
            contour_total_size += buff0[ii];
        }
        contour_average_size = contour_total_size / INTRUDE_BUFFER_NUM;;//round(contour_total_size / INTRUDE_BUFFER_NUM);
    }
    printf("contour_average_size=%f\n", contour_average_size);

    //2.1: 检测到前景
    if(contour_average_size > POSITIVE_CONTOURS_THRESHOLD)
    {
        //2.1.1: 之前没有前景, prepare 与 start
        if(m_iLastContourNum <= NEGATIVE_CONTOURS_THRESHOLD && !m_start_record_flag)
        {
            //上报信息
            ReportFunc(SM_EVT_INTRUDE);

            #ifdef RECORD_FUNC
            //初始化encoder
            RecordFunc(REC_PREPARE);

            //开始录像
            RecordFunc(REC_START);
            #endif
        }
        //2.1.2: 之前有前景, 继续录像
        else
        {
            #ifdef RECORD_FUNC
            //继续录像
            if(m_start_record_flag) {
                RecordFunc(REC_START);
            }
            #endif
        }
    }
    //2.2: 未检测到前景
    else if(contour_average_size <= NEGATIVE_CONTOURS_THRESHOLD)
    {
        //2.2.1: 前景消失
        if(m_iLastContourNum > POSITIVE_CONTOURS_THRESHOLD && m_start_record_flag)
        {
            //上报信息
            ReportFunc(SM_EVT_CLEAR);

            #ifdef RECORD_FUNC
            //结束录像
            RecordFunc(REC_STOP);
            #endif
        }
        //2.2.1: 之前也没有前景
        else
        {
            //by pass
        }
    }

    m_iLastContourNum = contour_average_size;

    return 0;
}

//3: 上报信息
int IDP::ReportFunc(int messageType)
{
#ifdef OUTPUT_PRINT
    time_t rawtime;  //8Byte
    time(&rawtime);
    //printf("time:%s", asctime(gmtime(&timep)));
    memcpy(m_sOutputContent+4, &rawtime, sizeof(time_t));

    switch(messageType) {
    //进入
    case SM_EVT_INTRUDE:
        m_sOutputContent[0] = 0x00;
        m_sOutputContent[1] = 0x00;
        m_sOutputContent[2] = 0x00;
        m_sOutputContent[3] = 0x01;
        //printf("\n");
        break;

        //离开
    case SM_EVT_CLEAR:
        m_sOutputContent[0] = 0x00;
        m_sOutputContent[1] = 0x00;
        m_sOutputContent[2] = 0x00;
        m_sOutputContent[3] = 0x81;
        //printf("\n");
        break;

    //环境改变
    case SM_EVT_REINIT:
        //printf("\n");
        break;

    //环境改变-开灯
    case SM_EVT_LIGHT_ON:
        m_sOutputContent[0] = 0x00;
        m_sOutputContent[1] = 0x00;
        m_sOutputContent[2] = 0x00;
        m_sOutputContent[3] = 0x02;
        //printf("\n");
        break;

    //环境改变-关灯
    case SM_EVT_LIGHT_OFF:
        m_sOutputContent[0] = 0x00;
        m_sOutputContent[1] = 0x00;
        m_sOutputContent[2] = 0x00;
        m_sOutputContent[3] = 0x82;
        //printf("\n");
        break;

    default:
        break;
    }

    printf("%s\n", m_sOutputContent);
#endif
    return 0;
}

//4: 本地保存h264
int IDP::RecordFunc(int recType)
{
    switch(recType) {
    case REC_PREPARE:        
        printf("\n\n开始录像\n");
        m_pRecorder->PrepareRecord();
        m_first_record_frame_coming_flag = true;
        //DUMB时，跳过
        m_start_record_flag = true;
        break;

    case REC_START:
        if(0 == m_lFrameIdx%100) {
            printf("正在录像, m_lFrameIdx=%d\n", m_lFrameIdx);
        }
        if(true == m_first_record_frame_coming_flag)
            m_pRecorder->DoRecord(inFrame);
        break;

    case REC_STOP:
        printf("结束录像\n");
        m_first_record_frame_coming_flag = false;
        //DUMB时，跳过
        m_start_record_flag = false;
        m_pRecorder->StopRecord();
        break;
    }

    return 0;
}

int IDP::RunIDP()
{
    int ret = 0;

    while(1)
    {
        if(0 == Camera.getnextframe(inFrame, m_lFrameIdx))   //m_lFrameIdx在UVCCamera种累加
        {
            //printf("Camera addr: %p\n", &Camera);
            //获取一帧YUYV图像
            //1: scene检测
            ret = SceneDetectFunc();

            //2: foregound检测
            ret = ForeDetectFunc();

            //3: 状态改变时，上报信息
            //ReportFunc();

            //4: 本地保存h264
            //int RecordFunc();
        }
    }

    return 0;
}

int IDP::ExitIDP()
{
    exit(-1);

    return 0;
}

#ifndef _UTILS_H
#define _UTILS_H

#define NO_ERROR   0
#define NO_DATA   -28
#define NO_MEMORY -29

#define DEV_VIDEO      "/dev/video0"

//#define WIDTH   1920
//#define HEIGHT  1080

//#define WIDTH   1280
//#define HEIGHT  720

//#define WIDTH   640
//#define HEIGHT  480

#define WIDTH   320
#define HEIGHT  240

#define VIDEO_NAME_MAX  256

//短视频帧数 - 30秒
#define FRAME_CNT_LIMIT  700  //24fps

#define POSITIVE_CONTOURS_THRESHOLD    4//4  //3
#define NEGATIVE_CONTOURS_THRESHOLD    2//2  //3
#define BRIGHTNESS_THRESHOLD  200

#define INTRUDE_BUFFER_NUM  30 //30
#define SCENE_BUFFER_NUM    10  //10

#define FRAME_SKIP_NUM  10

//每次Fore检测之后，跳过的帧数
//#define DUMB_FORE_COUNT  30
//每次Scene检测之后，跳过的帧数
#define DUMB_SCENE_CNT  30

typedef enum{
    SM_EVT_INIT = 1,  //开机初始化
    SM_EVT_INTRUDE,  //入侵
    SM_EVT_CLEAR,  //离开
    SM_EVT_REINIT,  //场景变化，重置背景
    SM_EVT_LIGHT_ON,  //开灯
    SM_EVT_LIGHT_OFF,  //关灯
} sm_evt_enum_t;

typedef enum{
    SM_STATE_INITING,
    SM_STATE_IDLE,
    SM_STATE_WORKING,
    SM_STATE_HALT,
} sm_state_enum_t;


typedef enum{
    REC_PREPARE,
    REC_START,
    REC_STOP,
} record_enum_t;
#endif // _UTILS_H

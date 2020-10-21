#include <stdio.h>

#include "SceneDetector.h"
#include "Utils.h"
#include "Macro.h"

SceneDetector::SceneDetector()
{
}

SceneDetector::~SceneDetector()
{
}

double SceneDetector::GetAvg(unsigned char *in_frame)
{
    input_mat = cv::Mat(HEIGHT, WIDTH, CV_8UC2, in_frame); //CV_8UC3
    cv::cvtColor(input_mat, input_mat, CV_YUV2BGR_YUYV);

    cv::cvtColor(input_mat, input_mat, CV_BGR2GRAY);
    cv::Scalar scalar = cv::mean(input_mat);
#ifdef LOG_SCENE_DEBUG
    printf("the average brightness(Gray) is %f\n", scalar.val[0]);
#endif

    return scalar.val[0];
}

double SceneDetector::GetAvg(cv::Mat img)
{
    cv::Mat gray;
    cv::cvtColor(img, gray, CV_BGR2GRAY);
    cv::Scalar scalar = cv::mean(gray);
#ifdef LOG_SCENE_DEBUG
    printf("Average Brightness(Gray) is %lf\n", scalar.val[0]);
#endif
    return scalar.val[0];
}

void SceneDetector::SetAvg(cv::Mat scr, cv::Mat dst, double avg)
{
   double fpreAvg = GetAvg(scr);
   scr.convertTo(dst,scr.type(),avg/fpreAvg);
}

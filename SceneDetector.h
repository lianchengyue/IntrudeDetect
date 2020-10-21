#ifndef _SCENEDETECT_H
#define _SCENEDETECT_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

class SceneDetector
{
public:
    SceneDetector();
    virtual ~SceneDetector();

    double GetAvg(unsigned char *in_frame);
    double GetAvg(cv::Mat img);
    void SetAvg(cv::Mat scr, cv::Mat dst, double avg);

    int CalculateFluctuation();
private:
    cv::Mat input_mat;
    //mean值保存在循环buffer中
};

#endif // _SCENEDETECT_H

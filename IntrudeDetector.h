#ifndef _INTRUDEDETECTOR_H
#define _INTRUDEDETECTOR_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "ForegroundDetect.h"
#include "Utils.h"

class IntrudeDetector
{
public:
    IntrudeDetector();
    virtual ~IntrudeDetector();

    int DoForeDetect();
    int procesUVCFrame(unsigned char *in_frame, long frame_idx);
    int IntrudeJudge(cv::Mat& input_img);
    int resetReferentialFrame(unsigned char *in_frame);

private:
    void processVideo(char* videoFilename);
    void* VaddrToMat(cv::Mat *des_mat, unsigned char *src_vaddr);

    /* Model for ViBe. */
    vibeModel_Sequential_t *model;
    cv::Mat input_mat;
    //long frameIdx;
    /* Will contain the segmentation map. This is the binary output map. */
    cv::Mat segmentationMap;

};

#endif // _INTRUDEDETECTOR_H

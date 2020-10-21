//#include <vector>
#include "IntrudeDetector.h"
#include "Macro.h"

//按轮廓周长(轮廓点数量)排序
static inline bool ContoursSortFun(std::vector<cv::Point> contour1, std::vector<cv::Point> contour2)
{
    return (contour1.size() > contour2.size());
}

//按轮廓面积排序
//static inline bool ContoursSortFun(std::vector<cv::Point> contour1, std::vector<cv::Point> contour2)
//{
//    return (cv::contourArea(contour1) > cv::contourArea(contour2));
//}

IntrudeDetector::IntrudeDetector()
{
    /* Model */
    model = NULL;

    //frameIdx = 0;

    segmentationMap = cv::Mat(HEIGHT, WIDTH, CV_8UC1);
}

IntrudeDetector::~IntrudeDetector()
{
    /* Frees the model. */
    libvibeModel_Sequential_Free(model);
}


//use the grayscale version (C1R) and the RGB version (C3R).
int IntrudeDetector::DoForeDetect()
{
#ifdef OPENCV_DISPLAY
    /* Create GUI windows. */
    cv::namedWindow("Frame");
    cv::namedWindow("Segmentation");
#endif

    processVideo("driveway-320x240.avi");

#ifdef OPENCV_DISPLAY
    /* Destroy GUI windows. */
    cv::destroyAllWindows();
#endif
    return EXIT_SUCCESS;
}

//读文件
void IntrudeDetector::processVideo(char* videoFilename)
{
  cv::VideoCapture capture(videoFilename);

  if (!capture.isOpened()) {
    printf("Unable to open video file: %s\n", videoFilename);
    //exit(EXIT_FAILURE);
  }

  /* Variables. */
  static int frameNumber = 1; /* The current frame number */
  cv::Mat frame;                  /* Current frame. */
  cv::Mat segmentationMap;        /* Will contain the segmentation map. This is the binary output map. */
  int keyboard = 0;           /* Input from keyboard. Used to stop the program. Enter 'q' to quit. */

  /* Model */
  vibeModel_Sequential_t *model = NULL;

  /* Read input data. ESC or 'q' for quitting. */
  while ((char)keyboard != 'q' && (char)keyboard != 27) {
    /* Read the current frame. */
    if (!capture.read(frame)) {
      printf("Unable to read next frame\n");
      //exit(EXIT_FAILURE);
    }

    if ((frameNumber % 100) == 0)
    {
        printf("Frame number = %d\n", frameNumber);
    }

    /*If you want to use the grayscale version of ViBe (which is much faster!):
     * (1) remplace C3R by C1R in this file.
     * (2) uncomment the next line (cvtColor).
     */
    /* cvtColor(frame, frame, CV_BGR2GRAY); */

    if (frameNumber == 1) {
      segmentationMap = cv::Mat(frame.rows, frame.cols, CV_8UC1);
      model = (vibeModel_Sequential_t*)libvibeModel_Sequential_New();
      libvibeModel_Sequential_AllocInit_8u_C3R(model, frame.data, frame.cols, frame.rows);
    }

    /* Segmentation and updating. */
    libvibeModel_Sequential_Segmentation_8u_C3R(model, frame.data, segmentationMap.data);
    libvibeModel_Sequential_Update_8u_C3R(model, frame.data, segmentationMap.data);

    /* Post-processes the segmentation map. This step is not compulsory.
       Note that we strongly recommend to use post-processing filters, as they
       always smooth the segmentation map. For example, the post-processing filter
       used for the Change Detection dataset (see http://www.changedetection.net/ )
       is a 5x5 median filter. */
    medianBlur(segmentationMap, segmentationMap, 3); /* 3x3 median filtering */

    /* Shows the current frame and the segmentation map. */
#ifdef OPENCV_DISPLAY
    imshow("Frame", frame);
    imshow("Segmentation", segmentationMap);
    /* Gets the input from the keyboard. */
    keyboard = cv::waitKey(1);
#endif

    frameNumber++;
  }

  /* Delete capture object. */
  capture.release();

  /* Frees the model. */
  libvibeModel_Sequential_Free(model);
}

//重置参考帧
int IntrudeDetector::resetReferentialFrame(unsigned char *in_frame)
{
    input_mat = cv::Mat(HEIGHT, WIDTH, CV_8UC2, in_frame); //CV_8UC3
    cv::cvtColor(input_mat, input_mat, CV_YUV2BGR_YUYV);

    model = (vibeModel_Sequential_t*)libvibeModel_Sequential_New();
    libvibeModel_Sequential_AllocInit_8u_C3R(model, input_mat.data, input_mat.cols, input_mat.rows);

    return 0;
}

int IntrudeDetector::procesUVCFrame(unsigned char *in_frame, long frame_idx)
{
    //VaddrToMat(input_mat, in_frame);
#if 0
    //input_mat = cv::Mat(HEIGHT, WIDTH, CV_8U, in_frame); //CV_8UC3
    //cv::imshow("Frame", input_mat);
    //cv::waitKey(1);

    input_mat = cv::Mat(HEIGHT, WIDTH, CV_8UC2, in_frame); //CV_8UC3
    cv::cvtColor(input_mat, input_mat, CV_YUV2BGR_YUYV);
    cv::imshow("Frame", input_mat);
    cv::waitKey(1);

    //cv::Mat yuvImg;
    //yuvImg.create(HEIGHT , WIDTH, CV_8UC2);
    //cv::Mat rgbImg(HEIGHT, WIDTH, CV_8UC3);
    //cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_I420);

    input_mat = cv::Mat(HEIGHT, WIDTH, CV_8UC2, in_frame); //CV_8UC3

    cv::Mat yuvImg;
    yuvImg.create(HEIGHT , WIDTH, CV_8UC2);
    cv::Mat rgbImg(HEIGHT, WIDTH, CV_8UC3);

    cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_I420);
#elif 1

    //FIX: AE不稳定问题
    if(frame_idx < FRAME_SKIP_NUM) {
        return 0;
    }

    input_mat = cv::Mat(HEIGHT, WIDTH, CV_8UC2, in_frame); //CV_8UC3
    cv::cvtColor(input_mat, input_mat, CV_YUV2BGR_YUYV);

    int keyboard = 0;           /* Input from keyboard. Used to stop the program. Enter 'q' to quit. */

    //if ((frame_idx % 100) == 0)
    //{
    //    printf("frame_idx: %d\n", frame_idx);
    //}

    /* If you want to use the grayscale version (which is much faster!):
     * (1) remplace C3R by C1R in this file.
     * (2) uncomment the next line (cvtColor).
     */
#ifdef GRAYSCALE_FORE_DETECT
    cvtColor(input_mat, input_mat, CV_BGR2GRAY);
#endif

    //init
    if (FRAME_SKIP_NUM == frame_idx) {
        model = (vibeModel_Sequential_t*)libvibeModel_Sequential_New();
#ifdef GRAYSCALE_FORE_DETECT
        libvibeModel_Sequential_AllocInit_8u_C1R(model, input_mat.data, input_mat.cols, input_mat.rows);
#else
        libvibeModel_Sequential_AllocInit_8u_C3R(model, input_mat.data, input_mat.cols, input_mat.rows);
#endif
    }

    /* ViBe: Segmentation and updating. */
#ifdef GRAYSCALE_FORE_DETECT
    libvibeModel_Sequential_Segmentation_8u_C1R(model, input_mat.data, segmentationMap.data);
    libvibeModel_Sequential_Update_8u_C1R(model, input_mat.data, segmentationMap.data);
#else
    libvibeModel_Sequential_Segmentation_8u_C3R(model, input_mat.data, segmentationMap.data);
    libvibeModel_Sequential_Update_8u_C3R(model, input_mat.data, segmentationMap.data);
#endif

    /* Post-processes the segmentation map. This step is not compulsory.
       Note that we strongly recommend to use post-processing filters, as they
       always smooth the segmentation map. For example, the post-processing filter
       used for the Change Detection dataset (see http://www.changedetection.net/ )
       is a 5x5 median filter. */
    medianBlur(segmentationMap, segmentationMap, 3); /* 3x3 median filtering */

#ifdef OPENCV_DISPLAY
    /* Shows the current frame and the segmentation map. */
    cv::imshow("Frame", input_mat);
    cv::imshow("Segmentation", segmentationMap);
    /* Gets the input from the keyboard. */
    keyboard = cv::waitKey(1);
#endif

#endif

    //判断是否有入侵
    return IntrudeJudge(segmentationMap);

}


void* IntrudeDetector::VaddrToMat(cv::Mat *des_mat, unsigned char *src_vaddr)
{
    //added for Visualization
    int width = WIDTH;
    int height = HEIGHT;

    //Mat赋值
    *des_mat = cv::Mat(height, width, CV_8U, src_vaddr); //CV_8UC3
    //转为3通道，再转回灰度图
    //cvtColor(*LeftPicMat,*LeftPicMat,CV_GRAY2BGR);
    //cvtColor(*LeftPicMat,*LeftPicMat,CV_BGR2GRAY);
}

int IntrudeDetector::IntrudeJudge(cv::Mat& input_img)
#if 1
{
    //cv::imshow("input_img", input_img);
    //erode(input_img, input_img, cv::Mat());
    //dilate(input_img, input_img, cv::Mat());
    //做一下膨胀，x与y方向都做，但系数不同
    //使用了Erode方法，腐蚀操作，针对白色区域，所以等效于对文字进行了膨胀
    cv::Mat erodeStruct = getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
    cv::erode(input_img, input_img, erodeStruct);

#ifdef OPENCV_DISPLAY
    cv::imshow("After erode&dilate", input_img);
#endif

    //检测连通域，每一个连通域以一系列的点表示，FindContours方法只能得到第一个域
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    //只检测最外层轮廓，并且保存轮廓上所有点
    findContours(input_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    //findContours(input_img, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    std::vector<cv::Rect> boundRect(contours.size());
    cv::Scalar color = cv::Scalar(0, 0, 255);

#ifdef LOG_INTRUDE_DEBUG
    printf("contours.size()=%d\n", contours.size());
#endif
    //开始遍历
    for (int i = 0; i < contours.size(); i++)
    {
        boundRect[i] = boundingRect(cv::Mat(contours[i]));
    }

    //按大小排序
    //std::sort(contours.begin(), contours.end(), ContoursSortFun);
    //再计算最大轮廓(contours中第一个元素contours[0]即是最大轮廓.)
    #ifdef LOG_INTRUDE_DEBUG
    if(contours.size() > 1) {
      printf("最大轮廓面积: %d\n", /*cv::contourArea(contours[0])*/contours.size());
    }
    //绘制最大轮廓
    #endif

    cv::Mat drawing = cv::Mat::zeros(input_img.size(), CV_8UC3);
    for (int i = 0; i< contours.size(); i++)
    {
        //轮廓
        drawContours(drawing, contours, i, color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
        rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
    }

#ifdef OPENCV_DISPLAY
    //显示
    cv::imshow("Result", drawing);
    cv::waitKey(1);
#endif

    return contours.size();
}
#else
{
   cv::Mat image;
   cv::GaussianBlur(input_img, image, cv::Size(3,3), 0);
   Canny(image, image, 100, 250);
   std::vector<std::vector<cv::Point>> contours;
   std::vector<cv::Vec4i> hierarchy;
   //findContours(image, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point());
   findContours(image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());
   cv::Mat imageContours = cv::Mat::zeros(image.size(),CV_8UC1);
   cv::Mat Contours = cv::Mat::zeros(image.size(),CV_8UC1);  //绘制

   for(int i=0;i<contours.size();i++)
   {
       //contours[i]代表的是第i个轮廓，contours[i].size()代表的是第i个轮廓上所有的像素点数
       for(int j=0;j<contours[i].size();j++)
       {
           //绘制出contours向量内所有的像素点
           cv::Point P = cv::Point(contours[i][j].x,contours[i][j].y);
           Contours.at<uchar>(P)=255;
       }

       //输出hierarchy向量内容
       char ch[256];
       sprintf(ch,"%d",i);
       std::string str = ch;
       std::cout << "向量hierarchy的第" << str << " 个元素内容为：" << std::endl << hierarchy[i] << std::endl << std::endl;

       //绘制轮廓
       drawContours(imageContours, contours, i, cv::Scalar(255), 1, 8, hierarchy);
   }
   cv::imshow("Contours Image",imageContours); //轮廓
   cv::imshow("Point of Contours",Contours);   //向量contours内保存的所有轮廓点集
   cv::waitKey(1);
   return 0;
}
#endif

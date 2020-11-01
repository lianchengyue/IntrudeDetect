#include "Convert.h"

#include <stdint.h>
#include <string.h>

/*
函数功能: 将YUV数据转为RGB格式
函数参数:
unsigned char *yuv_buffer: YUV源数据
unsigned char *rgb_buffer: 转换之后的RGB数据
int iWidth,int iHeight   : 图像的宽度和高度
*/
void yuyv_to_rgb(unsigned char *yuv_buffer,unsigned char *rgb_buffer,int iWidth,int iHeight)
{
    int x;
    int z=0;
    unsigned char *ptr = rgb_buffer;
    unsigned char *yuyv= yuv_buffer;
    for (x = 0; x < iWidth*iHeight; x++)
    {
        int r, g, b;
        int y, u, v;

        if (!z)
        y = yuyv[0] << 8;
        else
        y = yuyv[2] << 8;
        u = yuyv[1] - 128;
        v = yuyv[3] - 128;

        r = (y + (359 * v)) >> 8;
        g = (y - (88 * u) - (183 * v)) >> 8;
        b = (y + (454 * u)) >> 8;

        *(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
        *(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
        *(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

        if(z++)
        {
            z = 0;
            yuyv += 4;
        }
    }
}



// image_src是源图像，image_dst是转换后的图像
void NV21_YUV420P(const unsigned char* image_src, unsigned char* image_dst,int image_width, int image_height)
{
    unsigned char* p = image_dst;
    memcpy(p, image_src, image_width * image_height * 3 / 2);
    const unsigned char *pNV = image_src + image_width * image_height;
    unsigned char *pU = p + image_width * image_height;
    unsigned char *pV = p + image_width * image_height + ((image_width * image_height)>>2);
    for (int i=0; i<(image_width * image_height)/2; i++)
    {
        if((i%2)==0)
           *pV++=*(pNV+i);
        else
           *pU++=*(pNV+i);
    }
}


//YUYV==YV12
int yuyv_to_yuv420p(const unsigned char *in, unsigned char *out, unsigned int width, unsigned int height)
{
    unsigned char *y = out;
    unsigned char *u = out + width*height;
    unsigned char *v = out + width*height + width*height/4;

    unsigned int i,j;
    unsigned int base_h;
    unsigned int  is_u = 1;
    unsigned int y_index = 0, u_index = 0, v_index = 0;

    unsigned long yuv422_length = 2 * width * height;

    //序列为YU YV YU YV，一个yuv422帧的长度 width * height * 2 个字节
    //丢弃偶数行 u v
    for(i=0; i<yuv422_length; i+=2)
    {

        *(y+y_index) = *(in+i);

        y_index++;

    }

    for(i=0; i<height; i+=2)
    {

        base_h = i*width*2;

        for(j=base_h+1; j<base_h+width*2; j+=2)

        {

            if(is_u)
            {

            *(u+u_index) = *(in+j);

            u_index++;

            is_u = 0;

            }

            else
            {
                *(v+v_index) = *(in+j);
                v_index++;
                is_u = 1;
            }
        }
    }
    return 1;
}


void YUV420P_to_RGB24(unsigned char *data, unsigned char *rgb, int width, int height)
{
    int index = 0;
    unsigned char *ybase = data;
    unsigned char *ubase = &data[width * height];
    unsigned char *vbase = &data[width * height * 5 / 4];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            //YYYYYYYYUUVV
            unsigned char Y = ybase[x + y * width];
            unsigned char U = ubase[y / 2 * width / 2 + (x / 2)];
            unsigned char V = vbase[y / 2 * width / 2 + (x / 2)];
            rgb[index++] = Y + 1.402 * (V - 128); //R
            rgb[index++] = Y - 0.34413 * (U - 128) - 0.71414 * (V - 128); //G
            rgb[index++] = Y + 1.772 * (U - 128); //B
        }
    }
}


/**
 * NV21是android相机默认格式
 * @param data
 * @param rgb
 * @param width
 * @param height
 */
void NV21_TO_RGB24(unsigned char *yuyv, unsigned char *rgb, int width, int height)
{
        const int nv_start = width * height ;
        int  index = 0, rgb_index = 0;
        uint8_t y, u, v;
        int r, g, b, nv_index = 0,i, j;

        for(i = 0; i < height; i++){
            for(j = 0; j < width; j ++){
                //nv_index = (rgb_index / 2 - width / 2 * ((i + 1) / 2)) * 2;
                nv_index = i / 2  * width + j - j % 2;

                y = yuyv[rgb_index];
                u = yuyv[nv_start + nv_index ];
                v = yuyv[nv_start + nv_index + 1];

                r = y + (140 * (v-128))/100;  //r
                g = y - (34 * (u-128))/100 - (71 * (v-128))/100; //g
                b = y + (177 * (u-128))/100; //b

                if(r > 255)   r = 255;
                if(g > 255)   g = 255;
                if(b > 255)   b = 255;
                if(r < 0)     r = 0;
                if(g < 0)     g = 0;
                if(b < 0)     b = 0;

                index = rgb_index % width + (height - i - 1) * width;
                //rgb[index * 3+0] = b;
                //rgb[index * 3+1] = g;
                //rgb[index * 3+2] = r;

                //颠倒图像
                //rgb[height * width * 3 - i * width * 3 - 3 * j - 1] = b;
                //rgb[height * width * 3 - i * width * 3 - 3 * j - 2] = g;
                //rgb[height * width * 3 - i * width * 3 - 3 * j - 3] = r;

                //正面图像
                rgb[i * width * 3 + 3 * j + 0] = b;
                rgb[i * width * 3 + 3 * j + 1] = g;
                rgb[i * width * 3 + 3 * j + 2] = r;

                rgb_index++;
            }
        }
}

void I420_TO_RGB24(unsigned char *yuv420p, unsigned char *rgb24, int width, int height) {
#if 0
    int index = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int indexY = y * width + x;
            int indexU = width * height + y / 2 * width / 2 + x / 2;
            int indexV = width * height + width * height / 4 + y / 2 * width / 2 + x / 2;

            unsigned char Y = yuv420p[indexY];
            unsigned char U = yuv420p[indexU];
            unsigned char V = yuv420p[indexV];

            rgb24[index++] = Y + 1.402 * (V - 128); //R
            rgb24[index++] = Y - 0.34413 * (U - 128) - 0.71414 * (V - 128); //G
            rgb24[index++] = Y + 1.772 * (U - 128); //B
        }
    }
#else
    int index = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int indexY = y * width + x;
            int indexV = width * height + y / 2 * width / 2 + x / 2;
            int indexU = width * height + width * height / 4 + y / 2 * width / 2 + x / 2;

            unsigned char Y = yuv420p[indexY];
            unsigned char U = yuv420p[indexU];
            unsigned char V = yuv420p[indexV];

            rgb24[index++] = Y + 1.402 * (V - 128); //R
            rgb24[index++] = Y - 0.34413 * (U - 128) - 0.71414 * (V - 128); //G
            rgb24[index++] = Y + 1.772 * (U - 128); //B
        }
    }
#endif
}


unsigned char clip_value(unsigned char x,unsigned char min_val,unsigned char  max_val){
    if(x>max_val){
        return max_val;
    }else if(x<min_val){
        return min_val;
    }else{
        return x;
    }
}

//RGB to YUV420
bool RGB24_TO_YUV420(unsigned char *RgbBuf,int w,int h,unsigned char *yuvBuf)
{
    unsigned char*ptrY, *ptrU, *ptrV, *ptrRGB;
    memset(yuvBuf,0,w*h*3/2);
    ptrY = yuvBuf;
    ptrU = yuvBuf + w*h;
    ptrV = ptrU + (w*h*1/4);
    unsigned char y, u, v, r, g, b;
    for (int j = 0; j<h;j++){
        ptrRGB = RgbBuf + w*j*3 ;
        for (int i = 0;i<w;i++){

            r = *(ptrRGB++);
            g = *(ptrRGB++);
            b = *(ptrRGB++);
            y = (unsigned char)( ( 66 * r + 129 * g +  25 * b + 128) >> 8) + 16  ;
            u = (unsigned char)( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128 ;
            v = (unsigned char)( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128 ;
            *(ptrY++) = clip_value(y,0,255);
            if (j%2==0&&i%2 ==0){
                *(ptrU++) =clip_value(u,0,255);
            }
            else{
                if (i%2==0){
                *(ptrV++) =clip_value(v,0,255);
                }
            }
        }
    }
    return true;
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>

#include "UVCCamera.h"
#include "Utils.h"

UVCCamera::UVCCamera()
{
}

UVCCamera::~UVCCamera()
{
    free(usr_buf);
}

void UVCCamera::init_mmap(void)
{
	struct v4l2_requestbuffers reqbufs;
	memset(&reqbufs, 0, sizeof(reqbufs));
	reqbufs.count = 4; 	 							
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    
	reqbufs.memory = V4L2_MEMORY_MMAP;				

    if(-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbufs))
    {
		perror("Fail to ioctl 'VIDIOC_REQBUFS'");
		exit(EXIT_FAILURE);
	}

	n_buffer = reqbufs.count;
	printf("n_buffer = %d\n", n_buffer);
	usr_buf = (BUFTYPE*)calloc(reqbufs.count, sizeof(BUFTYPE));
	if(usr_buf == NULL)
	{
		printf("Out of memory\n");
		exit(-1);
	}

    //查询分辨率
    v4l2_ioctl_supported_framesize(fd);

	for(n_buffer = 0; n_buffer < reqbufs.count; ++n_buffer)
	{
		
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffer;
		
        if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
		{
			perror("Fail to ioctl : VIDIOC_QUERYBUF");
			exit(EXIT_FAILURE);
		}

		usr_buf[n_buffer].length = buf.length;
		usr_buf[n_buffer].start = (char *)mmap(NULL,buf.length,PROT_READ | PROT_WRITE,MAP_SHARED, fd,buf.m.offset);

		if(MAP_FAILED == usr_buf[n_buffer].start)
		{
			perror("Fail to mmap");
			exit(EXIT_FAILURE);
		}

	}

}

void UVCCamera::open_camera(char *dev_video)
{
	struct v4l2_input inp;

    //fd = open(DEV_VIDEO, O_RDWR | O_NONBLOCK,0);
    fd = open(dev_video, O_RDWR | O_NONBLOCK,0);
	if(fd < 0)
	{	
        fprintf(stderr, "%s open err \n", DEV_VIDEO);
		exit(EXIT_FAILURE);
	};

	inp.index = 0;
	if (-1 == ioctl (fd, VIDIOC_S_INPUT, &inp))
	{
		fprintf(stderr, "VIDIOC_S_INPUT \n");
	}
}

void UVCCamera::init_camera(void)
{
	struct v4l2_capability 	cap;		
	struct v4l2_format 	    tv_fmt;		
	struct v4l2_fmtdesc 	fmtdesc;  	
	int ret;
	
	memset(&fmtdesc, 0, sizeof(fmtdesc));
	fmtdesc.index = 0 ;                
	fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	ret=ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if(ret < 0)
	{
		fprintf(stderr, "fail to ioctl VIDEO_QUERYCAP \n");
		exit(EXIT_FAILURE);
	}
    else
    {
        printf("\ncamera driver name is : %s\n",cap.driver);
        printf("camera device name is : %s\n",cap.card);
        printf("camera bus information: %s\n",cap.bus_info);    }

    while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
    {
        printf("\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
        fmtdesc.index++;
    }

	if(!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE))
	{
		fprintf(stderr, "The Current device is not a video capture device \n");
		exit(EXIT_FAILURE);
	}

	if(!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("The Current device does not support streaming i/o\n");
		exit(EXIT_FAILURE);
	}

	tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   
	tv_fmt.fmt.pix.width = WIDTH;
	tv_fmt.fmt.pix.height = HEIGHT;
#ifdef YUYV
    tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
#elif defined NV21
    tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV21;
#elif defined MJPEG
    tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
#endif
    tv_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if (ioctl(fd, VIDIOC_S_FMT, &tv_fmt)< 0) 
	{
		fprintf(stderr,"VIDIOC_S_FMT set err\n");
		exit(-1);
		close(fd);
	}
}

void UVCCamera::start_capture(void)
{
	unsigned int i;
	enum v4l2_buf_type type;
	
	for(i = 0; i < n_buffer; i++)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
		{
			perror("Fail to ioctl 'VIDIOC_QBUF'");
			exit(EXIT_FAILURE);
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd, VIDIOC_STREAMON, &type))
	{
		printf("i=%d.\n", i);
		perror("VIDIOC_STREAMON");
		close(fd);
		exit(EXIT_FAILURE);
	}

}

void UVCCamera::read_one_frame(unsigned char *frame)
{
    struct v4l2_buffer v4l2_buf;
    
    memset(&v4l2_buf, 0, sizeof(v4l2_buf));
    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;

    if(-1 == ioctl(fd, VIDIOC_DQBUF, &v4l2_buf))
	{
		perror("Fail to ioctl 'VIDIOC_DQBUF'");
		exit(EXIT_FAILURE);
	}
    assert(v4l2_buf.index < n_buffer);

    //指向获取到的frame地址
#if 0
    frame = (unsigned char *)usr_buf[v4l2_buf.index].start;
#else
    memcpy(frame, usr_buf[v4l2_buf.index].start, WIDTH*HEIGHT*2);
#endif

    if(-1 == ioctl(fd, VIDIOC_QBUF, &v4l2_buf))
    {
        perror("Fail to ioctl 'VIDIOC_QBUF'");
        exit(EXIT_FAILURE);
    }
	
}

void UVCCamera::stop_capture(void)
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd,VIDIOC_STREAMOFF,&type))
	{
		perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
		exit(EXIT_FAILURE);
	}
}

void UVCCamera::close_camera(void)
{
	unsigned int i;
	for(i = 0;i < n_buffer; i++)
	{
		if(-1 == munmap(usr_buf[i].start,usr_buf[i].length))
		{
			exit(-1);
		}
	}

	free(usr_buf);
	if(-1 == close(fd))
	{
		perror("Fail to close fd");
		exit(EXIT_FAILURE);
	}

}

int UVCCamera::camera_able_read(void)
{
    fd_set fds;			
    struct timeval tv;
    int ret;			
    FD_ZERO(&fds);			
    FD_SET(fd,&fds);			/*Timeout*/			
    tv.tv_sec = 2;			
    tv.tv_usec = 0;			
    ret = select(fd + 1,&fds,NULL,NULL,&tv);

    if(-1 == ret)
    {    
        if(EINTR == errno)
		return -1;
     
        perror("Fail to select");   
        exit(EXIT_FAILURE);
    }
    
    if(0 == ret)
    { 
        fprintf(stderr,"select Timeout\n");
        //exit(-1);
    }
    
    return ret;
}


int UVCCamera::getnextframe(unsigned char *frame, long &frame_idx)
{
	int ret;
    ret = camera_able_read();
    if(ret > 0)
    {
         
        read_one_frame(frame);
        frame_idx++;
        return 0;
    }
	else
	{
        printf("Read Frame Failed!\n");
        return -1;
	}    
}

void UVCCamera::Init(char *dev_video)
{
    open_camera(dev_video);
	init_camera();
    init_mmap();
    start_capture();
}

void UVCCamera::Destory()
{
	stop_capture();
	close_camera();
}

int UVCCamera::v4l2_ioctl_supported_framesize(int fd)
{
    if (-1 != fd)
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        struct v4l2_fmtdesc fmtdesc;
        struct v4l2_frmivalenum frmival;
        struct v4l2_frmsizeenum frmsize;
        fmtdesc.index = 0;
        fmtdesc.type = type;

        while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0)
        {
            frmsize.pixel_format = fmtdesc.pixelformat;
            frmsize.index = 0;
            while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0)
            {
                if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
                {
                    printf("supported size = %dx%d\n", frmsize.discrete.width, frmsize.discrete.height);
                }
                else if(frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
                {
                    printf("allan2:%dx%d\n",frmsize.discrete.width, frmsize.discrete.height);
                }
                frmsize.index++;
            }
            fmtdesc.index++;
            printf("\n\n");
        }
    }
    return 0;
}

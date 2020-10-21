#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "RingBuffer.h"

//计算数据空间大小
int rb_data_size (Ringbuff *rb)
{
    //return ( (rb->wr_pointer - rb->rd_pointer) & (rb->size - 1));
    //modified by flq, input length
    return 1;
}

//计算空闲空间大小
int rb_free_size (Ringbuff *rb)
{
    return ( rb->size - 1 - rb_data_size(rb));
}

void rb_init (Ringbuff *rb, INT16 *buf, int size)
{
   memset(rb,0,sizeof(Ringbuff));
   rb->rd_pointer = 0;
   rb->wr_pointer = 0;
   rb->buff = buf;
   //printf("rb->buff=%p, buf=%p\n", rb->buff, buf);
   rb->size = size;
}

void clear (Ringbuff *rb)
{
    memset(rb->buff,0,rb->size);
    rb->rd_pointer = 0;
    rb->wr_pointer = 0;
}

int rb_write (Ringbuff *rb, INT16 *buf, int len)
{
    int rb_freesize = rb_free_size(rb);  //空闲空间大小
    int pos = 0;          //读指针位置

    if (len > rb_freesize)
    {
        len = rb_freesize;  //输入超过空闲空间，自动截断
    }
    else
    {
        rb_freesize = len;
    }

    pos = rb->wr_pointer;

    if (pos + len > rb->size)    //如果写入的数据加上超过循环buff大小
    {
        memcpy(rb->buff + pos, buf, rb->size - pos);   //先拷贝未超过那部分，剩下的写入前面那部分空闲空间
        buf += rb->size - pos;
        len -= rb->size - pos;
        pos = 0;
    }

    memcpy(rb->buff + pos, buf, len);
    rb->wr_pointer = pos + len;   //设置写指针位置
    return rb_freesize;
}

int rb_read (Ringbuff *rb, INT16 * buf, int max)
{
    int datasize=rb_data_size(rb);
    int pos = 0;

    if (max > datasize)
    {
        max = datasize;
    }
    else
    {
        datasize = max;
    }

    pos = rb->rd_pointer;

    if (pos + max > rb->size)
    {
        memcpy(buf,rb->buff + pos,rb->size - pos);
        buf +=rb->size;
        max -=rb->size;
        pos = 0;
    }

    memcpy(buf,rb->buff + pos,max);
    rb->rd_pointer = pos + max;

    return datasize;
}

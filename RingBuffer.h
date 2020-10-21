#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//u_char
typedef int	INT16;
typedef struct ring_buff Ringbuff;

struct ring_buff
{
    INT16 *buff;        //定义buff空间，用于存储数据
    int wr_pointer;      //写指针
    int rd_pointer;      //读指针
    int size;
};

int rb_data_size (Ringbuff *rb);    //计算数据空间大小
int rb_free_size (Ringbuff *rb);   //计算空闲空间大小
void rb_init (Ringbuff *rb, INT16 *buf, int size);
void clear (Ringbuff *rb);
int rb_write (Ringbuff *rb, INT16 *buf, int len);
int rb_read (Ringbuff *rb, INT16 * buf, int max);

#endif // _RINGBUFFER_H

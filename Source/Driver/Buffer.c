#include "buffer.h"
#include <string.h>
#
#define TRUE 0xFF
#define FALSE 0

#define BUFFER_LEN  16
#include <xc.h>

/**
 *缓冲空间，暂定为16个 
 */
CAN_msg CanMsg[BUFFER_LEN];

/**
 *缓冲队列信息
 */
FifoInformation FifoInfor;

/**
 *初始化缓冲区
 */
void BufferInit(void)
{
    FifoInfor.capacity = BUFFER_LEN;
    FifoInfor.count = 0;
    FifoInfor.end = 0;
    FifoInfor.head = 0;
    FifoInfor.pMsg = CanMsg;
}


/**
 * 缓冲数据入队
 * <p>
 * 
 * @param  pInf     FIFO信息
 * @param  pMsg    入队信息
 * @return          <code>TRUE</code>   成功入队
 *                  <code>FASLE</code>  失败
 */
uint8_t len = 0;
uint8_t BufferEnqueue( CAN_msg* pMsg)
{
    FifoInformation* pInf = &FifoInfor;
    //队列未满

    if (pInf->count < pInf->capacity)
    {
        len =  sizeof(CAN_msg);
        memcpy(pInf->pMsg + pInf->end, pMsg, len);
        pInf->end = (pInf->end + 1) % pInf->capacity;
        pInf->count++;
        return TRUE;
    }
    
    //溢出报错
    //TODO:溢出报错机制
    return FALSE;
}
/**
 * 缓冲数据出队
 * <p>
 * 
 * @param  pInf     FIFO信息
 * @param  pMsg    入队信息
 * @return          <code>TRUE</code>   成功出队
 *                  <code>FASLE</code>  失败
 */
uint8_t BufferDequeue( CAN_msg* pMsg)
{
    FifoInformation* pInf = &FifoInfor;
    ClrWdt();
    if (pInf->count > 0)
    {        
        memcpy( pMsg, pInf->pMsg + pInf->head , sizeof(CAN_msg));
        pInf->head = (pInf->head + 1) % pInf->capacity;
        pInf->count--;
        return TRUE;
    }
    return FALSE;
}

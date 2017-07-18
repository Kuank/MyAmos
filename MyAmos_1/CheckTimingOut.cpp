/*
 *The function TesterAcceptUDS() is used for to check timeout of Tester receiving responses.
 * TesterAcceptUDS(int fd, struct can_frame * UDS_frame_response, struct can_frame * UDS_frame_require);
 * The fd is a socketcan;
 * The UDS_frame_response is a response;
 * The UDS_frame_require is a require;
 * If the function don't get response or time out, it will send a next require.
 * 
 */


// SET TIMEOUT= TIME_OUT_SEC+TIME_OUT_USEC
#define TIME_OUT_SEC 5  // s
#define TIME_OUT_USEC 10 // ms

#include <cstdlib>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;
        
#include <linux/socket.h>
#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/raw.h>


#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>


void print_frame(struct can_frame *fr)
{
    int i;
    printf("CAN ID:%03x\n", fr->can_id & CAN_EFF_MASK);
    //printf("%08x\n", fr->can_id);
    printf("dlc = %d\n", fr->can_dlc);
    printf("data = ");
    for (i = 0; i < fr->can_dlc; i++)
        printf("%02x ", fr->data[i]);
    printf("\n");
}

#define errout(_s)    fprintf(stderr, "error class: %s\n", (_s))
#define errcode(_d) fprintf(stderr, "error code: %02x\n", (_d))

static void handle_err_frame(const struct can_frame *fr)
{
    if (fr->can_id & CAN_ERR_TX_TIMEOUT) {
        errout("CAN_ERR_TX_TIMEOUT");
    }
    if (fr->can_id & CAN_ERR_LOSTARB) {
        errout("CAN_ERR_LOSTARB");
        errcode(fr->data[0]);
    }
    if (fr->can_id & CAN_ERR_CRTL) {
        errout("CAN_ERR_CRTL");
        errcode(fr->data[1]);
    }
    if (fr->can_id & CAN_ERR_PROT) {
        errout("CAN_ERR_PROT");
        errcode(fr->data[2]);
        errcode(fr->data[3]);
    }
    if (fr->can_id & CAN_ERR_TRX) {
        errout("CAN_ERR_TRX");
        errcode(fr->data[4]);
    }
    if (fr->can_id & CAN_ERR_ACK) {
        errout("CAN_ERR_ACK");
    }
    if (fr->can_id & CAN_ERR_BUSOFF) {
        errout("CAN_ERR_BUSOFF");
    }
    if (fr->can_id & CAN_ERR_BUSERROR) {
        errout("CAN_ERR_BUSERROR");
    }
    if (fr->can_id & CAN_ERR_RESTARTED) {
        errout("CAN_ERR_RESTARTED");
    }
}

#define myerr(str)    fprintf(stderr, "%s, %s, %d: %s\n", __FILE__, __func__, __LINE__, str)

 int TesterAcceptUDS(int fd, struct can_frame * UDS_frame_response, struct can_frame * UDS_frame_require)
{
 
    int ret, i;
    struct can_frame frdup;
    struct timeval tv;
    fd_set rset;

    
        tv.tv_sec = TIME_OUT_SEC;          //set waiting time
        tv.tv_usec = TIME_OUT_USEC;
        FD_ZERO(&rset);
        FD_SET(fd, &rset);

        ret = select(fd+1, &rset, NULL, NULL, & tv);
        if (ret == 0) {
            cout<<"time out"<<endl;
            write(fd, UDS_frame_require, sizeof(struct can_frame));
            return 0;
        }

        ret = read(fd, &frdup, sizeof(frdup));
        if (ret < sizeof(frdup)) {
            myerr("read failed");
            return 0;
        }
     
        //print_frame(&frdup);
   
        UDS_frame_response->can_id=frdup.can_id;
        UDS_frame_response->can_dlc=frdup.can_dlc;
        for (int i=0;i<8;i++)
        {
            UDS_frame_response->data[i]=frdup.data[i];
        }
        

    return 1;
   
}



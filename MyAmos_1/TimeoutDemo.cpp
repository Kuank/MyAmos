/*
 * 
 */

/* 
 * File:   Demo.cpp
 * Author: Yanquan
 *
 * Created on June 12, 2017, 6:46 PM
 */


        
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>

#include <linux/socket.h>
#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/raw.h>


#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "CheckTimingOut.h"
using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

    int s;
    int ret;
    int c;
    struct sockaddr_can addr;
    struct ifreq ifr;
   
    struct can_frame UDS_frame_response, UDS_frame_require;
    
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0)
    {
       perror("Error while opening socket");
       return 1;
    }

    strcpy(ifr.ifr_name, "vcan0");
    
    ret = ioctl(s, SIOCGIFINDEX, &ifr);
    if (ret < 0) 
    {
        perror("ioctl failed");
        return 1;
    }
    
    
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    
    if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Error in socket bind");
		return 1;
	}
    
   
	UDS_frame_require.can_id  = 0x001;
	UDS_frame_require.can_dlc = 2;
	UDS_frame_require.data[0] = 0x10;
	UDS_frame_require.data[1] = 0x02;
        
    
    c=TesterAcceptUDS(s, & UDS_frame_response,& UDS_frame_require); // get UDS frame in time from socket s OR print time out and send require again
    
    
    if (c){
    cout<<"UDS_frame:"<<endl;
    print_frame(& UDS_frame_response);
    
    }
    else
    {
    cout<<"send next require"<<endl;
    }
    return 0;
}


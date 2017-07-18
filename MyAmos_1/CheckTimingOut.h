

/* 
 * File:   CheckTimingOut.h
 * Author: Yanquan
 *
 * Created on June 13, 2017, 10:37 AM
 */

#ifndef CHECKTIMINGOUT_H
#define CHECKTIMINGOUT_H


int TesterAcceptUDS(int fd, struct can_frame * UDS_frame_response, struct can_frame * UDS_frame_require);
void print_frame(struct can_frame *fr);
#endif /* CHECKTIMINGOUT_H */


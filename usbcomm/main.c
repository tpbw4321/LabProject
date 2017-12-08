//
//  main.c
//  usbcomm
//
//  Created by Barron Wong on 12/7/17.
//  Copyright © 2017 Barron Wong. All rights reserved.
//
#include "libusb.h"
#include <stdio.h>
#include "usbcomm.h"

#define EP1 (0x80|0x01)
#define EP2 (0x80|0x02)
#define EP3 (0x00|0x03)

int main(int argc, const char * argv[]) {
    
    libusb_device_handle * dev;
    int rcvd_bytes = 0;
    
    dev = SetupDevHandle(0x04B4, 0x8051);
    unsigned char buffer[1000];
    
    while(1){
        
        
        if(libusb_bulk_transfer(dev, EP1, buffer, 1000, &rcvd_bytes, 0)!=0)
            return -1;
        else{
            printf("Recieved EP1: %d\n",rcvd_bytes);
            for(int i = 0; i < rcvd_bytes; i++){
                printf("%3d ",buffer[i]);
                if(i%16==15)
                    printf("\n");
            }
        }
        
        if(libusb_bulk_transfer(dev, EP2, buffer, 1000, &rcvd_bytes, 0)!=0)
            return -1;
        else{
            printf("Recieved EP2: %d\n",rcvd_bytes);
            for(int i = 0; i < rcvd_bytes; i++){
                printf("%3d ",buffer[i]);
                if(i%16==15)
                    printf("\n");
            }
        }
        
        
    }
    
    
    return 0;
}


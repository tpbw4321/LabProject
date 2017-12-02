//
//  main.c
//  libusb
//
//  Created by Barron Wong on 12/2/17.
//  Copyright © 2017 libusb. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "libusb.h"

static struct libusb_transfer * iso = NULL; //Isochronous Transfer Handler
static libusb_device_handle * dev = NULL;   //USB Device Handler

//Callback function for isochronous transfer
static void LIBUSB_CALL ReadBufferData(struct libusb_transfer *transfer){
    static unsigned int count = 0;
    struct libusb_iso_packet_descriptor *ipd = transfer->iso_packet_desc;
    char * item;
    
    if(ipd->status == LIBUSB_TRANSFER_COMPLETED){
        *(int *)transfer->user_data = ipd->actual_length;
        for(int i = 0; i < ipd->actual_length; i++){
            item = (char*)malloc(sizeof(char));
            *item = buffer[i];
            Enqueue(&rawData, item);
        }
    }
    else{
        perror("Failed");
        *(int *)transfer->user_data = -1;
        
    }
}

int main(int argc, char **argv) {
    
}


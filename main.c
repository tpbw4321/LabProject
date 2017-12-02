//
//  main.c
//  libusb
//
//  Created by Barron Wong on 12/2/17.
//  Copyright © 2017 libusb. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "libusb.h"
#include "usbcomm.h"
#include 

#define EP1 (0x80|0x01)
#define EP2 (0x80|0x02)
#define EP3 (0x80|0x03)
#define EP4 (0x00|0x04)
#define PACKET_SIZE 500                     //Ischronous Packet Size


static struct libusb_transfer * iso = NULL; //Isochronous Transfer Handler
static libusb_device_handle * dev = NULL;   //USB Device Handler
static unsigned char buffer[PACKET_SIZE];   //Transfer Buffer
static queue rawData;                       //Data from PSOC
static queue processedData;                 //Data converted data_points


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


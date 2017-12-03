//
//  main.c
//  libusb
//
//  Created by Barron Wong on 12/2/17.
//  Copyright © 2017 libusb. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <shapes.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "queue.h"
#include "usbcomm.h"
#include "cmdargs.h"
#include "scope.h"

#define EP1 (0x80|0x01)
#define EP2 (0x80|0x02)
#define EP3 (0x80|0x03)
#define EP4 (0x00|0x04)
#define PACKET_SIZE 500                     //Ischronous Packet Size
#define BUFFER_SIZE 2000
#define PFLAG 2
#define PERIOD 3

static int check = 0;
static argOptions options;
static struct libusb_transfer * iso = NULL; //Isochronous Transfer Handler
static libusb_device_handle * dev = NULL;   //USB Device Handler
static unsigned char buffer[PACKET_SIZE];   //Transfer Buffer
static queue rawData;                       //Data from PSOC
static queue processedData;                 //Data converted data_points
static unsigned int trigFlag = 0;           //trigger detect
static unsigned char period[8];


//Callback function for isochronous transfer
static void LIBUSB_CALL ReadBufferData(struct libusb_transfer *transfer){
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

int main(int argc, const char **argv) {
    
    int width, height; // Width and height of screen in pixels
    int margin = 10; // Margin spacing around screen
    int xdivisions = 10; // Number of x-axis divisions
    int ydivisions = 8; // Number of y-axis divisions
    char str[100];
    
    int xscale = 100; // x scale (in units of us)
    int yscale = 8; // y scale (in units of 0.25V)
    VGfloat textcolor[4] = {0, 200, 200, 0.5}; // Color for displaying text
    VGfloat wave1color[4] = {240, 0, 0, 0.5}; // Color for displaying Channel 1 data
    VGfloat wave2color[4] = {200, 200, 0, 0.5}; // Color for displaying Channel 2 data
    
    saveterm(); // Save current screen
    init(&width, &height); // Initialize display and get width and height
    
    int xstart = margin;
    int xlimit = width - 2*margin;
    int ystart = margin;
    int ylimit = height - 2*margin;
    unsigned char potReading[2] = {0,0};
    
    int pixels_per_volt = (ylimit-ystart)*4/(ydivisions*yscale);
    
    int return_val;
    
    int sent_bytes = 0;
    int potScaleFac = height/256;
    int buffLoad = 0;
    
    
    //Setup USB Device Handler
    dev = SetupDevHandle(0x04B4, 0x8051);
    
    //Allocate Isochornous Transfer
    
    iso = SetupIsoTransfer(dev, EP1, buffer, 1, ReadBufferData, &check);
    
    //Grab options
    ParseArgs(argc, argv, &options);
    
    DisplayAllSettings(&options);
    
    period[PERIOD] = options.xScale.period;
    period[PFLAG]  = 1;
    
    //Send setting to PSOC
    libusb_interrupt_transfer(dev, EP4, period, 8, &sent_bytes, 0);
    
    
    while(1){
        if(PacketTransfer(dev, iso, EP1, NULL, &check, LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)){
            if(!trigFlag && !options.mode){
                if(FindTrigger(&rawData, &options)){
                    trigFlag = 1;
                }
            }
        }
        
        if(!PacketTransfer(dev, NULL, EP3, potReading, NULL, LIBUSB_TRANSFER_TYPE_INTERRUPT)){
            perror("Recieve 3");
            return -1;
        }
        
        
        Start(width, height);
        drawBackground(width, height, xdivisions, ydivisions, margin);
        printScaleSettings(xscale, yscale, width-300, height-50, textcolor);
        
        if(rawData.count > BUFFER_SIZE){
            buffLoad = 1;
        }
        
        
        if(rawData.count >= options.xScale.samples && buffLoad){
            if(trigFlag || options.mode){
                processSamples(&rawData, options.xScale.samples, margin, width, pixels_per_volt,options.yScale, &processedData);
                plotWave(&processedData, options.xScale.samples, margin+(potReading[0]*potScaleFac), wave1color);
                trigFlag = 0;
            }
        }else{
            buffLoad = 0;
        }
        End();
        
    }
    
    restoreterm();
    finish();
    return 0;
}


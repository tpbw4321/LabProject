#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <shapes.h>
#include <fontinfo.h>
#include <math.h>
#include <errno.h>
#include <libusb.h>
#include "usbcomm.h"
#include "queue.h"

#define PACKET_SIZE 500
#define SAMP_SIZE   100
#define BUFFER_SIZE 2000

#define PI 3.14159265
#define EP1 (0x80|0x01)
#define EP2 (0x80|0x02)
#define EP3 (0x80|0x03)
#define EP4 (0x00|0x04)
static unsigned char buffer[PACKET_SIZE];   //Transfer Buffer
static struct libusb_transfer * iso = NULL; //Isochronous Transfer Handler
static libusb_device_handle * dev = NULL;   //USB Device Handler
static queue rawData;                       //Data from PSOC
static queue processedData;                 //Data converted data_points
static int check = 0;                       //isochronous completion check
static int posTrigger = 1;                 //Flag for tigger 0 NEG 1 POS
static int trigger = 0;
static int trigFlag = 0;
static int freeRun = 1;
static int yScaleDivsor = 640;

typedef struct{
    VGfloat x, y;
} data_point;


// Draw grid lines
void grid(VGfloat x, VGfloat y, // Coordinates of lower left corner
          int nx, int ny, // Number of x and y divisions
          int w, int h) // screen width and height

{
    VGfloat ix, iy;
    Stroke(128, 128, 128, 0.5); // Set color
    StrokeWidth(2); // Set width of lines
    for (ix = x; ix <= x + w; ix += w/nx) {
        Line(ix, y, ix, y + h);
    }
    
    for (iy = y; iy <= y + h; iy += h/ny) {
        Line(x, iy, x + w, iy);
    }
}

// Draw the background for the oscilloscope screen
void drawBackground(int w, int h, // width and height of screen
                    int xdiv, int ydiv,// Number of x and y divisions
                    int margin){ // Margin around the image
    VGfloat x1 = margin;
    VGfloat y1 = margin;
    VGfloat x2 = w - 2*margin;
    VGfloat y2 = h - 2*margin;
    
    Background(128, 128, 128);
    
    Stroke(204, 204, 204, 1);
    StrokeWidth(1);
    
    Fill(44, 77, 120, 1);
    Rect(10, 10, w-20, h-20); // Draw framing rectangle
    
    grid(x1, y1, xdiv, ydiv, x2, y2); // Draw grid lines
}

// Display x and scale settings
void printScaleSettings(int xscale, int yscale, int xposition, int yposition, VGfloat tcolor[4]) {
    char str[100];
    
    setfill(tcolor);
    if (xscale >= 1000)
        sprintf(str, "X scale = %0d ms/div", xscale/1000);
    else
        sprintf(str, "X scale = %0d us/div", xscale);
    Text(xposition, yposition, str, SansTypeface, 18);
    
    sprintf(str, "Y scale = %3.2f V/div", yscale * 0.25);
    Text(xposition, yposition-50, str, SansTypeface, 18);
}

// Convert waveform samples into screen coordinates
void processSamples(queue *rawData,  // sample data
                    int nsamples,    // Number of samples
                    int xstart,      // starting x position of wave
                    int xfinish,     // Ending x position of wave
                    float yscale,    // y scale in pixels per volt
                    queue *processedData){
    VGfloat x1, y1;
    data_point * p;
    char * data;
    
    for (int i=0; i< nsamples; i++){
        data = (char*)Dequeue(rawData);
        p = (data_point *) malloc(sizeof(data_point));
        x1 = xstart + (xfinish-xstart)*i/nsamples;
        y1 = *data * 5 * yscale * 5/yScaleDivsor;
        p->x = x1;
        p->y = y1;
        free(data);
        Enqueue(processedData, p);
        
    }
}


// Plot waveform
void plotWave(queue *processedData, // sample data
              int nsamples, // Number of samples
              int yoffset, // y offset from bottom of screen
              VGfloat linecolor[4] // Color for the wave
){
    
    data_point * p;
    VGfloat x1, y1, x2, y2;
    
    Stroke(linecolor[0], linecolor[1], linecolor[2], linecolor[3]);
    StrokeWidth(4);
    
    p = (data_point*)Dequeue(processedData);
    x1 = p->x;
    y1 = p->y + yoffset;
    free(p);
    for(int i=1; i< nsamples; i++){
        p = (data_point*)Dequeue(processedData);
        x2 = p->x;
        y2 = p->y + yoffset;
        free(p);
        Line(x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;
    }
}
int FindTrigger(){
    char current = NULL;
    char previous = NULL;
    char * data;
    
    previous = *(char *) rawData.head->item;
    
    while(rawData.count > 0){
        if(rawData.head->prev){
            current  = *(char *) rawData.head->prev->item;
            if(posTrigger){
                if(previous < trigger && current > trigger){
                    return 1;
                }else{
                    data = Dequeue(&rawData);
                    free(data);
                    previous = current;
                }
            }else{
                if(previous > trigger && current < trigger){
                    return 1;
                }else{
                    data = Dequeue(&rawData);
                    free(data);
                    previous = current;
                }
            }
        }
        else{
            break;
        }
    }
    return 0;
}

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

// main initializes the system and shows the picture.
// Exit and clean up when you hit [RETURN].
int main(int argc, char **argv) {
    
    // Open the USB device (the Cypress device has
    // Vendor ID = 0x04B4 and Product ID = 0x8051)
    dev = SetupDevHandle(0x04B4, 0x8051);
    
    
    //Allocate isochronous transfer
    iso = SetupIsoTransfer(dev, EP1, buffer, 1, ReadBufferData, &check);
    
    
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
    char potReading[2] = {0,0};
    
    int pixels_per_volt = (ylimit-ystart)*4/(ydivisions*yscale);
    
    int return_val;
    
    int sent_bytes = 0;
    int potScaleFac = height/256;
    int buffLoad = 0;
    
    int count = 0;
    char period[8];
    trigger = 128;
    
    for(int i = 0; i < 8; i++){
        period[i] = 0;
    }
    
    period[2] = 1;
    period[3] = 100;


       /*libusb_bulk_transfer( dev, // Handle for the USB device
                             EP4, // Address of the Endpoint in USB device
                             period, // address of data block to transmit
                             8, // Size of data block
                             &sent_bytes, // Number of bytes actually sent
                             0 // Timeout in milliseconds (0 to disable timeout)
                             );*/
    
    libusb_interrupt_transfer(dev, EP4, period, 8, &sent_bytes, 0);


    
    while(1){
        if(PacketTransfer(dev, iso, EP1, NULL, &check, LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)){
            if(!trigFlag && !freeRun){
                if(FindTrigger()){
                    trigFlag = 1;
                    printf("Trigger Count: %d", ++count);
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
        
        if(rawData.count >= SAMP_SIZE && buffLoad){
            if(trigFlag || freeRun){
                processSamples(&rawData, SAMP_SIZE, margin, width, pixels_per_volt, &processedData);
                plotWave(&processedData, SAMP_SIZE, margin+(potReading[0]*potScaleFac), wave1color);
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



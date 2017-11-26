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

#define PI 3.14159265

typedef struct{
    VGfloat x, y;
} data_point;


// Square wave generator for demo
void sineWave(int period, // period in us
              int sampleInterval, // Samping interval in us
              int samples, // total samples
              int *data){
    double rad, f;
    for (int i=0; i<samples; i++){
        if ((i *sampleInterval) % period < period/2)
            data[i] = 0;
        else
            data[i] = 255;
    }
}

// wait for a specific key pressed
void waituntil(int endchar) {
    int key;
    
    for (;;) {
        key = getchar();
        if (key == endchar || key == '\n') {
            break;
        }
    }
}


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
void processSamples(int *data, // sample data
                    int nsamples, // Number of samples
                    int xstart, // starting x position of wave
                    int xfinish, // Ending x position of wave
                    float yscale, // y scale in pixels per volt
                    data_point *point_array){
    VGfloat x1, y1;
    data_point p;
    
    for (int i=0; i< nsamples; i++){
        x1 = xstart + (xfinish-xstart)*i/nsamples;
        y1 = data[i] * 5 * yscale/256;
        p.x = x1;
        p.y = y1;
        point_array[i] = p;
    }
}


// Plot waveform
void plotWave(data_point *data, // sample data
              int nsamples, // Number of samples
              int yoffset, // y offset from bottom of screen
              VGfloat linecolor[4] // Color for the wave
){
    
    data_point p;
    VGfloat x1, y1, x2, y2;
    
    Stroke(linecolor[0], linecolor[1], linecolor[2], linecolor[3]);
    StrokeWidth(4);
    
    p = data[0];
    x1 = p.x;
    y1 = p.y + yoffset;
    
    for (int i=1; i< nsamples; i++){
        p = data[i];
        x2 = p.x;
        y2 = p.y + yoffset;
        Line(x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;
    }
}

// main initializes the system and shows the picture.
// Exit and clean up when you hit [RETURN].
int main(int argc, char **argv) {
    
    //USB Setup
    libusb_device_handle* dev; // Pointer to data structure representing USB device
    
    libusb_init(NULL); // Initialize the LIBUSB library
    
    // Open the USB device (the Cypress device has
    // Vendor ID = 0x04B4 and Product ID = 0x8051)
    dev = libusb_open_device_with_vid_pid(NULL, 0x04B4, 0x8051);
    
    if (dev == NULL){
        perror("device not found\n");
    }
    
    // Reset the USB device.
    // This step is not always needed, but clears any residual state from
    // previous invocations of the program.
    if (libusb_reset_device(dev) != 0){
        perror("Device reset failed\n");
    }
    
    // Set configuration of USB device
    if (libusb_set_configuration(dev, 1) != 0){
        perror("Set configuration failed\n");
    }
    
    
    // Claim the interface.  This step is needed before any I/Os can be
    // issued to the USB device.
    if (libusb_claim_interface(dev, 0) !=0){
        perror("Cannot claim interface");
    }
    
    char adcData1[64]; // Channel1 data block
    char adcData2[64]; // Receive data block
    
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
    
    int channel1_data[10000]; // Data samples from Channel 1
    int channel2_data[10000]; // Data samples from Channel 2
    
    data_point channel1_points[10000];
    data_point channel2_points[10000];
    
    for(int i = 0; i < 10000; i++){
        channel1_data[i] = 0;
        channel2_data[i] = 128;
    }
    
    saveterm(); // Save current screen
    init(&width, &height); // Initialize display and get width and height
    //rawterm(); // Needed to receive control characters from keyboard, such as ESC
    
    int xstart = margin;
    int xlimit = width - 2*margin;
    int ystart = margin;
    int ylimit = height - 2*margin;
    char potReading[2] = {0,0};
    
    int pixels_per_volt = (ylimit-ystart)*4/(ydivisions*yscale);
    
    int return_val;
    
    
    int rcvd_bytes;
    int potScaleFac = 0;
    
    potScaleFac = height/256;

    
    while(1){
        
        //Endpoint 1
        do{
            return_val = libusb_bulk_transfer(dev,(0x01 | 0x80), adcData1, 64, &rcvd_bytes, 1000);
            if(return_val != 0){
                perror("Recieve 1");
            }
        }while(return_val != 0);
        
        
        for(int i = 0; i < rcvd_bytes; i++){
            channel1_data[i] = adcData1[i];
        }
        
        //Endpoint 2
        do{
            return_val = libusb_bulk_transfer(dev,(0x02 | 0x80), adcData2, 64, &rcvd_bytes, 1000);
            if(return_val != 0){
                perror("Recieve 2");
            }
        }while(return_val != 0);
        
        for(int i = 0; i < rcvd_bytes; i++){
            channel2_data[i] = adcData2[i];
        }
        
        return_val = libusb_interrupt_transfer(dev, (0x03| 0x80), potReading, 2, &rcvd_bytes, 1000);
        if(return_val != 0){
            perror("Recieve 3");
        }
        else{
            printf("%d %d\n",potReading[0], potReading[1]);
        }

        
        Start(width, height);
        drawBackground(width, height, xdivisions, ydivisions, margin);
        printScaleSettings(xscale, yscale, width-300, height-50, textcolor);
        processSamples(channel1_data, 64, margin, width-2*margin, pixels_per_volt, channel1_points);
        processSamples(channel2_data, 64, margin, width-2*margin, pixels_per_volt, channel2_points);
        plotWave(channel1_points, 64, margin+(potReading[0]*potScaleFac), wave1color);
        plotWave(channel2_points, 64, margin+(potReading[1]*potScaleFac), wave2color);
        End();
        
    }
    
    
    
    
    
    waituntil(0x1b); // Wait for user to press ESC or RET key
    
    restoreterm();
    finish();
    return 0;
}



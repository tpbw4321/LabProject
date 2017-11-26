//
// Basic Oscilloscope GUI demo
// A. Varma, 11/3/2017
//
// To compile, use the command
// gcc -I /opt/vc/include -I /usr/include -lshapes -lm -o scopedemo scopedemo.c
//
// Press ESC or RET to exit.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <shapes.h>
#include <fontinfo.h>
#include <math.h>

#define PI 3.14159265

typedef struct{
  VGfloat x, y;
} data_point;


// Sine wave generator for demo
void sineWave(int period, // period in us
	      int sampleInterval, // Samping interval in us
	      int samples, // total samples
	      int *data){
  double rad, f;
  for (int i=0; i<samples; i++){
    rad = (double) (i * sampleInterval) * ((double) (2.0*PI/period));
    f = sin(rad);
    data[i] = (int) (f*128.0 + 128.0);
    if (data[i] > 255) data[i] = 255;
    if (data[i] < 0) data[i] = 0;
  }
}

// Square wave generator for demo
void squareWave(int period, // period in us
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

  saveterm(); // Save current screen
  init(&width, &height); // Initialize display and get width and height
  Start(width, height);
  rawterm(); // Needed to receive control characters from keyboard, such as ESC

  // Generate sample data for Channel 1, assuming 10 kHz sine wave, samples at 1 MHz rate
  sineWave(100, 1, 10000, channel1_data);

  // Generate sample data for Channel 2, assuming 20 kHz square wave, samples at 1 MHz rate
  squareWave(50, 1, 10000, channel2_data);

  int xstart = margin;
  int xlimit = width - 2*margin;
  int ystart = margin;
  int ylimit = height - 2*margin;
  
  int pixels_per_volt = (ylimit-ystart)*4/(ydivisions*yscale);

  drawBackground(width, height, xdivisions, ydivisions, margin);

  printScaleSettings(xscale, yscale, width-300, height-50, textcolor);

  processSamples(channel1_data, xscale*xdivisions, margin, width-2*margin, pixels_per_volt, channel1_points);
  processSamples(channel2_data, xscale*xdivisions, margin, width-2*margin, pixels_per_volt, channel2_points);	   

  // Display 2 waves
  plotWave(channel1_points, xscale*xdivisions, 600, wave1color);
  plotWave(channel2_points, xscale*xdivisions, 200, wave2color);
  End(); 

  sleep(5);

  // Shift the second wave 
  for (int i=0; i<7; i++){
    WindowClear();
    Start(width, height);
    drawBackground(width, height, xdivisions, ydivisions, margin);

    printScaleSettings(xscale, yscale, width-300, height-50, textcolor);

    plotWave(channel1_points, xscale*xdivisions, 400, wave1color);
    plotWave(channel2_points, xscale*xdivisions, 100+i*100, wave2color);

    End();
  }

  waituntil(0x1b); // Wait for user to press ESC or RET key

  restoreterm();
  finish();
  return 0;
}

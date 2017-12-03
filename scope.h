#ifndef scope_h
#define scope_h

#define PI 3.14159265

typedef struct{
    VGfloat x, y;
} data_point;


// Draw grid lines
void grid(VGfloat x, VGfloat y, // Coordinates of lower left corner
          int nx, int ny, // Number of x and y divisions
          int w, int h); // screen width and height


// Draw the background for the oscilloscope screen
void drawBackground(int w, int h, // width and height of screen
                    int xdiv, int ydiv,// Number of x and y divisions
                    int margin);

// Display x and scale settings
void printScaleSettings(int xscale, int yscale, int xposition, int yposition, VGfloat tcolor[4]);

// Convert waveform samples into screen coordinates
void processSamples(queue *rawData,  // sample data
                    int nsamples,    // Number of samples
                    int xstart,      // starting x position of wave
                    int xfinish,     // Ending x position of wave
                    float yscale,    // y scale in pixels per volt
                    int yScaleDivisor,
                    queue *processedData);



// Plot waveform
void plotWave(queue *processedData, // sample data
              int nsamples, // Number of samples
              int yoffset, // y offset from bottom of screen
              VGfloat linecolor[4] // Color for the wave
);


int FindTrigger(queue * rawData, argOptions * args);

#endif


//
//  cmdargs.c
//  cmdargs
//
//  Created by Barron Wong on 12/1/17.
//  Copyright © 2017 Barron Wong. All rights reserved.
//

#include "cmdargs.h"
#include <string.h>
#include <stdlib.h>

void DisplayAllSettings(const argOptions * options){
    printf("Mode:    %d\n", options->mode);
    printf("Trigger: %d\n", options->trigger);
    printf("Slope:   %d\n", options->trigSlope);
    printf("Channel: %d\n", options->trigChan);
    printf("Y-Scale: %d\n", options->yScale);
    printf("X-Scale Period:  %d\n", options->xScale.period);
    printf("X-Scale Samples: %d\n", options->xScale.samples);
}

//parseArguemnts and and calculates equivalents
//returns zero if completed successfully
int ParseArgs(int argc, const char * argv[], argOptions * options){
    char currOption = 0;
    char setting[255];
    char defaultFlag = 0;
    
    SetDefaultOptions(options);
    
    if(argc == 0){
        defaultFlag = 1;
    }
    
    for(int i = 1; i < argc; i=i+2){
        if(*argv[i] == '-'){
            currOption = *(argv[i]+1);
            
            switch(currOption){
                case 'm':
                    strcpy(setting, argv[i+1]);
                    SetMode(setting, options);
                    continue;
                case 't':
                    strcpy(setting, argv[i+1]);
                    SetTrigger(atoi(setting), options);
                    continue;
                case 's':
                    strcpy(setting, argv[i+1]);
                    SetTrigSlope(setting, options);
                    continue;
                case 'c':
                    strcpy(setting, argv[i+1]);
                    SetTrigChan(atoi(setting), options);
                    continue;
                case 'x':
                    strcpy(setting, argv[i+1]);
                    SetXScale(atoi(setting), options);
                    continue;
                case 'y':
                    strcpy(setting, argv[i+1]);
                    SetYScale(atoi(setting), options);
                    continue;
                default:
                    defaultFlag = 1;
                    break;
            }
        }else{
            defaultFlag = 1;
        }
        if(defaultFlag)
            printf("Invalid Settings - Defaults Selected\n");
            SetDefaultOptions(options);
        break;
    }
    
    return 0;
    
}
//Sets Default Options
void SetDefaultOptions(argOptions * options){
    //No Free Run
    options->mode = 0;
    //Trigger Channel 1
    options->trigChan = 1;
    //Trigger 2.5v
    options->trigger = 128;
    //Trigger slope pos
    options->trigSlope = 1;
    //xScale 1000us
    options->xScale.period = 5;
    options->xScale.samples = 200;
    options->xScale.time = 1000;
    //yScale 1000mV
    options->yScale = 640;
}

//sets free run mode returns 1 for freerun and 0 for tiggered
int SetMode(char * setting, argOptions * options){
    if(strcmp(setting, FREE) == 0){
        options->mode = 1;
    }
    else if(strcmp(setting,TRIGGER) == 0){
        options->mode = 0;
    }
    else{
        printf("Invalid Mode Setting - Default Selected\n");
    }
    return options->mode;
}

//Sets trigger and returns the set trigger value
int SetTrigger(int setting, argOptions * options){
    
    if(setting % 100 != 0 || 0 > setting || setting > 5000){
        printf("Invalid Trigger Setting - Default Selected\n");
        return -1;
    }
    options->trigger = (setting/TRIG_MAX)*255;
    return options->trigger;
}

//Sets trigger to pos or neg slope
int SetTrigSlope(char * setting, argOptions * options){
    if(strcmp(setting,POS)==0)
        options->trigSlope = 1;
    else if(strcmp(setting,NEG)==0)
        options->trigSlope = 0;
    else
        printf("Invalid Slope Setting - Default Selected\n");
    return options->trigSlope;
}

//Sets Trigger Channel
int SetTrigChan(int setting, argOptions * options){
    if(setting == 1 || setting == 2)
        options->trigChan = setting;
    else
        printf("Invalid Channel Setting - Default Selected\n");
    return options->trigChan;
}

//Sets xScale
int SetXScale(int setting, argOptions * options){
    options->xScale.time = setting;
    switch(setting){
        case 100:
            options->xScale.period = 5;
            options->xScale.samples = 80;
            break;
        case 500:
            options->xScale.period = 5;
            options->xScale.samples = 100;
            break;
        case 1000:
            options->xScale.period = 5;
            options->xScale.samples = 200;
            break;
        case 2000:
            options->xScale.period = 5;
            options->xScale.samples = 400;
            break;
        case 5000:
            options->xScale.period = 5;
            options->xScale.samples = 1000;
            break;
        case 10000:
            options->xScale.period = 100;
            options->xScale.samples = 100;
            break;
        case 50000:
            options->xScale.period = 100;
            options->xScale.samples = 500;
            break;
        case 100000:
            options->xScale.period = 100;
            options->xScale.samples = 1000;
            break;
        default:
            printf("Invalid x-scale Setting - Default Selected\n");
            options->xScale.time = 1000;
            break;
    }
    return 0;
}

//Sets yScale
int SetYScale(int setting, argOptions * options){
    switch(setting){
        case 100:
            options->yScale = 64;
            break;
        case 500:
            options->yScale = 320;
            break;
        case 1000:
            options->yScale = 640;
            break;
        case 2000:
            options->yScale = 1280;
            break;
        case 2500:
            options->yScale = 1600;
            break;
        default:
            printf("Invalid y-scale Setting - Default Selected\n");
            break;
    }
    return options->yScale;
}


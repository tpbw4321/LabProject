//
//  usbcomm.c
//  libusb
//
//  Created by Barron Wong on 12/1/17.
//  Copyright © 2017 libusb. All rights reserved.
//

#include "usbcomm.h"
//Setups up Device Handle using  Vendor and Device ID
struct libusb_device_handle * SetupDevHandle(const int venId, const int devId ){
    
    static libusb_device_handle * dev = NULL;
    
    libusb_init(NULL); // Initialize the LIBUSB library
    
    dev = libusb_open_device_with_vid_pid(NULL, venId, devId);
    
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
    
    return dev;
}

//Allocates and Sets up a isochronous transfer
struct libusb_transfer * SetupIsoTransfer(libusb_device_handle * dev, int endPoint, unsigned char * dataBuffer, int numOfPackets, libusb_transfer_cb_fn CallBack, void * dataPtr ){
    
    static struct libusb_transfer * iso = NULL;
    
    //Allocate Iso Transfer
    iso = libusb_alloc_transfer(numOfPackets);
    if(!iso){
        perror("ISO Setup");
        return NULL;
    }
    
    int packetSize = libusb_get_max_iso_packet_size(libusb_get_device(dev), endPoint);
    
    libusb_fill_iso_transfer(
                             iso,             //Transfer Handle
                             dev,             //Device Handle
                             endPoint,        //Incoming EndPoint
                             dataBuffer,      //Buffer
                             packetSize,      //Transfer Count
                             1,               //Number of packets
                             CallBack,        //Callback Function
                             dataPtr,         //Data pointer
                             0);              //Timeout
    
    libusb_set_iso_packet_lengths(iso, packetSize);
    
    return iso;
}

//Grabs Packet from transfer returns number of bytes recieved
int PacketTransfer(libusb_device_handle * dev, struct libusb_transfer * iso, int endPoint ,unsigned char * dataBuffer, int * isoDataPtr, enum libusb_transfer_type type ){
    
    libusb_device * libDev;
    int rcvd_bytes = 0;
    int packetSize;
    
    if(!dev)
        perror("No Device");
    if(!iso && type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
        perror("No Transfer");
    
    libDev = libusb_get_device(dev);
    packetSize = libusb_get_max_packet_size(libDev, endPoint);
    
    switch(type){
        case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
            if(libusb_submit_transfer(iso) != 0){
                perror("Submit");
                return 0;
            }
            while (!(*isoDataPtr)){
                libusb_handle_events_completed(NULL, NULL);
            }
            rcvd_bytes = *isoDataPtr;
            *isoDataPtr = 0;
            break;
        case LIBUSB_TRANSFER_TYPE_INTERRUPT:
            if(libusb_interrupt_transfer(dev, endPoint, dataBuffer, packetSize, &rcvd_bytes, 0) != 0)
                perror("Recieve Interrupt");
                break;
        case LIBUSB_TRANSFER_TYPE_BULK:
            if(libusb_bulk_transfer(dev, endPoint, dataBuffer, packetSize, &rcvd_bytes, 0)!= 0)
                perror("Recieve Bulk");
            break;
        default:
            printf("Not Supported Transfer");
            break;
            
    }
    
    return rcvd_bytes;
    
    
}
void CheckUSBError(enum libusb_error error){
    switch(error){
        case LIBUSB_ERROR_NO_DEVICE:
            perror("No Device");
            break;
        case LIBUSB_ERROR_BUSY:
            perror("Busy");
            break;
        case LIBUSB_ERROR_NOT_SUPPORTED:
            perror("Not Supported");
            break;
        case LIBUSB_ERROR_IO:
            perror("IO");
            break;
        case LIBUSB_ERROR_OTHER:
            perror("Other");
            break;
        case LIBUSB_ERROR_ACCESS:
            perror("ACCESS");
            break;
        case LIBUSB_ERROR_PIPE:
            perror("PIPE");
            break;
        case LIBUSB_ERROR_NO_MEM:
            perror("MEM");
            break;
        case LIBUSB_ERROR_TIMEOUT:
            perror("TIMEOUT");
            break;
        case LIBUSB_ERROR_OVERFLOW:
            perror("OVERFLOW");
            break;
        case LIBUSB_ERROR_NOT_FOUND:
            perror("NOT FOUND");
            break;
        case LIBUSB_ERROR_INTERRUPTED:
            perror("INTERRUPTED");
            break;
        case LIBUSB_ERROR_INVALID_PARAM:
            perror("INVALID PARAM");
            break;
        case LIBUSB_SUCCESS:
            perror("SUCCESS");
            break;
        default:
            perror("Something Else");
            break;
    }
}

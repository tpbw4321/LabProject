//
//  usbcomm.h
//  libusb
//
//  Created by Barron Wong on 12/1/17.
//  Copyright © 2017 libusb. All rights reserved.
//

#ifndef usbcomm_h
#define usbcomm_h
#include "libusb.h"
#include <stdio.h>


//Setups up Device Handle with the PSOC
struct libusb_device_handle * SetupDevHandle(const int venId, const int devId);

//Sets up a isochronous transfer
struct libusb_transfer * SetupIsoTransfer(libusb_device_handle * dev, int endPoint, unsigned char * dataBuffer, int numOfPackets, libusb_transfer_cb_fn callback, void * dataPtr );

//Grabs Iso Packet from transfer
int PacketTransfer(libusb_device_handle * dev, struct libusb_transfer * iso, int endPoint ,unsigned char * dataBuffer, int * dataPtr, enum libusb_transfer_type type );

void CheckUSBError(enum libusb_error error);

#endif /* usbcomm_h */

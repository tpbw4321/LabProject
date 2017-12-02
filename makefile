All:
	gcc -I /opt/vc/include -I /usr/include -I /usr/include/libusb-1.0/ -lusb-1.0 -lshapes -lm -std=c99 -o LabProject LabProject.c queue.c usbcomm.c


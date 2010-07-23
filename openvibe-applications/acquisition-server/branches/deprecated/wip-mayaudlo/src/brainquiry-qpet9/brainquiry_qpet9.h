//
// C++ Interface: brainquiry_qpet9
//
// Description: 
//
//
// Author: Louis MAYAUD <louis.mayaud@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BRAINQUIRYBRAINQUIRY_QPET9_H
#define BRAINQUIRYBRAINQUIRY_QPET9_H

// Bainclinics QPET-9 commands
#define STOP 0x30
#define START 0x31
#define SETUP 0x32
#define STATUS 0x33
#define EXTERNE_FLASH 0x35
#define SHUTDOWN 0x36
#define FEEDBACK 0x37
#define BLETOOTH_ADDRESS 0x38
#define EMG_SETTINGS 0x39
#define PARAMETERS 0x70

// UART Messages
#define DLE 0x10 // Data link escape
#define STX 0x02 // Packet Start
#define ETX 0x03
#define stxA 0x40
#define stxB0 0x41
#define stxB1 0x81
#define stxC0 0x42
#define stxD0 0x43
#define stxD1 0x83





namespace brainquiry {

/**
	@author Louis MAYAUD <louis.mayaud@gmail.com>
*/
class brainquiry_qpet9{
public:
    brainquiry_qpet9();

    ~brainquiry_qpet9();

};

}

#endif

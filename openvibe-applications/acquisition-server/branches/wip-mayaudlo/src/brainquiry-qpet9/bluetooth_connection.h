//
// C++ Interface: bluetooth_connection
//
// Description: 
//
//
// Author: Louis MAYAUD <louis.mayaud@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>


#ifndef BRAINQUIRYBLUETOOTH_CONNECTION_H
#define BRAINQUIRYBLUETOOTH_CONNECTION_H

#define MAX_RSP 255

namespace brainquiry {

/**
	@author Louis MAYAUD <louis.mayaud@gmail.com>
*/
class CbluetoothConnection{

private:
	inquiry_info *ii ;	
	int i16DeviceId, i16Socket, i16Length;
	char sBluetoothAddress[19] ;
	char sBluetoothName[248] ;



public:
    CbluetoothConnection();

    ~CbluetoothConnection();

};

}

#endif

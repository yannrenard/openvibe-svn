//
// C++ Implementation: CbluetoothConnection
//
// Description: 
//
//
// Author: Louis MAYAUD <louis.mayaud@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//

//
#include "CbluetoothConnection.h"

namespace brainquiry {

CbluetoothConnection::CbluetoothConnection()
{
	// Initialize varibale values
	ii = NULL;	
	sBluetoothAddress = { 0 };
	sBluetoothName = { 0 };

}


CbluetoothConnection::~CbluetoothConnection()
{
	// Free allocated memory 
	free( ii );
	// and close connections
    	close( i16Socket );
}


CbluetoothConnection::initializeRessources()
{
	// retrieve the resource number of the first available Bluetooth adapter.		
	i16DeviceId = hci_get_route(NULL);
	// opens a Bluetooth socket with the specified resource number
	i16Socket = hci_open_dev( dev_id );
		//TODO test socket value and return appropriate error	
}

CbluetoothConnection::selectDevice()
{
	
	// Check avaible Bluetooth devices
	int i16Length  = 8; //TODO check that parameters and replace that variable
	ii = (inquiry_info*)malloc(MAX_RSP * sizeof(inquiry_info));
	int i16ResponseNumber = hci_inquiry(dev_id, len, MAX_RSP, NULL, &ii, IREQ_CACHE_FLUSH);
		//TODO check num_rsp and return error if necessary

	// Browse them and select the one
	for ( int i = 0 ; i < i16ResponseNumber ; i++) {
		char l_BluetoothName[248] ;	
		memset( l_BluetoothName, 0 , sizeof(l_BluetoothName) );
		// TODO check value of line upon
		hci_read_remote_name( i16Socket , &(ii+i)->bdaddr, sizeof(l_BluetoothName), l_BluetoothName, 0) ;
		//TODO check outside
		if ( l_BluetoothName == "BrainQuiryQPET9" ) //TODO get the true value  
			{
			ba2str( &(ii+i)->bdaddr, sBluetoothAddress );
			sBluetoothName = l_BluetoothName ;
			}
   	}

	// Select device
	

}


}

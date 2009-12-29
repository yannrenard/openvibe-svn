// author: Guillaume Gibert
// affiliation: INSERM U821
// date: 10/12/2008




#ifndef __VrpnListener_H__
#define __VrpnListener_H__

#include <vrpn/vrpn_Analog.h>
#include <iostream>
#include <string>

class VrpnListener
{
	private :
			
	public :
		VrpnListener();
		~VrpnListener();
		vrpn_Analog_Remote* m_pAnalog;
		std::string m_sDeviceName;
		int result;
	
	protected :
			
	public:
		void init();
		void setDeviceName(std::string sDeviceName);
		void process();
		
};

#endif //__VrpnListener_H__

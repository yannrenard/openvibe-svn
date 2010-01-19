#ifndef __OpenViBEApplication_CAbstractVrpnPeripheric_H__
#define __OpenViBEApplication_CAbstractVrpnPeripheric_H__

class CDeviceInfo;

//#include "ovavrdCOgreVRApplication.h"


#include <list>

namespace OpenViBEVRDemos 
{

	class CAbstractVrpnPeripheric
	{
	public:

		//std::list < COgreVRApplication* > m_vpApplication;
		std::list < std::pair < int, int > > m_vButton;
		std::list < std::list < double > > m_vAnalog;
		double m_dAnalogScale;
		double m_dAnalogOffset;

		std::string m_sDeviceName;
		CDeviceInfo* m_pDevice;

		CAbstractVrpnPeripheric(void);
		CAbstractVrpnPeripheric(const std::string deviceName);
		virtual ~CAbstractVrpnPeripheric(void);

		virtual void init(void);
		virtual void loop(void);
		
		inline void setDeviceName(const std::string deviceName) { m_sDeviceName = deviceName;}
		
		//virtual void registerApplication(COgreVRApplication* application);
	

		


	};
};
#endif

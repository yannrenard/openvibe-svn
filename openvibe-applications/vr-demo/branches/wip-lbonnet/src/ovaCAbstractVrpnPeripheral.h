#ifndef __OpenViBEApplication_CAbstractVrpnPeripheral_H__
#define __OpenViBEApplication_CAbstractVrpnPeripheral_H__

class CDeviceInfo;

#include <list>
#include <string>
#include <map>

namespace OpenViBEVRDemos 
{

	class CAbstractVrpnPeripheral
	{
	public:

		std::list < std::pair < int, int > > m_vButton; // list of pairs(button_id, button_state)
		std::list < std::list < double > > m_vAnalog;
		double m_dAnalogScale;
		double m_dAnalogOffset;

		std::string m_sDeviceAddress;
		CDeviceInfo* m_pDevice;

		CAbstractVrpnPeripheral(void);
		CAbstractVrpnPeripheral(const std::string deviceAddress);
		virtual ~CAbstractVrpnPeripheral(void);

		virtual void init(void);
		virtual void loop(void);
			
	};
};
#endif

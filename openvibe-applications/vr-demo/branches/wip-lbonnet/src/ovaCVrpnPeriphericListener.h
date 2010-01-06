#ifndef __OpenViBEApplication_CVrpnPeriphericListener_H__
#define __OpenViBEApplication_CVrpnPeriphericListener_H__

#include "ovaCAbstractVrpnPeripheric.h"

namespace OpenViBEVRDemos 
{
	class CVrpnPeriphericListener : public CAbstractVrpnPeripheric
	{
	public:
		virtual void loop(void);
	};
};

#endif // __OpenViBEApplication_CVrpnPeriphericListener_H__

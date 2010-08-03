#if defined TARGET_HAS_ThirdPartyEmotivAPI

#include "ovasCConfigurationEmotivEPOC.h"



using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;

#define boolean OpenViBE::boolean

//____________________________________________________________________________________

CConfigurationEmotivEPOC::CConfigurationEmotivEPOC(IDriverContext& rDriverContext, const char* sGladeXMLFileName)
	:CConfigurationGlade(sGladeXMLFileName)
	,m_rDriverContext(rDriverContext)
{
}

boolean CConfigurationEmotivEPOC::preConfigure(void)
{
	if(! CConfigurationGlade::preConfigure())
	{
		return false;
	}

	return true;
}

boolean CConfigurationEmotivEPOC::postConfigure(void)
{

	if(m_bApplyConfiguration)
	{
		
	}

	if(! CConfigurationGlade::postConfigure()) // normal header is filled, ressources are realesed
	{
		return false;
	}

	return true;
}

#endif // TARGET_HAS_ThirdPartyEmotivAPI

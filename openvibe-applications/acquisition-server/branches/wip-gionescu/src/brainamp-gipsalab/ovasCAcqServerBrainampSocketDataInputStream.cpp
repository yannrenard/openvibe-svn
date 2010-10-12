#include "ovasCAcqServerBrainampSocketDataInputStream.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

CAcqServerBrainampSocketDataInputStream::CAcqServerBrainampSocketDataInputStream(OpenViBE::CString& sServerHostName, OpenViBE::uint32& ui32ServerHostPort)
	: CAcqServerSocketDataInputStream(sServerHostName, ui32ServerHostPort)
{
}

CAcqServerBrainampSocketDataInputStream::~CAcqServerBrainampSocketDataInputStream(void)
{
}

OpenViBE::boolean CAcqServerBrainampSocketDataInputStream::read()
{
	DEFINE_GUID(GUID_RDAHeader,
		1129858446, 51606, 19590, char(175), char(74), char(152), char(187), char(246), char(201), char(20), char(80)
	);

	RDA_MessageHeader* l_pRDA_MessageHeader = (RDA_MessageHeader*) getBuffer();
	// Initialize vars for reception
	if(!readBlock((const char*) l_pRDA_MessageHeader, sizeof(RDA_MessageHeader)))
		return false;

	// Check for correct header GUID.
	if(!COMPARE_GUID(l_pRDA_MessageHeader->guid, GUID_RDAHeader))
		return false;

	if(!readBlock((const char*) l_pRDA_MessageHeader + sizeof(RDA_MessageHeader), l_pRDA_MessageHeader->nSize - sizeof(RDA_MessageHeader)))
		return false;

	return true;
}

OpenViBE::boolean CAcqServerBrainampSocketDataInputStream::readInfo()
{
	return read();
}
#include <iostream>

#include "ovasCAcqServerEyelinkSocketDataInputStream.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

CAcqServerEyelinkSocketDataInputStream::CAcqServerEyelinkSocketDataInputStream(OpenViBE::CString& sServerHostName, OpenViBE::uint32& ui32ServerHostPort)
	: CAcqServerSocketDataInputStream(sServerHostName, ui32ServerHostPort)
{
}

CAcqServerEyelinkSocketDataInputStream::~CAcqServerEyelinkSocketDataInputStream(void)
{
}

OpenViBE::boolean CAcqServerEyelinkSocketDataInputStream::read()
{
	OpenViBE::uint32	l_blockSize;

	// Initialize vars for reception
	if(!readBlock(&l_blockSize, sizeof(l_blockSize)))
		return false;

	if(!readBlock(getBuffer(), l_blockSize))
		return false;

	return true;
}

OpenViBE::boolean CAcqServerEyelinkSocketDataInputStream::readInfo()
{
	return read();
}

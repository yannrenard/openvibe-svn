#include "ovasCAcqServerDataInputStreamAbstract.h"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;

CAcqServerDataInputStreamAbstract::CAcqServerDataInputStreamAbstract()
	: std::vector<OpenViBE::float32>(INPUT_BUFFER_SIZE)
{
}

CAcqServerDataInputStreamAbstract::~CAcqServerDataInputStreamAbstract(void)
{
	close();
}

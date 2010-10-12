#ifndef __OpenViBE_AcquisitionServer_CAcqServerDataInputStreamAbstract_H__
#define __OpenViBE_AcquisitionServer_CAcqServerDataInputStreamAbstract_H__

#include <openvibe/ov_all.h>

#include <vector>

namespace OpenViBEAcquisitionServer
{
	class CAcqServerDataInputStreamAbstract : public std::vector<OpenViBE::float32>
	{
	private:
		typedef enum
		{	INPUT_BUFFER_SIZE	= 20000,
		};


	protected:
		CAcqServerDataInputStreamAbstract();

	public:
		virtual ~CAcqServerDataInputStreamAbstract(void);

		void*						getBuffer() const	{	return (void*) &(*this)[0];				}
		virtual	OpenViBE::boolean	close();
		virtual	OpenViBE::boolean	readInfo();

		virtual	OpenViBE::boolean	open() = 0;
		virtual	OpenViBE::boolean	read() = 0;
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerDataInputStreamAbstract_H__

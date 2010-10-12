#ifndef __OpenViBE_AcquisitionServer_CAcqServerDataInputStreamAbstract_H__
#define __OpenViBE_AcquisitionServer_CAcqServerDataInputStreamAbstract_H__

#include <openvibe/ov_all.h>

#include <vector>

namespace OpenViBEAcquisitionServer
{
	/**
	 * \class CAcqServerDataInputStreamAbstract
	 * \author Gelu Ionescu (Gipsa-lab)
	 * \date 2010-10-01
	 * \brief Abstract class that declares the API for a generic input stream 
	 *
	 * The role of different drivers developed in the acquisition server is to
	 * transform home formatted data coming from various acquisition devices
	 * toward the Open-ViBE standard.
	 * 
	 * This goal of this class is to define the general framework from data coming
	 * on various channels (sockets, USB, etc.). Furthermore, it reserves enough room
	 * for the incoming chunk of data.
	 *
	 */
	class CAcqServerDataInputStreamAbstract : public std::vector<OpenViBE::float32>
	{
	private:
		typedef enum
		{	INPUT_BUFFER_SIZE	= 20000,
		};

	protected:
		/** \name Class constructors / destructors*/
		//@{

		/**
		 * \brief Default constructor
		 *
		 * \note As you can see, the constructor is protected
		 */
		CAcqServerDataInputStreamAbstract();

	public:
		virtual ~CAcqServerDataInputStreamAbstract(void);
		//@}

		/** \name General API that respect the \i open/read/close life cycle */
		//@{

		/**
		 * \brief Returns the address of the internal contiguous memory buffer
		 */
		void*						getBuffer() const	{	return (void*) &(*this)[0];				}
		
		/**
		 * \brief Pure virtual methods that shoud be overloaded by the concret classes
		 */
		virtual	OpenViBE::boolean	open()		= 0;			//< open the stream
		virtual	OpenViBE::boolean	read()		= 0;			//< reads data from the stream
		virtual	OpenViBE::boolean	close() { return true;	}	//< close the stream
		virtual	OpenViBE::boolean	readInfo()	= 0;			//< reads information, if any, from the input device
		//@}
	};
};

#endif // __OpenViBE_AcquisitionServer_CAcqServerDataInputStreamAbstract_H__

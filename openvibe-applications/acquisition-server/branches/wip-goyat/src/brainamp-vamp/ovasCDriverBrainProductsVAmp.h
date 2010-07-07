#ifndef __OpenViBE_AcquisitionServer_CDriverBrainProductsVAmp_H__
#define __OpenViBE_AcquisitionServer_CDriverBrainProductsVAmp_H__

#if defined TARGET_HAS_ThirdPartyUSBFirstAmpAPI

#include "../ovasIDriver.h"
#include "ovasCHeaderBrainProductsVAmp.h"
#include <openvibe/ov_all.h>

#include <windows.h>
#include <FirstAmp.h>

#include <vector>

namespace OpenViBEAcquisitionServer
{
	
	class BrainAmpDataMode
	{
	 public:
		virtual ~BrainAmpDataMode(){}
		virtual void* getPtr(){ return NULL;}
		virtual OpenViBE::uint32 lenght(){ return 0;}
		virtual OpenViBE::uint32 EEGCount(){ return 0;}
		virtual OpenViBE::float32 getEEG(int i){ return 0;}
		virtual OpenViBE::uint32 AuxCount(){ return 0;}
		virtual OpenViBE::float32 getAUX(int i){ return 0;}
		virtual OpenViBE::uint32 TrigCount(){ return 0;}
		virtual OpenViBE::uint32 getTrigger(){ return 0;}
	};
	
	/**
	 * \class CDriverBrainProductsVAmp
	 * \author Laurent Bonnet (INRIA)
	 * \date 16 nov 2009
	 * \erief The CDriverBrainProductsVAmp allows the acquisition server to acquire data from a USB-VAmp-16 amplifier (BrainProducts GmbH).
	 *
	 * The driver allows 2 different acquisition modes: normal (2kHz sampling frequency - max 16 electrodes)
	 * or fast (20kHz sampling frequency, 4 monopolar or differential channels).
	 * The driver uses a dedicated Header.
	 *
	 * \sa CHeaderBrainProductsVAmp
	 */
	class CDriverBrainProductsVAmp : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverBrainProductsVAmp(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverBrainProductsVAmp(void);
		virtual const char* getName(void);

		virtual OpenViBE::boolean initialize(
			const OpenViBE::uint32 ui32SampleCountPerSentBlock,
			OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);

		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);
		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void) { return &m_oHeader; }

	protected:

		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;

		OpenViBEAcquisitionServer::CHeaderBrainProductsVAmp m_oHeader;

		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;
		OpenViBE::uint32 m_ui32TotalSampleCount;
		OpenViBE::float32* m_pSample;

		std::vector<OpenViBE::uint32> m_vStimulationIdentifier;
		std::vector<OpenViBE::uint64> m_vStimulationDate;
		std::vector<OpenViBE::uint64> m_vStimulationSample;
	private:
		OpenViBE::boolean m_bFirstStart;
		
		OpenViBE::uint32 m_ui32EEGChannelCount;
		OpenViBE::uint32 m_ui32AuxChannelCount;
		OpenViBE::uint32 m_ui32TriggerChannelCount;
		
		BrainAmpDataMode *m_oAmpliData;
	};
	

	/*
		//! Device data type for FirstAmp 16 model 
		typedef struct {
			signed int Main[FA_MODEL_16_CHANNELS_MAIN]; //!< main channels data, 16 channels + REF channel 
			signed int Aux[FA_MODEL_16_CHANNELS_AUX]; //!< auxiliary channels data, 2 channels 
			unsigned int Status; //!< Digital inputs (bits 0 - 8) + output (bit 9) state + 22 MSB reserved bits 
			unsigned int Counter; //!< 32-bit data sequencing cyclic counter 
		} t_faDataModel16;
	*/
	
	class BrainAmpDataMode8:public BrainAmpDataMode
	{
	 public:
		virtual ~BrainAmpDataMode8(){}
		virtual void* getPtr(){ return &data;}
		virtual OpenViBE::uint32 lenght(){ return sizeof(t_faDataModel8);}
		virtual OpenViBE::uint32 EEGCount(){ return 8;}
		virtual OpenViBE::float32 getEEG(int i){ return OpenViBE::float32(data.Main[i]);}
		virtual OpenViBE::uint32 AuxCount(){ return 2;}
		virtual OpenViBE::float32 getAUX(int i){ return OpenViBE::float32(data.Aux[i]);}
		virtual OpenViBE::uint32 TrigCount(){ return 1;}
		virtual OpenViBE::uint32 getTrigger(){ return OpenViBE::uint32(data.Status);}
	 protected:
		t_faDataModel8 data;
	};
	
	class BrainAmpDataMode16:public BrainAmpDataMode
	{
	 public:
		virtual ~BrainAmpDataMode16(){}
		virtual void* getPtr(){ return &data;}
		virtual OpenViBE::uint32 lenght(){ return sizeof(t_faDataModel16);}
		virtual OpenViBE::uint32 EEGCount(){ return 16;}
		virtual OpenViBE::float32 getEEG(int i){ return OpenViBE::float32(data.Main[i]);}
		virtual OpenViBE::uint32 AuxCount(){ return 2;}
		virtual OpenViBE::float32 getAUX(int i){ return OpenViBE::float32(data.Aux[i]);}
		virtual OpenViBE::uint32 TrigCount(){ return 1;}
		virtual OpenViBE::uint32 getTrigger(){ return OpenViBE::uint32(data.Status);}
	 protected:
		t_faDataModel16 data;
	};
		
	class BrainAmpDataMode4:public BrainAmpDataMode
	{
	 public:
		virtual ~BrainAmpDataMode4(){}
		virtual void* getPtr(){ return &data;}
		virtual OpenViBE::uint32 lenght(){ return sizeof(t_faDataFormatMode20kHz);}
		virtual OpenViBE::uint32 EEGCount(){ return 4;}
		virtual OpenViBE::float32 getEEG(int i){ return OpenViBE::float32(data.Main[i]);}
		virtual OpenViBE::uint32 AuxCount(){ return 0;}
		virtual OpenViBE::float32 getAUX(int i){ return 0;}
		virtual OpenViBE::uint32 TrigCount(){ return 1;}
		virtual OpenViBE::uint32 getTrigger(){ return OpenViBE::uint32(data.Status);}
	 protected:
		t_faDataFormatMode20kHz data;
	};
	
};

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI

#endif // __OpenViBE_AcquisitionServer_CDriverBrainProductsVAmp_H__

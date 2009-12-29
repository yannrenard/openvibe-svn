
//#define OVP_ClassId_VRPNButtonClient OpenViBE::CIdentifier(0x56FD308A, 0xEAF594FE)
//#define OVP_ClassId_VRPNButtonClientDesc OpenViBE::CIdentifier(0xAAB673FC, 0x947FC05D)
#ifndef __OpenViBE_Plugins_VRPN_CVRPNButtonClient_H__
#define __OpenViBE_Plugins_VRPN_CVRPNButtonClient_H__

#include <vrpn_Connection.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <ovp_global_defines.h>

#include <list>
#include <utility>


namespace OpenViBEPlugins
{
	namespace VRPN
	{

		class CVRPNDeviceInfo
		{
			 public :
				CVRPNDeviceInfo(OpenViBE::CString& name, char* cName);
				~CVRPNDeviceInfo();
				OpenViBE::CString m_name;
				vrpn_Button_Remote *btn;
		};

		class CVRPNButtonClient: public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{

			private :
				 CVRPNDeviceInfo* m_pDeviceInfo;


			protected:

				OpenViBE::Kernel::IAlgorithmProxy* m_pStreamEncoder;
				OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > m_oMemoryBufferHandle;

				OpenViBE::uint64 m_ui64StartTime;
				OpenViBE::uint64 m_ui64EndTime;

				std::list< std::pair<OpenViBE::uint32, OpenViBE::uint32> > m_lButtons;
				OpenViBE::uint32 m_ui32StateButton;
				OpenViBE::boolean m_bIsWaiting; 

				OpenViBE::boolean m_bHasSentHeader;
				OpenViBE::CStimulationSet m_oStimulationSet;
                
			public:
				virtual void release(void)
				{
					delete this;
				}
				virtual OpenViBE::uint64 getClockFrequency(void) { return 1LL<<38; }

				virtual OpenViBE::boolean initialize(void);
				virtual OpenViBE::boolean uninitialize(void);
				OpenViBE::boolean addDevice(OpenViBE::CString& name);
				virtual OpenViBE::boolean processClock(OpenViBE::Kernel::IMessageClock& rMessageClock);
				virtual OpenViBE::boolean process(void);
				void handleButton(OpenViBE::uint32 nbButton, OpenViBE::boolean stateButton);

				_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_VRPNButtonClient)
		};

		//this is the callback for our button client	
		void VRPN_CALLBACK g_handleButton(
			void *userdata,
			const vrpn_BUTTONCB b);


		class CVRPNButtonClientDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
			public:
				virtual OpenViBE::CString getName(void) const               { return OpenViBE::CString("Button VRPN Client"); }
				virtual OpenViBE::CString getAuthorName(void) const         { return OpenViBE::CString("Jean-Baptiste Sauvan"); }
				virtual OpenViBE::CString getAuthorCompanyName(void) const  { return OpenViBE::CString("INRIA/IRISA"); }
				virtual OpenViBE::CString getShortDescription(void) const   { return OpenViBE::CString("Button VRPN Client (connected to one server)"); }
				virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("VRPN Client which tries to connect itself to one server and get button signals"); }
				virtual OpenViBE::CString getCategory(void) const           { return OpenViBE::CString("VRPN"); }
				virtual OpenViBE::CString getVersion(void) const            { return OpenViBE::CString("1.0"); }
				virtual void release(void) 
				{
				}
				virtual OpenViBE::CIdentifier getCreatedClass(void) const
				{
					return OVP_ClassId_VRPNButtonClient;
				}
				virtual OpenViBE::Plugins::IPluginObject* create(void)
				{
					return new OpenViBEPlugins::VRPN::CVRPNButtonClient();
				}
				virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
				{
					//add output
					rPrototype.addOutput("Stimulation", OV_TypeId_Stimulations);

					//add settings
					rPrototype.addSetting("VRPN server name", OV_TypeId_String, "appliRV@localhost:3883");
					return true; 
				}
				_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_VRPNButtonClientDesc )
		};
	};
};	

#endif //__OpenViBE_Plugins_VRPN_CVRPNButtonClient_H__

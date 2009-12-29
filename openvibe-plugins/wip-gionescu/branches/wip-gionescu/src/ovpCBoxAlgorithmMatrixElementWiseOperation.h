#ifndef __OpenViBEPlugins_BoxAlgorithm_MatrixElementWiseOperation_H__
#define __OpenViBEPlugins_BoxAlgorithm_MatrixElementWiseOperation_H__

#include <sstream>                            // istringstream, ostringstream

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "ovpCAlgorithmMatrixElementWiseOperation.h"

// TODO:
// - please move the identifier definitions in ovp_defines.h
// - please include your desciptor in ovp_main.cpp

// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x125C4A4A, 0x22C7144F)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x36B632E4, 0x10D10060)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x48787A30, 0x264D766B)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x76304C1F, 0x5A054546)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x672368F0, 0x153509FA)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x6FF87A70, 0x461A3D66)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x662700F3, 0x0F407F92)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x3E1F1410, 0x1F893CD4)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x603A7320, 0x7EFC08A5)
// #define OVP_/*TODO*/ OpenViBE::CIdentifier(0x60612176, 0x5C534A40)

namespace OpenViBEPlugins
{
	namespace WipGionescu
	{
		class CBoxAlgorithmMatrixElementWiseOperation : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			typedef enum
			{	NB_ENTRIES = 3,
			} misc_type;

			CBoxAlgorithmMatrixElementWiseOperation(void);

			virtual void release(void) { delete this; }

			// virtual OpenViBE::uint64 getClockFrequency(void);
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			// virtual OpenViBE::boolean processEvent(OpenViBE::CMessageEvent& rMessageEvent);
			// virtual OpenViBE::boolean processSignal(OpenViBE::CMessageSignal& rMessageSignal);
			// virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_MatrixElementWiseOperation);

		protected:
            OpenViBE::boolean   IsValid(const std::vector<OpenViBE::IMatrix*>& matrixes,
                                        const std::vector<OpenViBE::uint64>& chunkStartTime,
                                        const std::vector<OpenViBE::uint64>& chunkEndTime);
        
		protected:
			OpenViBE::Kernel::IAlgorithmProxy*				m_pAlgorithmMatrixElementWiseOperation;
			
			std::vector<OpenViBE::Kernel::IAlgorithmProxy*>	m_vecInStreamDecoders;
			OpenViBE::Kernel::IAlgorithmProxy*				m_pOutStreamEncoder;
			
	        OpenViBE::CString                               m_grammar;
	        std::vector<OpenViBE::IMatrix*>                 m_matrixes;
		};

		class CBoxAlgorithmMatrixElementWiseOperationDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("MatrixElementWiseOperation"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("gelu ionescu"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Gipsa-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("matrix operation box"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("matrix operation box"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("MathTools"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-missing-image"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_MatrixElementWiseOperation; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::WipGionescu::CBoxAlgorithmMatrixElementWiseOperation; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				for(int ii=0; ii < CBoxAlgorithmMatrixElementWiseOperation::NB_ENTRIES; ii++)
				{	std::ostringstream os;
					os << "Matrix " << ii;
					rBoxAlgorithmPrototype.addInput  (os.str().c_str(), OV_TypeId_StreamedMatrix);
				}
				
				rBoxAlgorithmPrototype.addOutput ("Result matrix", OV_TypeId_StreamedMatrix);
				
				rBoxAlgorithmPrototype.addSetting("Formula", OV_TypeId_String, "(m0 + m1)/m2");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MatrixElementWiseOperationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_MatrixElementWiseOperation_H__

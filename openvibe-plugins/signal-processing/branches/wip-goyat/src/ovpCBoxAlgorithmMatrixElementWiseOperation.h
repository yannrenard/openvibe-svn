#ifndef __OpenViBEPlugins_BoxAlgorithm_MatrixElementWiseOperation_H__
#define __OpenViBEPlugins_BoxAlgorithm_MatrixElementWiseOperation_H__

#include <sstream>                            // istringstream, ostringstream

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include "ovpCAlgorithmMatrixElementWiseOperation.h"

namespace OpenViBEPlugins
{
	namespace WipGionescu
	{
		class CBoxAlgorithmMatrixElementWiseOperation : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			CBoxAlgorithmMatrixElementWiseOperation(void);
			virtual void release(void);

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_MatrixElementWiseOperation);

		protected:
            OpenViBE::boolean   honorDimensionAndSize(const std::vector<OpenViBE::IMatrix*>& matrixes,
                                        const std::vector<OpenViBE::uint64>& chunkStartTime,
                                        const std::vector<OpenViBE::uint64>& chunkEndTime);
        
			OpenViBE::CString	preFilter(OpenViBE::CString&);
		protected:
			
			//decoders
			std::vector<OpenViBE::Kernel::IAlgorithmProxy*>	m_vecInStreamDecoders;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > op_ui64SamplingRate;
			//algo
			OpenViBE::Kernel::IAlgorithmProxy*				m_pAlgorithmMatrixElementWiseOperation;
			OpenViBE::CString                   m_sgrammar;
	        std::vector<OpenViBE::IMatrix*>     m_pmatrixes;
			CMatrixElementWiseOperation*		m_pparser;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pAlgorithmMatrixOutput;
			//encoder
			OpenViBE::Kernel::IAlgorithmProxy*				m_pOutStreamEncoder;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pMatrixToEncode;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_ui64SamplingRate;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pBuffer;
		};

		class CBoxAlgorithmMatrixElementWiseOperationListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			OpenViBE::boolean check(OpenViBE::Kernel::IBox& rBox)
			{
				char l_sName[1024];
				OpenViBE::uint32 i;

				for(i=0; i<rBox.getInputCount(); i++)
				{
					sprintf(l_sName, "Input matrix m%u", i);
					rBox.setInputName(i, l_sName);
					rBox.setInputType(i, OV_TypeId_StreamedMatrix);
				}

				return true;
			}

			virtual OpenViBE::boolean onInputRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return this->check(rBox); }
			virtual OpenViBE::boolean onInputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return this->check(rBox); };

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};
		
		class CBoxAlgorithmMatrixElementWiseOperationDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("MatrixElementWiseOperation"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("gelu ionescu + Matthieu Goyat"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Gipsa-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("matrix operation box"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("matrix operation box"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/MathTools"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-missing-image"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_MatrixElementWiseOperation; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::WipGionescu::CBoxAlgorithmMatrixElementWiseOperation; }
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmMatrixElementWiseOperationListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Matrix m0", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addInput  ("Matrix m1", OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_CanAddInput);
				//
				rBoxAlgorithmPrototype.addOutput ("Result matrix", OV_TypeId_Signal);
				//
				rBoxAlgorithmPrototype.addSetting("Formula", OV_TypeId_String, "(m0 + m1)");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_MatrixElementWiseOperationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_MatrixElementWiseOperation_H__

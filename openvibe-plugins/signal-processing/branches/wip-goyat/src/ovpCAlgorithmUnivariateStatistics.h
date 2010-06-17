#ifndef __OpenViBEPlugins_UnivariateStatistics_H__
#define __OpenViBEPlugins_UnivariateStatistics_H__

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include <vector>

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CAlgoUnivariateStatistic : public OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVP_ClassId_AlgoUnivariateStatistic);

		protected:

			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > ip_pMatrix;
			//OpenViBE::Kernel::TParameterHandler < OpenViBE::uint64 > ip_uiStatisticMode;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pProcessedMatrix_Mean;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pProcessedMatrix_Variance;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pProcessedMatrix_Range;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pProcessedMatrix_Median;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pProcessedMatrix_IQR;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > op_pProcessedMatrix_Percentile;
			
			OpenViBE::Kernel::TParameterHandler < OpenViBE::boolean > ip_pStatisticMeanActive;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::boolean > ip_pStatisticVarianceActive;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::boolean > ip_pStatisticRangeActive;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::boolean > ip_pStatisticMedianActive;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::boolean > ip_pStatisticIQRActive;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::boolean > ip_pStatisticPercentileActive;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::uint32 >  ip_iPercentileValue;
			
			OpenViBE::Kernel::TParameterHandler < OpenViBE::float64 > op_fCompression;

			OpenViBE::boolean m_bsumActive;
			OpenViBE::CMatrix sum_mat;
			OpenViBE::boolean m_bsqaresumActive;
			OpenViBE::CMatrix sum2_mat;
			OpenViBE::boolean m_bsortActive;
			OpenViBE::CMatrix sort_mat;
			
			OpenViBE::uint32 m_uiPercentileValue;
			
			void setMatDim(OpenViBE::IMatrix* iomat, OpenViBE::IMatrix* ref);
			
		};

		class CAlgoUnivariateStatisticDesc : public OpenViBE::Plugins::IAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Signal Statistic"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Matthieu Goyat"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Gipsa-lab"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Statistics"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_AlgoUnivariateStatistic; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CAlgoUnivariateStatistic(); }

			virtual OpenViBE::boolean getAlgorithmPrototype(
				OpenViBE::Kernel::IAlgorithmProto& rAlgorithmProto) const
			{
				rAlgorithmProto.addInputParameter (OVP_Algorithm_UnivariateStatistic_InputParameter, "Matrix input", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addInputParameter (OVP_Algorithm_UnivariateStatistic_InputParameter_MeanActive, "active mean", OpenViBE::Kernel::ParameterType_Boolean );
				rAlgorithmProto.addInputParameter (OVP_Algorithm_UnivariateStatistic_InputParameter_VarActive, "active variance", OpenViBE::Kernel::ParameterType_Boolean );
				rAlgorithmProto.addInputParameter (OVP_Algorithm_UnivariateStatistic_InputParameter_RangeActive, "active range", OpenViBE::Kernel::ParameterType_Boolean );
				rAlgorithmProto.addInputParameter (OVP_Algorithm_UnivariateStatistic_InputParameter_MedActive, "active median", OpenViBE::Kernel::ParameterType_Boolean );
				rAlgorithmProto.addInputParameter (OVP_Algorithm_UnivariateStatistic_InputParameter_IQRActive, "active IQR", OpenViBE::Kernel::ParameterType_Boolean );
				rAlgorithmProto.addInputParameter (OVP_Algorithm_UnivariateStatistic_InputParameter_PercentActive, "active Percentile", OpenViBE::Kernel::ParameterType_Boolean );
				rAlgorithmProto.addInputParameter (OVP_Algorithm_UnivariateStatistic_InputParameter_PercentValue, "Percentile Value", OpenViBE::Kernel::ParameterType_Integer );
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Mean, "Mean output", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Var, "Variance output", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Range, "Range output", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Med, "Median output", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_IQR, "Inter-Quantile-Range output", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Percent, "Percentile output", OpenViBE::Kernel::ParameterType_Matrix);
				rAlgorithmProto.addOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Compression, "compression ratio", OpenViBE::Kernel::ParameterType_Float );
				
				rAlgorithmProto.addInputTrigger   (OVP_Algorithm_UnivariateStatistic_InputTriggerId_Initialize, "Initialize");
				rAlgorithmProto.addInputTrigger   (OVP_Algorithm_UnivariateStatistic_InputTriggerId_Process, "Process");
				rAlgorithmProto.addOutputTrigger  (OVP_Algorithm_UnivariateStatistic_OutputTriggerId_ProcessDone, "Done");
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IAlgorithmDesc, OVP_ClassId_AlgoUnivariateStatisticDesc);
		};
	};
};

#endif // __OpenViBEPlugins_SignalProcessing_Algorithms_Basic_CSignalStatistic_H__

#include "ovpCAlgorithmUnivariateStatistics.h"

#include <cmath>
#include <iostream>
#include <algorithm>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

// ________________________________________________________________________________________________________________
//

boolean CAlgoUnivariateStatistic::initialize(void)
{
	ip_pMatrix.initialize(getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter));
	op_pProcessedMatrix_Mean.initialize(getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Mean));
	op_pProcessedMatrix_Variance.initialize(getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Var));
	op_pProcessedMatrix_Range.initialize(getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Range));
	op_pProcessedMatrix_Median.initialize(getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Med));
	op_pProcessedMatrix_IQR.initialize(getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_IQR));
	op_pProcessedMatrix_Percentile.initialize(getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Percent));
				
	ip_pStatisticMeanActive.initialize(this->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_MeanActive));
	ip_pStatisticVarianceActive.initialize(this->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_VarActive));
	ip_pStatisticRangeActive.initialize(this->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_RangeActive));
	ip_pStatisticMedianActive.initialize(this->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_MedActive));
	ip_pStatisticIQRActive.initialize(this->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_IQRActive));
	ip_pStatisticPercentileActive.initialize(this->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_PercentActive));
	
	ip_iPercentileValue.initialize(this->getInputParameter(OVP_Algorithm_UnivariateStatistic_InputParameter_PercentValue));
	op_fCompression.initialize(this->getOutputParameter(OVP_Algorithm_UnivariateStatistic_OutputParameter_Compression));
	
	return true;
}

boolean CAlgoUnivariateStatistic::uninitialize(void)
{
	op_fCompression.uninitialize();
	ip_iPercentileValue.uninitialize();
	
	ip_pStatisticMeanActive.uninitialize();
	ip_pStatisticVarianceActive.uninitialize();
	ip_pStatisticRangeActive.uninitialize();
	ip_pStatisticMedianActive.uninitialize();
	ip_pStatisticIQRActive.uninitialize();
	ip_pStatisticPercentileActive.uninitialize();
	
	op_pProcessedMatrix_Mean.uninitialize();
	op_pProcessedMatrix_Variance.uninitialize();
	op_pProcessedMatrix_Range.uninitialize();
	op_pProcessedMatrix_Median.uninitialize();
	op_pProcessedMatrix_IQR.uninitialize();
	op_pProcessedMatrix_Percentile.uninitialize();
	ip_pMatrix.uninitialize();

	return true;
}

// ________________________________________________________________________________________________________________
//

boolean CAlgoUnivariateStatistic::process(void)
{
// std::cout<<"process algo"<<std::endl;

	IMatrix* l_pInputMatrix=ip_pMatrix;
	IMatrix* l_pOutputMatrix_Mean=op_pProcessedMatrix_Mean;
	IMatrix* l_pOutputMatrix_Variance=op_pProcessedMatrix_Variance;
	IMatrix* l_pOutputMatrix_Range=op_pProcessedMatrix_Range;
	IMatrix* l_pOutputMatrix_Median=op_pProcessedMatrix_Median;
	IMatrix* l_pOutputMatrix_IQR=op_pProcessedMatrix_IQR;
	IMatrix* l_pOutputMatrix_Percentile=op_pProcessedMatrix_Percentile;
	
	if(this->isInputTriggerActive(OVP_Algorithm_UnivariateStatistic_InputTriggerId_Initialize))
	{
		std::cout<<"input : "<<l_pInputMatrix->getDimensionCount()<<" : "<<l_pInputMatrix->getDimensionSize(0)<<"*"<<l_pInputMatrix->getDimensionSize(1)<<std::endl;

		setMatDim(l_pOutputMatrix_Mean,l_pInputMatrix);
		setMatDim(l_pOutputMatrix_Variance,l_pInputMatrix);
		setMatDim(l_pOutputMatrix_Range,l_pInputMatrix);
		setMatDim(l_pOutputMatrix_Median,l_pInputMatrix);
		setMatDim(l_pOutputMatrix_IQR,l_pInputMatrix);
		setMatDim(l_pOutputMatrix_Percentile,l_pInputMatrix);
		
		op_fCompression=1/float64(l_pInputMatrix->getDimensionSize(1));
		m_uiPercentileValue=ip_iPercentileValue;
		
		m_bsumActive=ip_pStatisticMeanActive || ip_pStatisticVarianceActive;
		m_bsqaresumActive=ip_pStatisticVarianceActive;
		m_bsortActive=ip_pStatisticRangeActive || ip_pStatisticMedianActive || ip_pStatisticIQRActive || ip_pStatisticPercentileActive;
		
		if(m_bsumActive)
		  {
			OpenViBEToolkit::Tools::Matrix::copyDescription(sum_mat, *l_pInputMatrix); 
			sum_mat.setDimensionSize(1,1);
		  }
		  
		if(m_bsqaresumActive)
		  {
			OpenViBEToolkit::Tools::Matrix::copyDescription(sum2_mat, *l_pInputMatrix); 
			sum2_mat.setDimensionSize(1,1);
		  }

		if(m_bsortActive)
		  {
			OpenViBEToolkit::Tools::Matrix::copyDescription(sort_mat, *l_pInputMatrix); 
		  }

	}

	if(this->isInputTriggerActive(OVP_Algorithm_UnivariateStatistic_InputTriggerId_Process))
	{
		// std::cout<<"process algo process"<<std::endl;

		///faire les opérations nécessaies aux calculs
		float64 l_fS=float64(l_pInputMatrix->getDimensionSize(1));
		std::vector<float64> vect(l_pInputMatrix->getDimensionSize(1));
		for(uint32 i=0; i<l_pInputMatrix->getDimensionSize(0);i++)
		  {
			if(m_bsortActive)
			  {
				//copy fonctionne pas car le buffer n'est pas unidirectionnel...
				for(uint32 j = 0; j < l_pInputMatrix->getDimensionSize(1); j++)
				  {vect[j]=l_pInputMatrix->getBuffer()[i*l_pInputMatrix->getDimensionSize(1)+j];}
				std::sort(vect.begin(),vect.end());
			  }

			float64 l_fY=0, l_fY2=0;
				for(uint32 j = 0; j < l_pInputMatrix->getDimensionSize(1); j++)
				  {
					float64 l_fX=l_pInputMatrix->getBuffer()[i*l_pInputMatrix->getDimensionSize(1)+j];
					if(m_bsumActive)
					  {l_fY+=l_fX;}
					if(m_bsqaresumActive)
					  {l_fY2+=l_fX*l_fX;}
					if(m_bsortActive)
					  {sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+j]=vect.at(j);}
				  }
				
				if(m_bsumActive)
				  {sum_mat.getBuffer()[i*sum_mat.getDimensionSize(1)]=l_fY;}
				if(m_bsqaresumActive)
				  {sum2_mat.getBuffer()[i*sum2_mat.getDimensionSize(1)]=l_fY2;}
		  }
		
		///verification : 
		/*std::cout<<"somme = ";
		for(uint32 i=0; i<sum_mat.getDimensionSize(0);i++)
		  {std::cout<<sum_mat.getBuffer()[i*sum_mat.getDimensionSize(1)]<<",";}
		std::cout<<std::endl<<"somme2 = ";
		for(uint32 i=0; i<sum2_mat.getDimensionSize(0);i++)
		  {std::cout<<sum2_mat.getBuffer()[i*sum2_mat.getDimensionSize(1)]<<",";}		
		 std::cout<<std::endl<<"sort = ";
		for(uint32 i=0; i<sort_mat.getDimensionSize(0);i++)
		  {
			for(uint32 j=0; j<sort_mat.getDimensionSize(1);j++)
			  {std::cout<<sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+j]<<",";}
			std::cout<<std::endl;
		  }*/
		
		///
		if(ip_pStatisticMeanActive)
		{
			// std::cout<<"Mean"<<std::endl;
			for(uint32 i=0; i<l_pOutputMatrix_Mean->getDimensionSize(0);i++)
			  {
				l_pOutputMatrix_Mean->getBuffer()[i*l_pOutputMatrix_Mean->getDimensionSize(1)]=sum_mat.getBuffer()[i*sum_mat.getDimensionSize(1)]/l_pInputMatrix->getDimensionSize(1);
			  }
		}
		if(ip_pStatisticVarianceActive)
		{
			// std::cout<<"Variance"<<std::endl;
			for(uint32 i = 0; i < l_pOutputMatrix_Variance->getDimensionSize(0); i++)
			  {
				float64 l_fY=sum_mat.getBuffer()[i*sum_mat.getDimensionSize(1)], 
						l_fY2=sum2_mat.getBuffer()[i*sum2_mat.getDimensionSize(1)];				
				l_pOutputMatrix_Variance->getBuffer()[i*l_pOutputMatrix_Variance->getDimensionSize(1)]=l_fY2/l_fS-l_fY*l_fY/(l_fS*l_fS);
				// std::cout<<"cumul = "<<l_fY<<"|"<<l_fY2<<". Dimension = "<<l_fS<<". Final = "<<l_fY2/l_fS-l_fY*l_fY/(l_fS*l_fS)<<std::endl;
			  }
		}
		if(ip_pStatisticRangeActive)
		{
			// std::cout<<"Range"<<std::endl;
			for(uint32 i = 0; i < l_pOutputMatrix_Range->getDimensionSize(0); i++)
			  {
				float64 l_min=sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+0], 
						l_max=sort_mat.getBuffer()[(i+1)*sort_mat.getDimensionSize(1)-1];
				l_pOutputMatrix_Range->getBuffer()[i*l_pOutputMatrix_Range->getDimensionSize(1)]=l_max-l_min;
			  }
		}
		if(ip_pStatisticMedianActive)
		{
			// std::cout<<"Median"<<std::endl;
			for(uint32 i = 0; i < l_pOutputMatrix_Median->getDimensionSize(0); i++)
			  {
				l_pOutputMatrix_Median->getBuffer()[i*l_pOutputMatrix_Median->getDimensionSize(1)]=
				(sort_mat.getDimensionSize(1)%2) ? 
					sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+sort_mat.getDimensionSize(1)/2+1-1] :
					(sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+sort_mat.getDimensionSize(1)/2-1]+sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+sort_mat.getDimensionSize(1)/2+1-1])/2 ;
			  }
		}
		if(ip_pStatisticIQRActive)
		{
			// std::cout<<"IQR"<<std::endl;
			for(uint32 i = 0; i < l_pOutputMatrix_IQR->getDimensionSize(0); i++)
			  {
				float64 l_flow=0,l_fup=0;
				unsigned int l_uireste=sort_mat.getDimensionSize(1)%4, l_uinb=4-l_uireste;
				for(int k=0; k<l_uinb;k++) {l_flow+=sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+sort_mat.getDimensionSize(1)/4-(l_uinb-1)+k-1];}
				l_flow/=l_uinb; 
				for(int k=0; k<l_uinb;k++) {l_fup+=sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+sort_mat.getDimensionSize(1)-sort_mat.getDimensionSize(1)/4-1+k-1];}
				l_fup/=l_uinb; 
				l_pOutputMatrix_IQR->getBuffer()[i*l_pOutputMatrix_IQR->getDimensionSize(1)]=l_fup-l_flow;
			  }
		}
		if(ip_pStatisticPercentileActive)
		{
			// std::cout<<"Percentile"<<std::endl;
			uint32 l_iValue=m_uiPercentileValue;
			for(uint32 i = 0; i <l_pOutputMatrix_Percentile->getDimensionSize(0); i++)
			  {
				l_pOutputMatrix_Percentile->getBuffer()[i*l_pOutputMatrix_Percentile->getDimensionSize(1)]=sort_mat.getBuffer()[i*sort_mat.getDimensionSize(1)+std::max(int(0),int(sort_mat.getDimensionSize(1)*l_iValue/100-1))];
			  }
		}
		
		//emit
		this->activateOutputTrigger(OVP_Algorithm_UnivariateStatistic_OutputTriggerId_ProcessDone, true);
		//std::cout<<"emit OK"<<std::endl;
	}

	// std::cout<<"end process"<<std::endl;
	return true;
}

void CAlgoUnivariateStatistic::setMatDim(OpenViBE::IMatrix* m_matrice, OpenViBE::IMatrix* m_matrref)
{
	m_matrice->setDimensionCount(m_matrref->getDimensionCount());
	m_matrice->setDimensionSize(0,m_matrref->getDimensionSize(0));
	m_matrice->setDimensionSize(1,1);
	OpenViBEToolkit::Tools::Matrix::clearContent(*m_matrice);
	// std::cout<<"output : "<<m_matrice->getDimensionCount()<<" : "<<m_matrice->getDimensionSize(0)<<"*"<<m_matrice->getDimensionSize(1)<<std::endl;
}
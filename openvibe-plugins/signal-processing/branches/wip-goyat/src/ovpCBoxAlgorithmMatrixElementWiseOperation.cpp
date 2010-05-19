#include "ovpCBoxAlgorithmMatrixElementWiseOperation.h"

#include <openvibe-toolkit/ovtk_all.h>
#include <iostream>
#include <string>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::WipGionescu;

CBoxAlgorithmMatrixElementWiseOperation::CBoxAlgorithmMatrixElementWiseOperation(void)
	: m_pAlgorithmMatrixElementWiseOperation(0)
{
}

void CBoxAlgorithmMatrixElementWiseOperation::release(void) 
{
 delete this; 
}

boolean CBoxAlgorithmMatrixElementWiseOperation::initialize(void)
{
	IBox&	l_rStaticBoxContext   = this->getStaticBoxContext();

	//reads the plugin settings
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, m_sgrammar);
	m_sgrammar=preFilter(m_sgrammar);
	// std::cout<<"parse : "<<m_sgrammar<<std::endl;
	
	m_pparser=new CMatrixElementWiseOperation();
	if(!m_pparser->parse(l_rStaticBoxContext.getInputCount(), m_sgrammar.toASCIIString(),false,getLogManager()))
    {
		//getLogManager()<<LogLevel_Trace<< __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::initialize" <<"\n";
		getLogManager()<<LogLevel_Trace<< "echec in parsing"<<"\n";
		return false;
    }

	// Create algorithm
	m_pAlgorithmMatrixElementWiseOperation   = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_MatrixElementWiseOperation));
	m_pAlgorithmMatrixElementWiseOperation->initialize();
    op_pAlgorithmMatrixOutput.initialize(m_pAlgorithmMatrixElementWiseOperation->getOutputParameter(OVP_Algorithm_MatrixElementWiseOperation_OutputParameterId_Result));
    
    // Create inputs decoder
	for(uint32 i=0; i < l_rStaticBoxContext.getInputCount(); i++)
	{   
		m_vecInStreamDecoders.push_back(&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamDecoder)));
        (*m_vecInStreamDecoders.rbegin())->initialize();
    }
	
    // Create output encoder
    m_pOutStreamEncoder = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_SignalStreamEncoder));
    m_pOutStreamEncoder->initialize();
	//
    ip_pMatrixToEncode.initialize(m_pOutStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix));
	ip_ui64SamplingRate.initialize(m_pOutStreamEncoder->getInputParameter(OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate));
	op_pBuffer.initialize(m_pOutStreamEncoder->getOutputParameter(OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
	
	//
    ip_pMatrixToEncode.setReferenceTarget(op_pAlgorithmMatrixOutput);
    
    return true;
}

boolean CBoxAlgorithmMatrixElementWiseOperation::uninitialize(void)
{	
	op_pBuffer.uninitialize();
	ip_ui64SamplingRate.uninitialize();
	ip_pMatrixToEncode.uninitialize();
	m_pOutStreamEncoder->uninitialize();
	getAlgorithmManager().releaseAlgorithm(*m_pOutStreamEncoder);

	op_ui64SamplingRate.uninitialize();	
	for(std::vector<OpenViBE::Kernel::IAlgorithmProxy*>::iterator it=m_vecInStreamDecoders.begin(); it != m_vecInStreamDecoders.end(); it++)
        {
		 (*it)->uninitialize();
		 getAlgorithmManager().releaseAlgorithm(**it);
		}

	op_pAlgorithmMatrixOutput.uninitialize();	
    m_pAlgorithmMatrixElementWiseOperation->uninitialize();
    getAlgorithmManager().releaseAlgorithm(*m_pAlgorithmMatrixElementWiseOperation);
	
	delete m_pparser;
	
	return true;
}

boolean CBoxAlgorithmMatrixElementWiseOperation::processInput(uint32 ui32InputIndex)
{
	IBox&	l_rStaticBoxContext     = this->getStaticBoxContext();
	IBoxIO&	l_rDynamicBoxContext    = this->getDynamicBoxContext();
	
	for(uint32 i=0; i < l_rStaticBoxContext.getInputCount(); i++)
	{   if(l_rDynamicBoxContext.getInputChunkCount(i) == 0)
		    return true;
    }
    
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	
	return true;
}

boolean CBoxAlgorithmMatrixElementWiseOperation::process(void)
{
	// std::cout<<"process"<<std::endl;

//	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

    std::vector<OpenViBE::uint64>   l_chunkStartTime;
    std::vector<OpenViBE::uint64>   l_chunkEndTime;
    
    uint32 headers = 0, buffers = 0, ends = 0;
    
    uint32 i = 0;
	for(std::vector<OpenViBE::Kernel::IAlgorithmProxy*>::iterator it=m_vecInStreamDecoders.begin(); it != m_vecInStreamDecoders.end(); i++, it++)
    {
		TParameterHandler < const IMemoryBuffer* > ip_pMatrixToDecode((*it)->getInputParameter(OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode));
		op_ui64SamplingRate.initialize((*it)->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate));
		ip_pMatrixToDecode  = l_rDynamicBoxContext.getInputChunk(i, 0);
		
        (*it)->process();
		
        if((*it)->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
            headers++;
	        
            TParameterHandler < IMatrix* > op_iMatrix((*it)->getOutputParameter(OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix));
            m_pmatrixes.push_back(op_iMatrix);
            l_chunkStartTime.push_back(l_rDynamicBoxContext.getInputChunkStartTime(i, 0));
            l_chunkEndTime.push_back(l_rDynamicBoxContext.getInputChunkEndTime(i, 0));
        }            
		if((*it)->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
            buffers++;	    
        } 
		if((*it)->isOutputTriggerActive(OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
            ends++;	    
        }
    
		ip_pMatrixToDecode.uninitialize();
        l_rDynamicBoxContext.markInputAsDeprecated(i, 0);
    }
    
	getLogManager()<<LogLevel_Trace<< "nb of headers = " << headers << ". nb of buffers = " << buffers << ". nb of ends = " << ends << "\n";

    if(ends == m_vecInStreamDecoders.size())
    {

	}
    
    if(headers == m_vecInStreamDecoders.size())
    {
		//write output header and prepare output
        if(!honorDimensionAndSize(m_pmatrixes, l_chunkStartTime, l_chunkEndTime))
        {   
			//getLogManager()<<LogLevel_Warning<<  "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::process::honorDimensionAndSize " << "\n";
			getLogManager()<<LogLevel_Warning<<  "matrixes don't match dimension or size" << "\n";
		
            return true;
		}   
	}
    
    // if all input bufferes received
    if(buffers == m_vecInStreamDecoders.size())
    {
		TParameterHandler < void* > ip_pMatrixes(m_pAlgorithmMatrixElementWiseOperation->getInputParameter(OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Inputs));
		TParameterHandler < OpenViBE::CString* > ip_pGrammar(m_pAlgorithmMatrixElementWiseOperation->getInputParameter(OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Grammar));
		TParameterHandler < CMatrixElementWiseOperation* > ip_pParser(m_pAlgorithmMatrixElementWiseOperation->getInputParameter(OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_ParserOperator));

        ip_pMatrixes    = &m_pmatrixes;
        ip_pGrammar     = &m_sgrammar;
		ip_pParser     = m_pparser;
        m_pAlgorithmMatrixElementWiseOperation->process(OVP_Algorithm_MatrixElementWiseOperation_InputTriggerId_Evaluate);
		//op_pAlgorithmMatrixOutput should be filled here, now
        if(m_pAlgorithmMatrixElementWiseOperation->isOutputTriggerActive(OVP_Algorithm_MatrixElementWiseOperation_OutputTriggerId_Success))
        {   
			ip_ui64SamplingRate=op_ui64SamplingRate;
            op_pBuffer = l_rDynamicBoxContext.getOutputChunk(0);
            
            m_pOutStreamEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer);
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, 0), l_rDynamicBoxContext.getInputChunkEndTime(0, 0));
        }
    }
    
	return true;
}

OpenViBE::boolean CBoxAlgorithmMatrixElementWiseOperation::honorDimensionAndSize(const std::vector<OpenViBE::IMatrix*>& matrixes,
    const std::vector<OpenViBE::uint64>& chunkStartTime, const std::vector<OpenViBE::uint64>& chunkEndTime)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
    
    if(matrixes.size() == 0)
    {
		//getLogManager()<<LogLevel_Debug<<  "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::honorDimensionAndSize" << "\n";
		getLogManager()<<LogLevel_Debug<<  "input matrixes size == 0" << "\n";
		return false;
    }
        
    if((matrixes.size() != chunkStartTime.size()) || (matrixes.size() != chunkEndTime.size()))
    {   
		//getLogManager()<<LogLevel_Debug<<  "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::honorDimensionAndSize" << "\n";
		getLogManager()<<LogLevel_Debug<<  "chunkStart or End size is different of matrixes size" << "\n";
		return false;
    }
    
    uint32  l_uinbDim       = matrixes[0]->getDimensionCount();
    uint32  l_uidimSize     = matrixes[0]->getDimensionSize(0);
    uint32  l_uibuffSize    = matrixes[0]->getBufferElementCount();
    uint32  l_uistartTime   = uint32(chunkStartTime[0]);
    uint32  l_uiendTime     = uint32(chunkEndTime[0]);
    
	OpenViBEToolkit::Tools::Matrix::copyDescription(*op_pAlgorithmMatrixOutput, *matrixes[0]);
    
    op_pBuffer = l_rDynamicBoxContext.getOutputChunk(0);
       
    m_pOutStreamEncoder->process(OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader);
	l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, 0), l_rDynamicBoxContext.getInputChunkEndTime(0, 0));
        
    getLogManager()<<LogLevel_Trace<<	"Dimensions : "<<l_uinbDim <<". Sizes : " << l_uidimSize << ". Buffer size : " << l_uibuffSize << "\n";
	getLogManager()<<LogLevel_Trace<<	op_pAlgorithmMatrixOutput->getDimensionCount() << " " << op_pAlgorithmMatrixOutput->getDimensionSize(0) << " " << op_pAlgorithmMatrixOutput->getBufferElementCount() << "\n";

	for(uint32 i=1; i < matrixes.size(); i++)
    {   if( (l_uinbDim != matrixes[i]->getDimensionCount()) ||
            (l_uidimSize != matrixes[i]->getDimensionSize(0)) ||
            (l_uibuffSize != matrixes[i]->getBufferElementCount()) ||
            (l_uistartTime != chunkStartTime[i]) ||
            (l_uiendTime != chunkEndTime[i]))
        {   
		    //getLogManager()<<LogLevel_Debug<<	"GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::honorDimensionAndSize" << "\n";
			getLogManager()<<LogLevel_Debug<<	"dimension or size is different from first one" << "\n";
            return false;
        }
    }
    
    return true;
}

CString	CBoxAlgorithmMatrixElementWiseOperation::preFilter(CString& str_input)
{
	std::string l_str_output=str_input;
	std::string l_str_noise=" ";
	size_t l_sz=l_str_output.find(l_str_noise);
	while(l_sz>0 && l_sz<l_str_output.length()-1)
	  {
		l_str_output.replace(l_sz,l_str_noise.length(),"");
		l_sz=l_str_output.find(l_str_noise);
	  }

	return CString(l_str_output.c_str());
}


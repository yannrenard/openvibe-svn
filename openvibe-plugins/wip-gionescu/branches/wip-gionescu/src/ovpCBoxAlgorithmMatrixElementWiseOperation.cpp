#include <iostream>

#include <openvibe-toolkit/ovtk_all.h>

#include "ovpCBoxAlgorithmMatrixElementWiseOperation.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::WipGionescu;

CBoxAlgorithmMatrixElementWiseOperation::CBoxAlgorithmMatrixElementWiseOperation(void)
	: m_pAlgorithmMatrixElementWiseOperation(0)
{
}

/*
uint64 CBoxAlgorithmMatrixElementWiseOperation::getClockFrequency(void)
{
	return 0; // the box clock frequency
}
*/

boolean CBoxAlgorithmMatrixElementWiseOperation::initialize(void)
{
	IBox&	l_rStaticBoxContext   = this->getStaticBoxContext();
	
	//reads the plugin settings
	getBoxAlgorithmContext()->getStaticBoxContext()->getSettingValue(0, m_grammar);
	
	if(!CMatrixElementWiseOperation::parse(l_rStaticBoxContext.getInputCount(), m_grammar.toASCIIString()))
    {   std::cout << "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::initialize" << std::endl;
		return false;
    }

	// Creates algorithms
	m_pAlgorithmMatrixElementWiseOperation   = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_ClassId_Algorithm_MatrixElementWiseOperation));
	m_pAlgorithmMatrixElementWiseOperation->initialize();
    
    // Create input decoders
	for(uint32 i=0; i < l_rStaticBoxContext.getInputCount(); i++)
	{   m_vecInStreamDecoders.push_back(&getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder)));
        (*m_vecInStreamDecoders.rbegin())->initialize();
    }
	
    // Create output encoder
    m_pOutStreamEncoder = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder));
    m_pOutStreamEncoder->initialize();
	
    TParameterHandler < IMatrix* > op_pMatrixToEncode(m_pAlgorithmMatrixElementWiseOperation->getOutputParameter(OVP_Algorithm_MatrixElementWiseOperation_OutputParameterId_Result));
    TParameterHandler < IMatrix* > ip_pMatrixToEncode(m_pOutStreamEncoder->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix));
    
    ip_pMatrixToEncode.setReferenceTarget(op_pMatrixToEncode);
    
    return true;
}

boolean CBoxAlgorithmMatrixElementWiseOperation::uninitialize(void)
{

    // uninitialize section
    m_pOutStreamEncoder->uninitialize();

	for(std::vector<OpenViBE::Kernel::IAlgorithmProxy*>::iterator it=m_vecInStreamDecoders.begin(); it != m_vecInStreamDecoders.end(); it++)
        (*it)->uninitialize();
    
    m_pAlgorithmMatrixElementWiseOperation->uninitialize();
    	
    // release section
    getAlgorithmManager().releaseAlgorithm(*m_pAlgorithmMatrixElementWiseOperation);
	
	return true;
}

/*
boolean CBoxAlgorithmMatrixElementWiseOperation::processEvent(IMessageEvent& rMessageEvent)
{
	// ...

	// getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
*/

/*
boolean CBoxAlgorithmMatrixElementWiseOperation::processSignal(IMessageSignal& rMessageSignal)
{
	// ...

	// getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
*/

/*
boolean CBoxAlgorithmMatrixElementWiseOperation::processClock(IMessageClock& rMessageClock)
{
	// ...

	// getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
*/

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
//	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

    std::vector<OpenViBE::uint64>   l_chunkStartTime;
    std::vector<OpenViBE::uint64>   l_chunkEndTime;
    
    uint32 headers = 0, buffers = 0, ends = 0;
    
    uint32 i = 0;
	for(std::vector<OpenViBE::Kernel::IAlgorithmProxy*>::iterator it=m_vecInStreamDecoders.begin(); it != m_vecInStreamDecoders.end(); i++, it++)
    {   TParameterHandler < const IMemoryBuffer* > ip_pMatrixToDecode((*it)->getInputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_InputParameterId_MemoryBufferToDecode));
		ip_pMatrixToDecode  = l_rDynamicBoxContext.getInputChunk(i, 0);
        
        (*it)->process();
		
        if((*it)->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedHeader))
		{   std::cout << "GELU INFO >>> OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedHeader" << std::endl;
            headers++;
	        
            TParameterHandler < IMatrix* > op_iMatrix((*it)->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix));
            m_matrixes.push_back(op_iMatrix);
            l_chunkStartTime.push_back(l_rDynamicBoxContext.getInputChunkStartTime(i, 0));
            l_chunkEndTime.push_back(l_rDynamicBoxContext.getInputChunkEndTime(i, 0));
        }            
		if((*it)->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{   std::cout << "GELU INFO >>> OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer" << std::endl;
            buffers++;	    
        } 
		if((*it)->isOutputTriggerActive(OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedEnd))
		{   std::cout << "GELU INFO >>> OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedEnd" << std::endl;
            ends++;	    
        }
    
        l_rDynamicBoxContext.markInputAsDeprecated(i, 0);
    }
    
    std::cout << "GELU INFO >>> headers = " << headers << " buffers = " << buffers << " ends = " << ends << std::endl;
    
    if(headers == m_vecInStreamDecoders.size())
    {   std::cout << "GELU INFO >>> IsValid()" << std::endl;
        if(!IsValid(m_matrixes, l_chunkStartTime, l_chunkEndTime))
        {   std::cout << "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::process::IsValid " << std::endl;
		
            return true;
    }   }
    
    // if all input bufferes received
    if(buffers == m_vecInStreamDecoders.size())
    {   TParameterHandler < void* > ip_pMatrixes(m_pAlgorithmMatrixElementWiseOperation->getInputParameter(OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Inputs));
		TParameterHandler < OpenViBE::CString* > ip_pGrammar(m_pAlgorithmMatrixElementWiseOperation->getInputParameter(OVP_Algorithm_MatrixElementWiseOperation_InputParameterId_Grammar));

        ip_pMatrixes    = &m_matrixes;
        ip_pGrammar     = &m_grammar;
        
        m_pAlgorithmMatrixElementWiseOperation->process(OVP_Algorithm_MatrixElementWiseOperation_InputTriggerId_Evaluate);
        
        if(m_pAlgorithmMatrixElementWiseOperation->isOutputTriggerActive(OVP_Algorithm_MatrixElementWiseOperation_OutputTriggerId_Success))
        {   TParameterHandler < IMemoryBuffer* > op_pBuffer(m_pOutStreamEncoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
            op_pBuffer = l_rDynamicBoxContext.getOutputChunk(0);
            
            m_pOutStreamEncoder->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer);
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, 0), l_rDynamicBoxContext.getInputChunkEndTime(0, 0));
        }
    }
    
	return true;
}

OpenViBE::boolean CBoxAlgorithmMatrixElementWiseOperation::IsValid(const std::vector<OpenViBE::IMatrix*>& matrixes,
    const std::vector<OpenViBE::uint64>& chunkStartTime, const std::vector<OpenViBE::uint64>& chunkEndTime)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
    
    if(matrixes.size() == 0)
    {   std::cout << "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::IsValid" << std::endl;
		return false;
    }
        
    if((matrixes.size() != chunkStartTime.size()) || (matrixes.size() != chunkEndTime.size()))
    {   std::cout << "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::IsValid" << std::endl;
		return false;
    }
    
    uint32  nbDim       = matrixes[0]->getDimensionCount();
    uint32  dimSize     = matrixes[0]->getDimensionSize(0);
    uint32  buffSize    = matrixes[0]->getBufferElementCount();
    uint32  startTime   = chunkStartTime[0];
    uint32  endTime     = chunkEndTime[0];
    
    TParameterHandler < IMatrix* > op_iMatrix(m_pAlgorithmMatrixElementWiseOperation->getOutputParameter(OVP_Algorithm_MatrixElementWiseOperation_OutputParameterId_Result));
	OpenViBEToolkit::Tools::Matrix::copyDescription(*op_iMatrix, *matrixes[0]);
    
    TParameterHandler < IMemoryBuffer* > op_pBuffer(m_pOutStreamEncoder->getOutputParameter(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer));
    op_pBuffer = l_rDynamicBoxContext.getOutputChunk(0);
       
    m_pOutStreamEncoder->process(OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader);
	l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, 0), l_rDynamicBoxContext.getInputChunkEndTime(0, 0));
        
    std::cout << nbDim << " " << dimSize << " " << buffSize << std::endl;
    std::cout << op_iMatrix->getDimensionCount() << " " << op_iMatrix->getDimensionSize(0) << " " << op_iMatrix->getBufferElementCount() << std::endl;
      
	for(uint32 i=1; i < matrixes.size(); i++)
    {   if( (nbDim != matrixes[i]->getDimensionCount()) ||
            (dimSize != matrixes[i]->getDimensionSize(0)) ||
            (buffSize != matrixes[i]->getBufferElementCount()) ||
            (startTime != chunkStartTime[i]) ||
            (endTime != chunkEndTime[i]))
        {   std::cout << "GELU ERROR >>> " << __FILE__ << "::" << __LINE__ << " CBoxAlgorithmMatrixElementWiseOperation::IsValid" << std::endl;
		    
            return false;
        }
    }
    
    return true;
}

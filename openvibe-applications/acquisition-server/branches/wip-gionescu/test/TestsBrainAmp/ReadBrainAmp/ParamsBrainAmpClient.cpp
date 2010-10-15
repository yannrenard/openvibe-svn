#include "ParamsBrainAmpClient.h"

#include <fstream>

std::ofstream	dumpFile("c:/tmp/ParamsBrainAmpClient.txt", std::ios::out | std::ios::trunc);

bool ParamsBrainAmpClient::StartThreads(HWND hWnd, const WORD wWidth, const WORD wHeight)
{
	m_hWnd		= hWnd;
	m_wndWidth	= wWidth;
	m_wndHeight	= wHeight;
	m_wndPos	= 0;

	if(!initialize())
		return false;

	if(!m_acqThreadRead.start(this))
		return false;
	
	if(!m_acqThreadDisplay.start(this))
		return false;

	m_channelCount	= nbChannels + 1;

	return true;
}

bool ParamsBrainAmpClient::SetParams(const std::string& cmdLine)
{
	std::istringstream iss(cmdLine);

	iss >> serverName >> serverPort >> chunkSize >> sendWait;

	
	return true;
}

bool ParamsBrainAmpClient::ProcessReadData()
{
	return readData();
}

bool ParamsBrainAmpClient::ProcessDisplayData()
{
	Sleep(sendWait + 4);
		
#if defined(STRATEGY_CSignalsAndStimulationFifo) || defined(STRATEGY_CSignalsAndStimulationCircular)
	if(!m_outputData.isValid()&& !m_outputData.build(m_signalsAndStimulation.getChannelCount()-1, chunkSize))
		return false;

	if(m_signalsAndStimulation.canExtract(chunkSize))	
	{	m_stimulations.resize(0);
	
		OpenViBE::uint32	nbStimulations = m_signalsAndStimulation.getStimulationCount(chunkSize);

		for(OpenViBE::uint32 iStimulation=0; iStimulation < nbStimulations; iStimulation++)
		{	OpenViBE::uint32	stimulationsPos;
			OpenViBE::uint32	stimulation = m_signalsAndStimulation.getStimulation(stimulationsPos, iStimulation, chunkSize);
		
			m_stimulations.push_back(CStimulation(stimulationsPos, stimulation));
		}
		
		m_signalsAndStimulation.extractData(m_outputData);

		Display(m_outputData.getChannelBegin(0));
	}
#elif defined(STRATEGY_TAcqSignalsAndStimulationFifo) || defined(STRATEGY_TAcqSignalsAndStimulationCircular)
	if(m_signalsAndStimulation.canExtract())
	{	m_stimulations.resize(0);
	
		OpenViBE::uint32	nbStimulations = m_signalsAndStimulation.getStimulationCount();

		for(OpenViBE::uint32 iStimulation=0; iStimulation < nbStimulations; iStimulation++)
		{	OpenViBE::uint32	stimulationsPos;
			OpenViBE::uint32	stimulation = m_signalsAndStimulation.getStimulation(stimulationsPos, iStimulation);
		
			m_stimulations.push_back(CStimulation(stimulationsPos, stimulation));
		}
		
		Display(m_signalsAndStimulation.getExtractPtr());

		m_signalsAndStimulation.adjusteExtract();
	}
#elif defined(STRATEGY_CAcqServerCircularBuffer)
	if(m_signalsAndStimulation.canExtract())
	{	if(!m_outputData.isValid() && !m_outputData.build(nbChannels, 1, chunkSize, 1))
			return false;

		std::vector<CAcqServerCircularBuffer::CStimulation>	stimulations;
		if(!m_signalsAndStimulation.extract(m_outputData.getBuffer(), stimulations))
			return false;

		amplifyData();

		m_stimulations.resize(0);
		for(std::vector<CAcqServerCircularBuffer::CStimulation>::const_iterator it=stimulations.begin(); it != stimulations.end(); it++)
			m_stimulations.push_back(CStimulation(it->position, it->value));
		
		Display(m_outputData.getBuffer());

		dumpFile	<< "   Display "
					<< m_signalsAndStimulation.dump()
					<< std::endl;
	}
#endif
  	
	return true;
}

void ParamsBrainAmpClient::Display(const OpenViBE::float32* pData)
{
	HDC hdc = GetDC(m_hWnd);

	for(OpenViBE::uint32 ii=0; ii < m_channelCount; ii++, pData+=chunkSize)
		DrawSignal(hdc, ii, pData, OpenViBE::float32((ii == (m_channelCount - 1)) ? 4 : 0x7fff));
	DrawTriggers(hdc);
		
	m_wndPos	+= chunkSize;
	if(m_wndPos >= m_wndWidth)
		m_wndPos	= 0;

	ClearSignal(hdc);

	ReleaseDC(m_hWnd, hdc);
}

void ParamsBrainAmpClient::DrawTriggers(HDC hdc)
{
#define	COLOR(r, g, b)	(r + (g << 8) + (b << 16))

	for(std::vector<CStimulation>::iterator it=m_stimulations.begin(); it != m_stimulations.end(); it++)
	{	HPEN hPen;
	
		if(it->value == 1)
			hPen = ::CreatePen(PS_SOLID, 1, COLOR(0xff, 0, 0));
		else if(it->value == 2)
			hPen = ::CreatePen(PS_SOLID, 1, COLOR(0, 0xff, 0));
		else if(it->value == 3)
			hPen = ::CreatePen(PS_SOLID, 1, COLOR(0, 0, 0xff));
		else if(it->value == 4)
			hPen = ::CreatePen(PS_SOLID, 1, COLOR(0xff, 0xff, 0));
		else if(it->value == 5)
			hPen = ::CreatePen(PS_SOLID, 1, COLOR(0xff, 0, 0xff));
		else if(it->value == 6)
			hPen = ::CreatePen(PS_SOLID, 1, COLOR(0, 0xff, 0xff));
		else
			hPen = ::CreatePen(PS_SOLID, 1, 0);
		::SelectObject(hdc, hPen);
		::MoveToEx(hdc, m_wndPos+it->position, 0, 0);
		::LineTo(hdc, m_wndPos+it->position, m_wndHeight);
		::DeleteObject(hPen);
	}

	m_stimulations.resize(0);
#undef COLOR
}

void ParamsBrainAmpClient::ClearSignal(HDC hdc)
{
	SelectObject(hdc, GetStockObject(DC_PEN));
	SetDCPenColor(hdc, RGB(0xff,0xff,0xff));
	
	for(OpenViBE::uint32 ii=0, jj = m_wndPos+1; ii < OpenViBE::uint32(chunkSize); ii++, jj++)
	{	::MoveToEx(hdc, jj, 0, 0);
		::LineTo(hdc, jj, m_wndHeight);
	}

	SetDCPenColor(hdc, RGB(0, 0, 0));
}


void ParamsBrainAmpClient::DrawSignal(HDC hdc, const int signal, const OpenViBE::float32* data, const OpenViBE::float32 gain)
{
	int	deltaY	 = m_wndHeight/m_channelCount;
	int	yMin     = deltaY*(signal + 1) - deltaY/2;
	deltaY		-= 4;
	deltaY		/= 2;

	for(OpenViBE::uint32 ii=0, jj=m_wndPos; ii < OpenViBE::uint32(chunkSize); ii++, data++, jj++)
	{	if(ii == 0)
		{	if(m_wndPos == 0)
				::MoveToEx(hdc, jj, yMin - int(*data * deltaY / gain), 0);
			else
				::MoveToEx(hdc, jj - 1, yMin - int(m_lastValues[signal] * deltaY / gain), 0);
		}
		::LineTo(hdc, jj, yMin - int(*data * deltaY / gain));
	}
	
	m_lastValues[signal]	= *(data - 1);
}


bool ParamsBrainAmpClient::initialize()
{
	if(m_initialized)
		return true;

	if(!myTcpConnection.Connect(serverPort, serverName))
		return false;

#if defined(STRATEGY_CSignalsAndStimulationFifo) || defined(STRATEGY_CSignalsAndStimulationCircular)
	if(!m_startBuffer.build(1, 2000))
		return false;
	if(!m_inputBuffer.build(1, 10000))
		return false;
#elif defined(STRATEGY_TAcqSignalsAndStimulationFifo) || defined(STRATEGY_TAcqSignalsAndStimulationCircular)
	if(!m_startBuffer.build(1, 2000))
		return false;
	if(!m_inputBuffer.build(1, 10000))
		return false;
#elif defined(STRATEGY_CAcqServerCircularBuffer)
	if(!m_startBuffer.build(1, 4000, 1, 1))
		return false;
	if(!m_inputBuffer.build(1, 20000, 1, 1))
		return false;
#endif

	m_pStructRDA_MessageStart	= (RDA_MessageStart*)	m_startBuffer.getBuffer();
	m_pStructRDA_MessageHeader	= (RDA_MessageHeader*)	m_inputBuffer.getBuffer();
	m_pStructRDA_MessageData	= (RDA_MessageData*)	m_pStructRDA_MessageHeader;
	m_pStructRDA_MessageData32	= (RDA_MessageData32*)	m_pStructRDA_MessageHeader;

	// receive header
	if(!receiveStart())
		return false;
	
	// Check for correct header nType
	if(!m_pStructRDA_MessageStart->IsStart())
		return false;

	nbChannels		= m_pStructRDA_MessageStart->nChannels;

	m_initialized	= true;

	return true;
}

bool ParamsBrainAmpClient::readData()
{
	if(!receiveData())
		return false;
	
	// Check for correct header nType
	if(!m_pStructRDA_MessageHeader->IsData())
		return true;

	RDA_Marker*	l_pMarker;

#if defined(STRATEGY_CSignalsAndStimulationFifo) || defined(STRATEGY_CSignalsAndStimulationCircular)
#if defined(STRATEGY_CSignalsAndStimulationCircular)
	if(!m_signalsAndStimulation.isValid())
	{	if(!m_signalsAndStimulation.build(nbChannels, 4*m_pStructRDA_MessageData->nPoints*chunkSize))
			return false;
	}
#endif

	if(m_pStructRDA_MessageHeader->IsData16())
	{	if(!m_signalsAndStimulation.appendTranspozedData(m_pStructRDA_MessageData->nData, nbChannels, m_pStructRDA_MessageData->nPoints))
			return false;

		l_pMarker	= (RDA_Marker*) (m_pStructRDA_MessageData->nData + nbChannels*m_pStructRDA_MessageData->nPoints);
	}
	else
	{	if(!m_signalsAndStimulation.appendTranspozedData(m_pStructRDA_MessageData32->fData, nbChannels, m_pStructRDA_MessageData32->nPoints))
			return false;

		l_pMarker	= (RDA_Marker*) (m_pStructRDA_MessageData32->fData + nbChannels*m_pStructRDA_MessageData32->nPoints);
	}

	amplifyData(m_pStructRDA_MessageData32->nPoints);
#elif defined(STRATEGY_TAcqSignalsAndStimulationFifo) || defined(STRATEGY_TAcqSignalsAndStimulationCircular)
#if defined(STRATEGY_TAcqSignalsAndStimulationFifo)
	if(!m_signalsAndStimulation.isValid() && !m_signalsAndStimulation.build(nbChannels, chunkSize))
		return false;
#elif defined(STRATEGY_TAcqSignalsAndStimulationCircular)
	size_t boufferCount = (m_pStructRDA_MessageData->nPoints < size_t(chunkSize)) ? 8 : 8*(m_pStructRDA_MessageData->nPoints/chunkSize + 1);
	if(!m_signalsAndStimulation.isValid() && !m_signalsAndStimulation.build(nbChannels, chunkSize, boufferCount))
		return false;
#endif

	if(m_pStructRDA_MessageHeader->IsData16())
	{	if(!m_signalsAndStimulation.appendTranspozedData(m_pStructRDA_MessageData->nData, m_pStructRDA_MessageData->nPoints))
			return false;

		l_pMarker	= (RDA_Marker*) (m_pStructRDA_MessageData->nData + nbChannels*m_pStructRDA_MessageData->nPoints);
	}
	else
	{	if(!m_signalsAndStimulation.appendTranspozedData(m_pStructRDA_MessageData32->fData, m_pStructRDA_MessageData32->nPoints))
			return false;

		l_pMarker	= (RDA_Marker*) (m_pStructRDA_MessageData32->fData + nbChannels*m_pStructRDA_MessageData32->nPoints);
	}
	
	amplifyData(m_pStructRDA_MessageData32->nPoints);
#elif defined(STRATEGY_CAcqServerCircularBuffer)
	if(!m_signalsAndStimulation.isValid() && !m_signalsAndStimulation.build(nbChannels, m_pStructRDA_MessageData->nPoints, chunkSize))
		return false;
	
	if(m_pStructRDA_MessageHeader->IsData16())
		l_pMarker	= (RDA_Marker*) (m_pStructRDA_MessageData->nData + nbChannels*m_pStructRDA_MessageData->nPoints);
	else
		l_pMarker	= (RDA_Marker*) (m_pStructRDA_MessageData32->fData + nbChannels*m_pStructRDA_MessageData32->nPoints);

	std::vector<CAcqServerCircularBuffer::CStimulation>	stimulations;
	stimulations.reserve(100);

	for (OpenViBE::uint32 i = 0; i < m_pStructRDA_MessageData32->nMarkers; i++)
	{	char*	pszType			= l_pMarker->sTypeDesc;
		char*	pszDescription	= pszType + strlen(pszType) + 1;
		char*	pszNextToken	= 0;
		char*	pToken			= strtok_s(pszDescription, "S", &pszNextToken);
		int		stimulation		= atoi(pToken);

		stimulations.push_back(CAcqServerCircularBuffer::CStimulation(l_pMarker->nPosition, stimulation));
		l_pMarker				= (RDA_Marker*)((char*) l_pMarker + l_pMarker->nSize);
	}

	if(m_pStructRDA_MessageHeader->IsData16())
		m_signalsAndStimulation.append(m_pStructRDA_MessageData->nData, stimulations);
	else
		m_signalsAndStimulation.append(m_pStructRDA_MessageData32->fData, stimulations);

	dumpFile	<< "Block " 
				<< m_pStructRDA_MessageData->nBlock
				<< m_signalsAndStimulation.dump()
				<< std::endl;
#endif

#if !defined(STRATEGY_CAcqServerCircularBuffer)
	//////////////////////
	//Markers processing
	for (OpenViBE::uint32 i = 0; i < m_pStructRDA_MessageData32->nMarkers; i++)
	{	char*	pszType			= l_pMarker->sTypeDesc;
		char*	pszDescription	= pszType + strlen(pszType) + 1;
		char*	pszNextToken	= 0;
		char*	pToken			= strtok_s(pszDescription, "S", &pszNextToken);
		int		stimulation		= atoi(pToken);

		m_signalsAndStimulation.addStimulation(l_pMarker->nPosition, OpenViBE::float32(stimulation));
		m_signalsAndStimulation.addStimulation(l_pMarker->nPosition, m_pStructRDA_MessageData32->nPoints, OpenViBE::float32(stimulation));
		l_pMarker				= (RDA_Marker*)((char*) l_pMarker + l_pMarker->nSize);
	}

	dumpFile	<< "Block " << m_pStructRDA_MessageData->nBlock
				<< m_signalsAndStimulation.dump()
				<< std::endl;
#endif

	return true;
}

OpenViBE::boolean ParamsBrainAmpClient::receiveData()
{
	DEFINE_GUIDD(GUID_RDAHeader,
		1129858446, 51606, 19590, char(175), char(74), char(152), char(187), char(246), char(201), char(20), char(80)
	);

	RDA_MessageHeader* l_pRDA_MessageHeader = (RDA_MessageHeader*) m_inputBuffer.getBuffer();
	// Initialize vars for reception
	if(!readData((char*) l_pRDA_MessageHeader, sizeof(RDA_MessageHeader)))
		return false;

	// Check for correct header GUID.
	if(!COMPARE_GUIDD(l_pRDA_MessageHeader->guid, GUID_RDAHeader))
		return false;

	if(!readData((const char*) l_pRDA_MessageHeader + sizeof(RDA_MessageHeader), l_pRDA_MessageHeader->nSize - sizeof(RDA_MessageHeader)))
		return false;

	return true;
}

OpenViBE::boolean ParamsBrainAmpClient::receiveStart()
{
	DEFINE_GUIDD(GUID_RDAHeader,
		1129858446, 51606, 19590, char(175), char(74), char(152), char(187), char(246), char(201), char(20), char(80)
	);

	RDA_MessageHeader* l_pRDA_MessageHeader = (RDA_MessageHeader*) m_startBuffer.getBuffer();
	// Initialize vars for reception
	if(!readData((char*) l_pRDA_MessageHeader, sizeof(RDA_MessageHeader)))
		return false;

	// Check for correct header GUID.
	if(!COMPARE_GUIDD(l_pRDA_MessageHeader->guid, GUID_RDAHeader))
		return false;

	if(!readData((const char*) l_pRDA_MessageHeader + sizeof(RDA_MessageHeader), l_pRDA_MessageHeader->nSize - sizeof(RDA_MessageHeader)))
		return false;

	return true;
}

OpenViBE::boolean ParamsBrainAmpClient::readData(const char* data, const OpenViBE::uint32 uint32DimData)
{
	OpenViBE::uint32 l_uint32DimData = uint32DimData;
	while(l_uint32DimData)
	{	OpenViBE::uint32 l_ui32Result  = myTcpConnection.receive(data, uint32DimData);
		l_uint32DimData		-= l_ui32Result;
		data				+= l_ui32Result;
	}

	return true;
}

void ParamsBrainAmpClient::amplifyData(const OpenViBE::uint32 nbSamples)
{
#if !defined(STRATEGY_CAcqServerCircularBuffer)
	for(OpenViBE::uint32 iChannel=0; iChannel < OpenViBE::uint32(nbChannels); iChannel++)
		m_signalsAndStimulation.amplifyData(iChannel, nbSamples, OpenViBE::float32(m_pStructRDA_MessageStart->dResolutions[iChannel]));
#endif
}

void ParamsBrainAmpClient::amplifyData()
{
#if defined(STRATEGY_CAcqServerCircularBuffer)
	for(OpenViBE::uint32 iChannel=0; iChannel < OpenViBE::uint32(nbChannels); iChannel++)
		m_outputData.amplifyData(iChannel, OpenViBE::float32(m_pStructRDA_MessageStart->dResolutions[iChannel]));
#endif
}

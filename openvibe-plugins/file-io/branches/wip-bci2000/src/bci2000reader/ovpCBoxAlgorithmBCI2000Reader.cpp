#include "ovpCBoxAlgorithmBCI2000Reader.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include "bci2000helper.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::FileIO;
using namespace OpenViBEToolkit;

using namespace BCI2000;

boolean CBoxAlgorithmBCI2000Reader::initialize(void)
{
    CString filename=FSettingValueAutoCast(*this->getBoxAlgorithmContext(),0);
    m_b2k_reader=new BCI2000Reader(filename);
    if (!m_b2k_reader->is_good())
    {
        this->getLogManager() << LogLevel_ImportantWarning << "Could not open file ["
                              << filename << "]\n";
        return false;
    }

    m_header_sent=false;
    m_rate=m_b2k_reader->get_rate();
    m_channels=m_b2k_reader->get_channels();
    m_per_buffer=(uint64)FSettingValueAutoCast(*this->getBoxAlgorithmContext(),1);
    m_samples_sent=0;
    m_buffer=new float64[m_channels*m_per_buffer];
    m_stims=new uint32[m_b2k_reader->get_state_vector_size()*m_per_buffer];

    m_signal_encoder.initialize(*this);
    m_signal_out=m_signal_encoder.getInputMatrix();
    m_signal_out->setDimensionCount(2);
    m_signal_out->setDimensionSize(0,m_channels);
    m_signal_out->setDimensionSize(1,m_per_buffer);
    for (uint32 i=0; i<m_channels; i++)
    {
        std::stringstream name;
        name << "Channel " << i;
        m_signal_out->setDimensionLabel(0,i,name.str().c_str());
    }

    m_state_encoder.initialize(*this);
    m_state_out=m_state_encoder.getInputMatrix();
    m_state_out->setDimensionCount(2);
    m_state_out->setDimensionSize(0,m_b2k_reader->get_state_vector_size());
    m_state_out->setDimensionSize(1,m_per_buffer);
    for (int i=0; i<m_b2k_reader->get_state_vector_size(); i++)
    {
        m_state_out->setDimensionLabel(0,i,m_b2k_reader->get_state_name(i).c_str());
    }
    m_signal_encoder.getInputSamplingRate()=m_rate;
    m_state_encoder.getInputSamplingRate()=m_rate;
    for (int i=0; i<m_b2k_reader->get_state_vector_size(); i++)
    {
        this->getLogManager() << LogLevel_Info << "BCI2000 state var "<< i
                              << " is : " << m_b2k_reader->get_state_name(i).c_str() << "\n";
    }
    return true;
}

boolean CBoxAlgorithmBCI2000Reader::uninitialize(void)
{
    delete[] m_buffer;
    delete[] m_stims;
    delete m_b2k_reader;
    // TODO: check if init failed
    m_signal_encoder.uninitialize();
    m_state_encoder.uninitialize();

    return true;
}

boolean CBoxAlgorithmBCI2000Reader::processClock(IMessageClock& rMessageClock)
{
    if (m_b2k_reader->get_samples_left()>0)
    {
        getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
        return true;
    }

    return false;
}

uint64 CBoxAlgorithmBCI2000Reader::getClockFrequency(void)
{
    std::cout << ((uint64)m_rate<<32)/m_per_buffer << std::endl;
    return ((uint64)m_rate<<32)/m_per_buffer;
}

void CBoxAlgorithmBCI2000Reader::sendHeader(void)
{
    m_signal_encoder.encodeHeader(0);
    m_state_encoder.encodeHeader(1);
    m_header_sent=true;
    getDynamicBoxContext().markOutputAsReadyToSend(0,0,0);
    getDynamicBoxContext().markOutputAsReadyToSend(1,0,0);
}

boolean CBoxAlgorithmBCI2000Reader::process(void)
{
    if (!m_header_sent)
    {
        sendHeader();
    }

    //prepare data
    int r=m_b2k_reader->read_samples(m_buffer,m_stims,m_per_buffer);
    if (r>0)
    {
        // padding. TODO: is it necessary ? or even dangerous ?
        for (uint32 i=r; i<m_per_buffer; i++)
        {
            for (uint32 j=0; j<m_channels; j++)
            {
                m_buffer[i*m_channels+j]=0.0;
            }
        }
        // transpose (yeah, I know... ugly)
        for (uint32 i=0; i<m_per_buffer; i++)
        {
            for (uint32 j=0; j<m_channels; j++)
            {
                m_signal_out->getBuffer()[j*m_per_buffer+i]=m_buffer[i*m_channels+j];
            }
        }
        m_signal_encoder.encodeBuffer(0);
        uint64 StartTime;
        uint64 EndTime;
        StartTime=(((uint64)(m_samples_sent))<<32)/m_rate;
        EndTime=(((uint64)(m_samples_sent+m_per_buffer))<<32)/m_rate;
        m_samples_sent+=r;
        if (m_b2k_reader->get_samples_left()==0)
        {
            m_signal_encoder.encodeEnd(0);
        }

        getDynamicBoxContext().markOutputAsReadyToSend(0,StartTime,EndTime);

        // padding. TODO: is it necessary ? or even dangerous ?
        for (uint32 i=r; i<m_per_buffer; i++)
        {
            for (int j=0; j<m_b2k_reader->get_state_vector_size(); j++)
            {
                m_stims[i*m_b2k_reader->get_state_vector_size()+j]=0;
            }
        }
        // transpose (yeah, I know... ugly)
        for (uint32 i=0; i<m_per_buffer; i++)
        {
            for (int j=0; j<m_b2k_reader->get_state_vector_size(); j++)
            {
                m_state_out->getBuffer()[j*m_per_buffer+i]=
                    m_stims[i*m_b2k_reader->get_state_vector_size()+j];
            }
        }
        m_state_encoder.encodeBuffer(1);

        if (m_b2k_reader->get_samples_left()==0)
        {
            m_signal_encoder.encodeEnd(1);
        }

        getDynamicBoxContext().markOutputAsReadyToSend(1,StartTime,EndTime);
    }
    return true;
}

#ifndef __OpenViBEPlugins_BoxAlgorithm_BCI2000Reader_H__
#define __OpenViBEPlugins_BoxAlgorithm_BCI2000Reader_H__

#include "bci2000helper.h"
#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#define OVP_ClassId_BoxAlgorithm_BCI2000Reader OpenViBE::CIdentifier(0xFF53D107, 0xC31144B8)
#define OVP_ClassId_BoxAlgorithm_BCI2000ReaderDesc OpenViBE::CIdentifier(0xFF53D107, 0xC31144B8)

namespace OpenViBEPlugins
{
namespace FileIO
{
/**
 * \class CBoxAlgorithmBCI2000Reader
 * \author Olivier Rochel (INRIA)
 * \date Tue Jun 21 11:11:04 2011
 * \brief The class CBoxAlgorithmBCI2000Reader describes the box BCI2000 Reader.
 *
 */
class CBoxAlgorithmBCI2000Reader : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
{
public:
    virtual void release(void)
    {
        delete this;
    }

    virtual OpenViBE::boolean initialize(void);
    virtual OpenViBE::boolean uninitialize(void);

    virtual OpenViBE::boolean processClock(OpenViBE::CMessageClock& rMessageClock);
    virtual OpenViBE::uint64 getClockFrequency(void);

    virtual OpenViBE::boolean process(void);

    _IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_BCI2000Reader);

protected:

    OpenViBE::boolean m_header_sent;

    OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmBCI2000Reader > m_signal_encoder;
    OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmBCI2000Reader > m_state_encoder;

    // These 2 were from the time the matrices were built, not given by the encoders.
    // They could be removed, but for now make the code a bit easier to read - that's
    // why they're still there.
    OpenViBE::IMatrix *m_signal_out;
    OpenViBE::IMatrix *m_state_out;

    OpenViBE::uint32 m_rate;
    OpenViBE::uint32 m_channels;
    OpenViBE::uint32 m_per_buffer;
    OpenViBE::float64 *m_buffer; 	// temporary buffer as we'll have to transpose data for signal_out
    OpenViBE::uint32 *m_stims; 	// state variables, to be converted too;
    OpenViBE::uint64 m_samples_sent;
    BCI2000::BCI2000Reader *m_b2k_reader;
    // helpers
    void sendHeader(void);

};

/**
 * \class CBoxAlgorithmBCI2000ReaderDesc
 * \author Olivier Rochel (INRIA)
 * \date Tue Jun 21 11:11:04 2011
 * \brief Descriptor of the box BCI2000 Reader.
 *
 */
class CBoxAlgorithmBCI2000ReaderDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
{
public:

    virtual void release(void) { }

    virtual OpenViBE::CString getName(void) const
    {
        return OpenViBE::CString("BCI2000 Reader");
    }
    virtual OpenViBE::CString getAuthorName(void) const
    {
        return OpenViBE::CString("Olivier Rochel");
    }
    virtual OpenViBE::CString getAuthorCompanyName(void) const
    {
        return OpenViBE::CString("INRIA");
    }
    virtual OpenViBE::CString getShortDescription(void) const
    {
        return OpenViBE::CString("BCI2000 reader");
    }
    virtual OpenViBE::CString getDetailedDescription(void) const
    {
        return OpenViBE::CString("The box will attempt to read a BCI2000 file (.dat)");
    }
    virtual OpenViBE::CString getCategory(void) const
    {
        return OpenViBE::CString("File reading and writing");
    }
    virtual OpenViBE::CString getVersion(void) const
    {
        return OpenViBE::CString("1.2");
    }
    virtual OpenViBE::CString getStockItemName(void) const
    {
        return OpenViBE::CString("gtk-open");
    }

    virtual OpenViBE::CIdentifier getCreatedClass(void) const
    {
        return OVP_ClassId_BoxAlgorithm_BCI2000Reader;
    }
    virtual OpenViBE::Plugins::IPluginObject* create(void)
    {
        return new OpenViBEPlugins::FileIO::CBoxAlgorithmBCI2000Reader;
    }

    virtual OpenViBE::boolean getBoxPrototype(OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
    {
        rBoxAlgorithmPrototype.addOutput("signal",OV_TypeId_Signal);
        rBoxAlgorithmPrototype.addOutput("state",OV_TypeId_Signal);
        rBoxAlgorithmPrototype.addSetting("file name",OV_TypeId_Filename,"");
        rBoxAlgorithmPrototype.addSetting("samples per buffer",OV_TypeId_Integer,"16");
        rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable); // meuh non !

        return true;
    }
    _IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_BCI2000ReaderDesc);
};
};
};

#endif


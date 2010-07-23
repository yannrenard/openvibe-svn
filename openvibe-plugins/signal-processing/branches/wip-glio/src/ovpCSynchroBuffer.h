#ifndef __OpenViBEPlugins_SynchroBuffer_H__
#define __OpenViBEPlugins_SynchroBuffer_H__

#include "ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

#include <fstream>
#include <sstream>
#include <iomanip>

#ifndef NDEBUG
extern	std::ofstream	theEngineDebug;
#define	DumpF(text)		Dump(theEngineDebug, text)
#define	DumpS(text)		Dump(text)
#define	DumpM(text)		DumpF(text)
#else
#define	DumpM(text)
#endif

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		class CMatrix2d : public OpenViBE::CMatrix
		{
		public:
			CMatrix2d();
			CMatrix2d(const OpenViBE::uint32 dimChannels, const OpenViBE::uint32 dimSamples);

			void				build(const OpenViBE::uint32 dimChannels, const OpenViBE::uint32 dimSamples); 

			OpenViBE::float64*	begin() const		{	return m_beginPtr;															}
			OpenViBE::float64*	end() const			{	return m_endPtr;															}
			OpenViBE::float64*	getChannel(const OpenViBE::uint32 channel)			{	return m_beginPtr + channel*dimChannels();	}
			void				increase(const OpenViBE::uint32 dimSamples)
								{	m_endPtr	+= dimSamples;	DumpM("increase ");													}
			void				decrease(const OpenViBE::uint32 dimSamples)
								{	m_endPtr	-= dimSamples;	DumpM("decrease ");													}
			OpenViBE::uint32	dimChannels()
								{	return getDimensionSize(0);																		}
			OpenViBE::uint32	dimSamples()
								{	return getDimensionSize(1);																		}
			OpenViBE::uint32	nbSamples()
								{	return m_endPtr - m_beginPtr;																	}
			void				copy(OpenViBE::float64* dest, OpenViBE::float64* src, const OpenViBE::uint32 srcStride, const OpenViBE::uint32 nbSample);
			void				copy(OpenViBE::float64*	dest, CMatrix2d& src)
								{	copy(dest, src.getBuffer(), src.dimSamples(), src.dimSamples());								}
			void				copy(OpenViBE::float64*	dest, OpenViBE::CMatrix& src)
								{	copy(dest, src.getBuffer(), src.getDimensionSize(1), src.getDimensionSize(1));					}
			void				copy(OpenViBE::float64*	dest, OpenViBE::IMatrix& src)
								{	copy(dest, src.getBuffer(), src.getDimensionSize(1), src.getDimensionSize(1));					}
			
			void				append(OpenViBE::float64* src, const OpenViBE::uint32 srcStride, const OpenViBE::uint32 nbSample)
								{	copy(m_endPtr, src, srcStride, nbSample);
									increase(nbSample);
								}
			void				append(CMatrix2d& src)
								{	copy(m_endPtr, src.getBuffer(), src.dimSamples(), src.dimSamples());
									increase(src.dimSamples());
								}
			void				append(OpenViBE::CMatrix& src)
								{	copy(m_endPtr, src.getBuffer(), src.getDimensionSize(1), src.getDimensionSize(1));
									increase(src.getDimensionSize(1));
								}
			void				append(OpenViBE::IMatrix& src)
								{	copy(m_endPtr, src.getBuffer(), src.getDimensionSize(1), src.getDimensionSize(1));
									increase(src.getDimensionSize(1));
								}
			
			void				extract(OpenViBE::float64* dest, const OpenViBE::uint32 destStride, const OpenViBE::uint32 nbSample);
			void				extract(CMatrix2d& dest)
								{	extract(dest.getBuffer(), dest.dimSamples(), dest.dimSamples());						}
			void				extract(OpenViBE::CMatrix& dest)
								{	extract(dest.getBuffer(), dest.getDimensionSize(1), dest.getDimensionSize(1));		}
			void				extract(OpenViBE::IMatrix& dest)
								{	extract(dest.getBuffer(), dest.getDimensionSize(1), dest.getDimensionSize(1));		}

			void				extract(CMatrix2d& dest, const OpenViBE::uint32 destIndChannel)
								{	extract(dest.getBuffer() + destIndChannel*dest.dimSamples(), dest.dimSamples(), dest.dimSamples());							}
			void				extract(OpenViBE::CMatrix& dest, const OpenViBE::uint32 destIndChannel)
								{	extract(dest.getBuffer() + destIndChannel*dest.getDimensionSize(1), dest.getDimensionSize(1), dest.getDimensionSize(1));			}
			void				extract(OpenViBE::IMatrix& dest, const OpenViBE::uint32 destIndChannel)
								{	extract(dest.getBuffer() + destIndChannel*dest.getDimensionSize(1), dest.getDimensionSize(1), dest.getDimensionSize(1));			}
			
			void				shiftLeft(const OpenViBE::uint32 shift);
			void				shiftLeft(const OpenViBE::uint32 shift, const OpenViBE::uint32 decreaseVal);

			virtual void		Dump(std::ofstream& ofs, const std::string& tag);
			virtual std::string	Dump(const std::string& tag);

		protected:
			OpenViBE::float64*	m_beginPtr;
			OpenViBE::float64*	m_endPtr;
		};

		class CSynchroBuffer : public CMatrix2d
		{
		public:
			typedef enum
			{	INTERPOLATION_NONE	= -1,
				INTERPOLATION_LINEAR,
			} interpolation_type;

		public:
			CSynchroBuffer(const OpenViBE::uint32 groupId, const OpenViBE::uint32 offset = 5);

			void				Build(const OpenViBE::uint32 samplingRate, const OpenViBE::uint32 otherSamplingRate, const OpenViBE::uint32 nbChannel, const OpenViBE::uint32 nbChunkSamples, const OpenViBE::uint32 durationBuffer, const OpenViBE::uint32 interpolationMode = INTERPOLATION_LINEAR); 
			void				Push(OpenViBE::IMatrix& data);
			OpenViBE::boolean	IsInitialized() const	{	return m_initialized;								}
			OpenViBE::boolean	IsDetected() const		{	return m_detected;									}
			OpenViBE::boolean	IsSlower() const		{	return m_samplingRate < m_otherSamplingRate;		}
			OpenViBE::boolean	IsFaster() const		{	return m_samplingRate > m_otherSamplingRate;		}
			OpenViBE::boolean	HasSynchro(OpenViBE::uint32& dim) const
								{	dim		= m_synchroPtr - m_beginSynchroPtr;
									return	m_inSynchro && m_detected;
								}
			OpenViBE::uint32	NbChunkSamples()		{	return m_nbChankSamples;							}
			OpenViBE::uint32	PrepareTransfer(const OpenViBE::uint32 nbSample);
			void				GetResult(OpenViBE::IMatrix& result, const OpenViBE::uint32 offChannels);
			void				SetAsDeprecated();

		private:
			void				Interpolate(const OpenViBE::uint32 nbSample);
			void				FindEdge();
			void				Dump(std::ofstream& ofs, const std::string& tag);
			std::string			Dump(const std::string& tag);

		private:
			OpenViBE::uint32	m_offset;			
			
			OpenViBE::uint32	m_groupId;
			OpenViBE::uint32	m_samplingRate;
			OpenViBE::uint32	m_otherSamplingRate;
			interpolation_type	m_interpolationMode;

			OpenViBE::boolean	m_initialized;
			OpenViBE::boolean	m_inSynchro;
			OpenViBE::boolean	m_detected;
			
			OpenViBE::uint32	m_nbChankSamples;
		
			OpenViBE::float64*	m_limitPtr;
			
			OpenViBE::float64*	m_beginSynchroPtr;
			OpenViBE::float64*	m_endSynchroPtr;
			OpenViBE::float64*	m_synchroPtr;

			CMatrix2d			m_transferBuffer;
		};

	};
};

#endif // __OpenViBEPlugins_SynchroBuffer_H__

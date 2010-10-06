#ifndef __DataRecord_H__
#define __DataRecord_H__

#include <openvibe-toolkit/ovtk_all.h>

class DataRecordingInformation
{
     public : 
		DataRecordingInformation();
		~DataRecordingInformation();
		DataRecordingInformation(const DataRecordingInformation&);
		DataRecordingInformation& operator=(const DataRecordingInformation&);
	
    private : 	
		OpenViBE::uint32 dataMaxSize;
		OpenViBE::uint32 dataDesiredSize;
		OpenViBE::uint32 dataSize;
		OpenViBE::float32 *data;
		
	public:
		
		bool isFull();
		OpenViBE::uint32 Size();
		void flush();
		
		OpenViBE::uint32 setMaxSize(OpenViBE::uint32 size);
		void setDesiredSize(OpenViBE::uint32 size);
		void addData(OpenViBE::float32 buffer);
		void addData(OpenViBE::float32 *buffer,OpenViBE::uint32 size);
		
		void restartDoubleBuffer();
		OpenViBE::float32* concatData(OpenViBE::float32 *buffer);
};
	
class DataReader
{
     public : 
		DataReader();
		
	private :
		std::vector<DataRecordingInformation> VectDRI;
		std::vector<OpenViBE::uint32> startingPoints;
		
	public : 
		void reset();
		void setRecordingChannel(int count, OpenViBE::uint32 desired);
		OpenViBE::uint32 iStart(int idx);
		void addiStart(OpenViBE::uint32 DataSize, int idx);
		void addData(double *l_pSampleDb,OpenViBE::uint32 DataSize, int idx);
		bool sendData(OpenViBE::float32 *);
		
};
	
#endif // 

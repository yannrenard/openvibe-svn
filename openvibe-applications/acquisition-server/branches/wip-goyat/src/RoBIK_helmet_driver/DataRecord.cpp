#include "DataRecord.h"
#include <iostream>

DataRecordingInformation::DataRecordingInformation()
{
 data=NULL;
 dataSize=0;
 dataDesiredSize=0;
 dataMaxSize=0;
}

DataRecordingInformation::DataRecordingInformation(const DataRecordingInformation& dri)
{
 dataMaxSize=dri.dataMaxSize;
 dataDesiredSize=dri.dataDesiredSize;
 dataSize=dri.dataSize;
 data=new OpenViBE::float32[dataMaxSize];
 for(OpenViBE::uint32 i=0; i<dataSize; i++)
   {data[i]=dri.data[i];}
}

DataRecordingInformation::~DataRecordingInformation()
{
 delete [] data;
 data=NULL;
}

DataRecordingInformation& DataRecordingInformation::operator=(const DataRecordingInformation& dri)
{
 if(&dri==this) {return *this;}
 
 dataMaxSize=dri.dataMaxSize;
 dataDesiredSize=dri.dataDesiredSize;
 dataSize=dri.dataSize;
 if(data) {delete [] data;}
 data=new OpenViBE::float32[dataMaxSize];
 for(OpenViBE::uint32 i=0; i<dataSize; i++)
   {data[i]=dri.data[i];}
 
 return *this;
}

void DataRecordingInformation::restartDoubleBuffer()
{
 for(OpenViBE::uint32 i=dataDesiredSize; i<dataSize; i++)
   {data[i-dataDesiredSize]=data[i];}
   
 dataSize=(dataSize-dataDesiredSize>=0 ? dataSize-dataDesiredSize : dataSize); 
}

bool DataRecordingInformation::isFull()
{
 return dataSize>=dataDesiredSize ? true : false;
}

OpenViBE::uint32 DataRecordingInformation::Size()
{
 return dataSize;
}

OpenViBE::uint32 DataRecordingInformation::setMaxSize(OpenViBE::uint32 size)
{
 //adjust size
 if(size<dataDesiredSize) {size=dataDesiredSize;}
 if(size<dataSize) {size=dataSize;}
 dataMaxSize=size;
 
 //save data
 OpenViBE::float32 *dataTmp=new OpenViBE::float32[dataSize];
 for(OpenViBE::uint32 i=0; i<dataSize; i++)
   {dataTmp[i]=data[i];}
 //do
 if(data) {delete [] data;}
 data=new OpenViBE::float32[dataMaxSize];
 for(OpenViBE::uint32 i=0; i<dataSize; i++)
   {data[i]=dataTmp[i];}  
 //release
 delete[] dataTmp;
 
 return size;
}

void DataRecordingInformation::setDesiredSize(OpenViBE::uint32 size)
{
 if(size>dataMaxSize) {setMaxSize(size);}
 dataDesiredSize=size;
}

void DataRecordingInformation::addData(OpenViBE::float32 buffer)
{
 if(dataMaxSize<=dataSize) {setMaxSize(dataMaxSize+1);}
 data[dataSize]=buffer;
 dataSize++;
}

void DataRecordingInformation::addData(OpenViBE::float32 *buffer,OpenViBE::uint32 size)
{
 if(dataMaxSize<=dataSize-1+size) 
   {
	setMaxSize(dataMaxSize+size);
   }
 for(OpenViBE::uint32 i=0; i<size; i++)
   {
    data[dataSize]=buffer[i];
    dataSize++;
   }
}

OpenViBE::float32* DataRecordingInformation::concatData(OpenViBE::float32 *buffer)
{
 if(!isFull()) {return buffer+dataDesiredSize;}
 for(OpenViBE::uint32 i=0; i<dataDesiredSize; i++)
   {
    buffer[0]=data[i];
    buffer++;
   }
 return buffer;
}


//----------------------------------------------------------------------//
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


DataReader::DataReader()
{
}

void DataReader::setRecordingChannel(int count, OpenViBE::uint32 desired)
{
 DataRecordingInformation rdi;
 rdi.setDesiredSize(desired);
 VectDRI=std::vector<DataRecordingInformation>(count,rdi);
 startingPoints=std::vector<OpenViBE::uint32>(count,0);
}

OpenViBE::uint32 DataReader::iStart(int idx)
{
 if(startingPoints.empty()) {return -1;}
 return idx>=int(startingPoints.size())? -1 : startingPoints[idx];
}

void DataReader::addiStart(OpenViBE::uint32 DataSize, int idx)
{
 if(idx<0 || idx>=int(startingPoints.size())) {return;}
 startingPoints[idx]=startingPoints[idx]+DataSize;
}

void DataReader::addData(double *l_pSampleDb,OpenViBE::uint32 DataSize, int idx)
{
 ///security
 if(DataSize<=0)  {return;}
 //fill data
 OpenViBE::float32 *data=new OpenViBE::float32[DataSize];
 for(int i=0; i<DataSize; i++)
   {
    data[i]=OpenViBE::float32(l_pSampleDb[i]);
   }
 //copy data
 VectDRI[idx].addData(data,DataSize);
 //release
 delete[] data;
}

bool DataReader::sendData(OpenViBE::float32 *buffer)
{
   for(unsigned int i=0; i<VectDRI.size(); i++)
    {
	 if(!VectDRI[i].isFull()) {return false;}
	}
	
  for(unsigned int i=0; i<VectDRI.size(); i++)
    {
	 buffer=VectDRI[i].concatData(buffer);
	 VectDRI[i].restartDoubleBuffer();
	}
	
 return true;
}

		
		

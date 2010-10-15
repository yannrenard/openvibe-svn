#pragma once

#include "TcpConnection.h"

#include <openvibe/ov_all.h>

class ParamsRoot
{
public:
	ParamsRoot();
	virtual ~ParamsRoot();

	virtual void	Dump() = 0;
	virtual bool	SetParams(int argc, char* argv[]) = 0;
	virtual void	Usage() = 0;
	virtual void	Process() = 0;
	virtual void	Build();
	bool	HasClient()
			{	
				return myTcpConnection.HasClient();
			}
	bool	HasListener()
			{	
				return myTcpConnection.HasListener();
			}
	bool	Listen()
			{	
				return myTcpConnection.Listen(serverPort);
			}
	bool	Accept()
			{	
				return myTcpConnection.Accept();
			}
	template <class type> bool	Send(const type value)
			{
				return myTcpConnection.Send(&value, sizeof(type));
			}
	bool	Send(const void* data, socklen_t dataLen)
			{
				return myTcpConnection.Send(data, dataLen);
			}
	bool	Receive(const void* data, socklen_t dataLen)
			{
				return myTcpConnection.Receive(data, dataLen);
			}
	void	DumpData();
	int		ChunkSize() const	{	return chunkSize;		}
protected:
	int						serverPort;
	int						nbChannels;
	int						chunkSize;
	int						sendWait;
	int						samplingRate;
	int						otherSamplingRate;
	int						interpolationMode;

	OpenViBE::float32*		data;
	int						dataSize;

	TcpConnection			myTcpConnection;
};

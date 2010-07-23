#pragma once

namespace OpenViBE
{
	typedef	unsigned long long	uint64;
	typedef	long				int32;
	typedef	unsigned long		uint32;
	typedef	short				int16;
	typedef	unsigned short		uint16;
	typedef	float				float32;
	typedef	double				float64;
	typedef	bool				boolean;

	class IMatrix
	{
	public:
		IMatrix()
			: nbDim(0)
			, data(0)
		{
			sizeDim[0]	= 0;
			sizeDim[1]	= 0;
		}
		IMatrix(const uint32 _size1, const uint32 _size2)
			: data(0)
		{
			sizeDim[0]	= 0;
			sizeDim[1]	= 0;
			Build(_size1, _size2);
		}
		~IMatrix()
		{
			Clear();
		}

		void Clear()
		{
			delete data;
		}
		void	Build(const uint32 _size1, const uint32 _size2)
				{
					Clear();

					setDimensionCount(2);
					setDimensionSize(0, _size1);
					setDimensionSize(1, _size2);
				}
		void	setDimensionCount(const uint32 _nbDim)
				{
					delete	data;
					
					data	= 0;
					nbDim	= _nbDim;
				}
		void	setDimensionSize(const uint32 dim, const uint32 size)
				{
					sizeDim[dim]	= size;

					if(dim == (nbDim - 1))
					{	uint32	totSize = 1;
						for(uint32 ii=0; ii < nbDim; ii++)
							totSize	*= sizeDim[ii];
						
						float32* data_	= new float32[totSize];
						data	= data_;
					}
				}
		uint32	getDimensionSize(const uint32 dim) const
				{
					return sizeDim[dim];
				}
		uint32	getBufferElementCount() const
				{
					return getBuffer() ? *sizeDim * *(sizeDim+1): 0;
				}
		float32*	getBuffer() const {	return data;	}

	protected:
		uint32		nbDim;
		uint32		sizeDim[2];
		float32*	data;
	};

	class CMatrix : public IMatrix
	{
	public:
		CMatrix()
		{
		}
		CMatrix(const uint32 _size1, const uint32 _size2)
			: IMatrix(_size1, _size2)
		{
		}
	};
	
	namespace Kernel
	{
	};
	namespace Plugins
	{
	};
	
};

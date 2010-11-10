#ifndef __OpenViBEApplication_CLog_H__
#define __OpenViBEApplication_CLog_H__

#include <iostream>

namespace OpenViBESSVEP
{

	class CLog
	{
		private:
			CLog() {};
			~CLog() {};
			CLog(const CLog&);
			CLog& operator=(const CLog&);

		public:
			static CLog log;
	};

	template <typename T> CLog& operator <<(CLog& log, T const& value)
	{
		std::cout << value << std::endl;
		return log;
	}
	

}
#endif // __OpenViBEApplication_CLog_H__

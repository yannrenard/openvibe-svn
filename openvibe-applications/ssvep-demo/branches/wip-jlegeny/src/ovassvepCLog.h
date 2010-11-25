#ifndef __OpenViBEApplication_CLog_H__
#define __OpenViBEApplication_CLog_H__

#include <iostream>

namespace OpenViBESSVEP
{

	class CLog
	{
		public:
			static CLog log;
			static CLog err;

			template <typename T>
				CLog& operator<<(const T& x)
				{
					std::cout << x;

					return *this;
				}


			typedef CLog& (*CLogManipulator)(CLog&);

			CLog& operator<<(CLogManipulator manip)
			{
				return manip(*this);
			}

			static CLog& endl(CLog& stream)
			{
				std::cout << std::endl;

				stream << std::endl;

				return stream;
			}

			typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

			typedef CoutType& (*StandardEndLine)(CoutType&);

			CLog& operator<<(StandardEndLine manip)
			{
				manip(std::cout);

				return *this;
			}
	};

}
#endif // __OpenViBEApplication_CLog_H__

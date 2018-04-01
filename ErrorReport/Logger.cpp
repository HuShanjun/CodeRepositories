#include "stdafx.h"
#include "Logger.h"
#include <time.h>  
#include <stdarg.h>  
#include <direct.h>  
#include <vector>  
#include <Dbghelp.h>  
//#include <windows.h>

#pragma comment(lib,"Dbghelp.lib")  

using std::string;
using std::vector;

static const int MAX_LEN = 1024;		//默认最大字节数

namespace LOGGER
{
	CLogger::CLogger(const std::string strLogName, const std::string strLogPath, EnumLogLevel nLogLevel)
		:m_nLogLevel(nLogLevel),
		m_strLogPath(strLogPath),
		m_strLogName(strLogName)
	{
		//初始化  
		m_pFileStream = NULL;
		if (m_strLogPath.empty())
		{
			m_strLogPath = GetAppPathA();
		}
		if (m_strLogPath[m_strLogPath.length() - 1] != '\\')
		{
			m_strLogPath.append("\\");
		}
		//创建文件夹  
		MakeSureDirectoryPathExists(m_strLogPath.c_str());
		//创建日志文件  
		if (m_strLogName.empty())
		{
			time_t curTime;
			time(&curTime);
			tm tm1;
			localtime_s(&tm1, &curTime);
			//日志的名称如：201601012130.log  
			m_strLogName = FormatString("%04d%02d%02d_%02d%02d%02d.log", tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
		}
		m_strLogFilePath = m_strLogPath.append(m_strLogName);

		//以追加的方式打开文件流  
		fopen_s(&m_pFileStream, m_strLogFilePath.c_str(), "a+");

		InitializeCriticalSection(&m_cs);
	}


	//析构函数  
	CLogger::~CLogger()
	{
		//释放临界区  
		DeleteCriticalSection(&m_cs);
		//关闭文件流  
		if (m_pFileStream)
		{
			fclose(m_pFileStream);
			m_pFileStream = NULL;
		}
	}

	//文件全路径得到文件名  
	const char *CLogger::path_file(const char *path, char splitter)
	{
		return strrchr(path, splitter) ? strrchr(path, splitter) + 1 : path;
	}

	//写严重错误信息  
	void CLogger::TraceFatal(const char *lpcszFormat, ...)
	{
		//判断当前的写日志级别  
		if (EnumLogLevel::LogLevel_Fatal > m_nLogLevel)
			return;
		string strResult;
		if (NULL != lpcszFormat)
		{
			va_list marker = NULL;
			va_start(marker, lpcszFormat); //初始化变量参数  
			size_t nLength = _vscprintf(lpcszFormat, marker) + 1; //获取格式化字符串长度  
			std::vector<char> vBuffer(nLength, '\0'); //创建用于存储格式化字符串的字符数组  
			int nWritten = _vsnprintf_s(&vBuffer[0], vBuffer.size(), nLength, lpcszFormat, marker);
			if (nWritten > 0)
			{
				strResult = &vBuffer[0];
			}
			va_end(marker); //重置变量参数  
		}
		if (strResult.empty())
		{
			return;
		}
		string strLog = strFatalPrefix;
		strLog.append(GetTime()).append(strResult);

		//写日志文件  
		Trace(strLog);
	}

	//写错误信息  
	void CLogger::TraceError(const char *lpcszFormat, ...)
	{
		//判断当前的写日志级别  
		if (EnumLogLevel::LogLevel_Error > m_nLogLevel)
			return;
		string strResult;
		if (NULL != lpcszFormat)
		{
			va_list marker = NULL;
			va_start(marker, lpcszFormat); //初始化变量参数  
			size_t nLength = _vscprintf(lpcszFormat, marker) + 1; //获取格式化字符串长度  
			std::vector<char> vBuffer(nLength, '\0'); //创建用于存储格式化字符串的字符数组  
			int nWritten = _vsnprintf_s(&vBuffer[0], vBuffer.size(), nLength, lpcszFormat, marker);
			if (nWritten > 0)
			{
				strResult = &vBuffer[0];
			}
			va_end(marker); //重置变量参数  
		}
		if (strResult.empty())
		{
			return;
		}
		string strLog = strErrorPrefix;
		strLog.append(GetTime()).append(strResult);

		//写日志文件  
		Trace(strLog);
	}

	//写警告信息  
	void CLogger::TraceWarning(const char *lpcszFormat, ...)
	{
		//判断当前的写日志级别  
		if (EnumLogLevel::LogLevel_Warning > m_nLogLevel)
			return;
		string strResult;
		if (NULL != lpcszFormat)
		{
			va_list marker = NULL;
			va_start(marker, lpcszFormat); //初始化变量参数  
			size_t nLength = _vscprintf(lpcszFormat, marker) + 1; //获取格式化字符串长度  
			std::vector<char> vBuffer(nLength, '\0'); //创建用于存储格式化字符串的字符数组  
			int nWritten = _vsnprintf_s(&vBuffer[0], vBuffer.size(), nLength, lpcszFormat, marker);
			if (nWritten > 0)
			{
				strResult = &vBuffer[0];
			}
			va_end(marker); //重置变量参数  
		}
		if (strResult.empty())
		{
			return;
		}
		string strLog = strWarningPrefix;
		strLog.append(GetTime()).append(strResult);

		//写日志文件  
		Trace(strLog);
	}


	//写一般信息  
	void CLogger::TraceInfo(const char *lpcszFormat, ...)
	{
		//判断当前的写日志级别  
		if (EnumLogLevel::LogLevel_Info > m_nLogLevel)
			return;
		string strResult;
		if (NULL != lpcszFormat)
		{
			va_list marker = NULL;
			va_start(marker, lpcszFormat); //初始化变量参数  
			size_t nLength = _vscprintf(lpcszFormat, marker) + 1; //获取格式化字符串长度  
			std::vector<char> vBuffer(nLength, '\0'); //创建用于存储格式化字符串的字符数组  
			int nWritten = _vsnprintf_s(&vBuffer[0], vBuffer.size(), nLength, lpcszFormat, marker);
			if (nWritten > 0)
			{
				strResult = &vBuffer[0];
			}
			va_end(marker); //重置变量参数  
		}
		if (strResult.empty())
		{
			return;
		}
		string strLog = strInfoPrefix;
		strLog.append(GetTime()).append(strResult);

		//写日志文件  
		Trace(strLog);
	}

	//获取系统当前时间  
	string CLogger::GetTime()
	{
		time_t curTime;
		time(&curTime);
		tm tm1;
		localtime_s(&tm1, &curTime);
		//2016-01-01 21:30:00  
		string strTime = FormatString("[%04d-%02d-%02d %02d:%02d:%02d] ", tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);

		return strTime;
	}

	//改变写日志级别  
	void CLogger::ChangeLogLevel(EnumLogLevel nLevel)
	{
		m_nLogLevel = nLevel;
	}

	//写文件操作  
	void CLogger::Trace(const string &strLog)
	{
		try
		{
			//进入临界区  
			EnterCriticalSection(&m_cs);
			//若文件流没有打开，则重新打开  
			if (NULL == m_pFileStream)
			{
				fopen_s(&m_pFileStream, m_strLogFilePath.c_str(), "a+");
				if (!m_pFileStream)
				{
					return;
				}
			}
			//写日志信息到文件流  
			fprintf(m_pFileStream, "%s\n", strLog.c_str());
			fflush(m_pFileStream);
			//离开临界区  
			LeaveCriticalSection(&m_cs);
		}
		//若发生异常，则先离开临界区，防止死锁  
		catch (...)
		{
			LeaveCriticalSection(&m_cs);
		}
	}

	string CLogger::GetAppPathA()
	{
		char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 }, szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, szFilePath, sizeof(szFilePath));
		_splitpath_s(szFilePath, szDrive, szDir, szFileName, szExt);

		string str(szDrive);
		str.append(szDir);
		return str;
	}

	string CLogger::FormatString(const char *lpcszFormat, ...)
	{
		string strResult;
		if (NULL != lpcszFormat)
		{
			va_list marker = NULL;
			va_start(marker, lpcszFormat); //初始化变量参数  
			size_t nLength = _vscprintf(lpcszFormat, marker) + 1; //获取格式化字符串长度  
			std::vector<char> vBuffer(nLength, '\0'); //创建用于存储格式化字符串的字符数组  
			int nWritten = _vsnprintf_s(&vBuffer[0], vBuffer.size(), nLength, lpcszFormat, marker);
			if (nWritten > 0)
			{
				strResult = &vBuffer[0];
			}
			va_end(marker); //重置变量参数  
		}
		return strResult;
	}

	//字符串转化相关函数
	int UnicodeToAnsi(LPSTR szAnsi, LPCWSTR wstrUnicode)
	{
		DWORD dwMinSize = 0;
		//转化后需要多少多字节来存储
		dwMinSize = WideCharToMultiByte(CP_OEMCP, 0, wstrUnicode, -1, NULL, 0, NULL, FALSE);
		if (0 == dwMinSize)
		{
			return 0;
		}

		WideCharToMultiByte(CP_OEMCP, 0, wstrUnicode, -1, szAnsi, dwMinSize, NULL, FALSE);

		return dwMinSize;
	}
	int AnsiToUnicode(LPWSTR wstrUnicode, LPCSTR szAnsi)
	{
		DWORD dwMinSize = 0;
		dwMinSize = MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, NULL, 0);
		if (0 == dwMinSize)
		{
			return 0;
		}
		MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, wstrUnicode, dwMinSize);

		return dwMinSize;
	}

	int UTF8ToUnicode(LPWSTR wstrUnicoe, LPCSTR szUTF8)
	{
		//转换后Unicode的长度  

		DWORD dwMinSize = MultiByteToWideChar(CP_UTF8, 0, szUTF8, -1, NULL, 0);

		if (0 != dwMinSize)
		{
			//转为Unicode  
			dwMinSize = MultiByteToWideChar(CP_UTF8, 0, szUTF8, -1, wstrUnicoe, dwMinSize);

			return dwMinSize;
		}

		return 0;
	}

	int UnicodeToUTF8(LPSTR szUTF8, LPCWSTR wszUnicode)
	{
		int dwMinSize = WideCharToMultiByte(CP_UTF8, 0, wszUnicode, -1, NULL, 0, NULL, NULL);
		if (dwMinSize)
		{
			dwMinSize = WideCharToMultiByte(CP_UTF8, 0, wszUnicode, -1, szUTF8, dwMinSize, NULL, NULL);
		}

		return dwMinSize;
	}


	int UTF8ToAnsi(LPSTR szAnsi, LPCSTR szUTF8)
	{
		wchar_t szUnicode[1024] = L"";
		UTF8ToUnicode(szUnicode, szUTF8);

		return UnicodeToAnsi(szAnsi, szUnicode);
	}

	int AnsiToUTF8(LPSTR szUTF8, LPCSTR szAnsi)
	{
		wchar_t szUnicode[MAX_LEN] = L"";
		AnsiToUnicode(szUnicode, szAnsi);

		return UnicodeToUTF8(szUTF8, szUnicode);
	}


	std::string UnicodeToAnsi(const std::wstring wstrUnicode)
	{
		char szAnsi[MAX_LEN] = "";
		UnicodeToAnsi(szAnsi, wstrUnicode.data());

		return std::string(szAnsi);
	}

	std::wstring AnsiToUnicode(const std::string szAnsi)
	{
		wchar_t szUnicoe[MAX_LEN] = L"";
		AnsiToUnicode(szUnicoe, szAnsi.data());

		return std::wstring(szUnicoe);

	}
	std::wstring UTF8ToUnicode(const std::string szUTF8)
	{
		wchar_t szUnicode[MAX_LEN] = L"";
		UTF8ToUnicode(szUnicode, szUTF8.data());

		return std::wstring(szUnicode);
	}

	std::string UnicodeToUTF8(const std::wstring wszUnicode)
	{
		char szutf8[MAX_LEN] = "";
		UnicodeToUTF8(szutf8, wszUnicode.data());

		return std::string(szutf8);
	}

	std::string UTF8ToAnsi(const std::string szUTF8)
	{
		char szAnsi[MAX_LEN] = "";
		UTF8ToAnsi(szAnsi, szUTF8.data());

		return std::string(szAnsi);

	}

	std::string AnsiToUTF8(const std::string szAnsi)
	{
		char szUtf8[MAX_LEN] = "";
		AnsiToUTF8(szUtf8, szAnsi.data());

		return std::string(szUtf8);
	}
}

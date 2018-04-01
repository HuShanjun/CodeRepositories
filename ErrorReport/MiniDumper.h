#pragma  once   
  
#include <windows.h>
#include <dbghelp.h> 
#include <vector>
  

const int MAX_ADDRESS_LENGTH = 32;
const int MAX_NAME_LENGTH = 1024;

// 崩溃信息  
//   
struct CrashInfo
{
	CHAR ErrorCode[MAX_ADDRESS_LENGTH];
	CHAR Address[MAX_ADDRESS_LENGTH];
	CHAR Flags[MAX_ADDRESS_LENGTH];
};

// CallStack信息  
//  
struct CallStackInfo
{
	CHAR ModuleName[MAX_NAME_LENGTH];
	CHAR MethodName[MAX_NAME_LENGTH];
	CHAR FileName[MAX_NAME_LENGTH];
	CHAR LineNumber[MAX_NAME_LENGTH];
};



class CMiniDumper  
{  
public:  
    static HRESULT  CreateInstance();  
    static HRESULT  ReleaseInstance();  
  
public:  
    LONG WriteMiniDump(_EXCEPTION_POINTERS *pExceptionInfo);  
  
private:  
	//设置dump文件名称
    void SetMiniDumpFileName(void);  
    BOOL GetImpersonationToken(HANDLE* phToken); 
	//
    BOOL EnablePrivilege(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld); 

    BOOL RestorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld); 
	//禁止设置异常处理函数
	void DisableSetUnhandledExceptionFilter();
	//安全拷贝字符串
	void SafeStrCpy(char* szDest, size_t nMaxDestSize, const char* szSrc);
	// 得到程序崩溃信息   
	CrashInfo GetCrashInfo(const EXCEPTION_RECORD *pRecord);
	// 得到CallStack信息  
	std::vector<CallStackInfo> GetCallStack(const CONTEXT *pContext);
	//写崩溃调用栈信息
	LONG  CMiniDumper::WriteApplicationCrashInfo(EXCEPTION_POINTERS *pException);
	

private:
	//异常回调函数
	static LONG WINAPI UnhandledExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo);
	//写dump回调
	static BOOL WINAPI MiniDumpCallback(PVOID  pParam, const PMINIDUMP_CALLBACK_INPUT   pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput);
	//判断模块是否存在
	static BOOL IsDataSectionNeeded(const WCHAR* pModuleName);

private:  
    CMiniDumper();  
    virtual ~CMiniDumper(void);  
  
private:  
    TCHAR   m_szMiniDumpPath[MAX_PATH]; 
	//TCHAR	m_szCrashInfoPath[MAX_PATH];
    TCHAR   m_szAppPath[MAX_PATH]; 

	static CMiniDumper*			gs_pMiniDumper;
	static LPCRITICAL_SECTION	gs_pCriticalSection;
};  
#include "stdafx.h"  
#include <windows.h>  
#include <stdio.h>  
#include <assert.h>  
#include <time.h>  
#include <tchar.h>  
#include "MiniDumper.h"  
#include "Logger.h"
  
#ifdef UNICODE  
    #define _tcssprintf wsprintf  
    #define tcsplitpath _wsplitpath  
#else  
    #define _tcssprintf sprintf  
    #define tcsplitpath _splitpath  
#endif  
  
  
//-----------------------------------------------------------------------------  
// GLOBALs  
//-----------------------------------------------------------------------------  
CMiniDumper*		CMiniDumper::gs_pMiniDumper = NULL;
LPCRITICAL_SECTION	CMiniDumper::gs_pCriticalSection = NULL;
  
//-----------------------------------------------------------------------------  
// APIs  
//-----------------------------------------------------------------------------  
// Based on dbghelp.h  
typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess,  
                                        DWORD dwPid,  
                                        HANDLE hFile,  
                                        MINIDUMP_TYPE DumpType,  
                                        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,  
                                        CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,  
                                        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);  
 

BOOL CMiniDumper::IsDataSectionNeeded(const WCHAR* pModuleName)
{  
    if (pModuleName == 0)  
    {  
        return FALSE;  
    }  
  
    WCHAR szFileName[_MAX_FNAME] = {0};  
    _wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);  
  
    if (_wcsicmp(szFileName, L"ntdll") == 0)  
        return TRUE;  
  
    return FALSE;  
}  
  
BOOL WINAPI CMiniDumper::MiniDumpCallback(PVOID                  pParam,
                                const PMINIDUMP_CALLBACK_INPUT   pInput,  
                                PMINIDUMP_CALLBACK_OUTPUT        pOutput)  
{  
    if (pInput == 0 || pOutput == 0)  
        return FALSE;  
  
    switch (pInput->CallbackType)  
    {  
    case ModuleCallback:  
        if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg)  
        {  
            if (!IsDataSectionNeeded(pInput->Module.FullPath))  
                pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);  
        }  
        return TRUE;  
    case IncludeModuleCallback:  
    case IncludeThreadCallback:  
    case ThreadCallback:  
    case ThreadExCallback:  
        return TRUE;  
    default:;  
    }  
  
    return FALSE;  
}  
  
//-----------------------------------------------------------------------------  
// Name: unhandledExceptionHandler()  
// Desc: Call-back filter function for unhandled exceptions  
//-----------------------------------------------------------------------------  
LONG WINAPI CMiniDumper::UnhandledExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo)
{  
    if (NULL == gs_pMiniDumper)  
        return EXCEPTION_CONTINUE_SEARCH;  

	gs_pMiniDumper->WriteApplicationCrashInfo(pExceptionInfo);

	return gs_pMiniDumper->WriteMiniDump(pExceptionInfo);

	//重启应用


	//return EXCEPTION_EXECUTE_HANDLER;
}  
  
// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效    
void CMiniDumper::DisableSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	if (hKernel32 == NULL)
		return;
	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL)
		return;
	unsigned char newJump[100];
	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far   
	void *pNewFunc = &UnhandledExceptionHandler;
	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
	newJump[0] = 0xE9;  // JMP absolute   
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc)+1, &bytesWritten);

}
  
//-----------------------------------------------------------------------------  
// Name: CreateInstance()  
// Desc: Instanse gs_pMiniDumper  
//-----------------------------------------------------------------------------  
HRESULT CMiniDumper::CreateInstance()  
{  
    if (NULL == gs_pMiniDumper)  
    {  
        gs_pMiniDumper = new CMiniDumper();  
    }  
    if (NULL == gs_pCriticalSection)  
    {  
        gs_pCriticalSection = new CRITICAL_SECTION;  
        InitializeCriticalSection(gs_pCriticalSection);  
    }  
     
    return(S_OK);  
}  
  
//-----------------------------------------------------------------------------  
// Name: ReleaseInstance()  
// Desc: Release gs_pMiniDumper  
//-----------------------------------------------------------------------------  
HRESULT  CMiniDumper::ReleaseInstance()  
{  
    if (NULL != gs_pMiniDumper)  
    {  
        delete gs_pMiniDumper;  
        gs_pMiniDumper = NULL;  
    }  
    if (NULL != gs_pCriticalSection)  
    {  
        DeleteCriticalSection(gs_pCriticalSection);  
        gs_pCriticalSection = NULL;  
    }  
  
    return(S_OK);  
}  
  
//-----------------------------------------------------------------------------  
// Name: CMiniDumper()  
// Desc: Constructor  
//-----------------------------------------------------------------------------  
CMiniDumper::CMiniDumper()  
{  
    // 使应用程序能够取代每个进程和线程的顶级异常处理程序      
    ::SetUnhandledExceptionFilter(UnhandledExceptionHandler);  
    DisableSetUnhandledExceptionFilter();  
}  
  
//析构函数
CMiniDumper::~CMiniDumper(void)  
{  
  
}  
  
//设置dump文件名称
void CMiniDumper::SetMiniDumpFileName(void)  
{  
    time_t currentTime;  
	time(&currentTime);
	tm tm1;
	localtime_s(&tm1, &currentTime);
  
	TCHAR szAppName[MAX_PATH] = { 0 };
	TCHAR szFileName[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szAppName, MAX_PATH);
	_wsplitpath(szAppName, NULL, NULL, szFileName, NULL);

	TCHAR szTime[64] = { 0 };
	wcsftime(szTime, 64, L"%Y%m%d%H%M%S", &tm1);

	_tcssprintf(m_szMiniDumpPath, _T("%s-%s.dmp"), szFileName,szTime);
}  
  
//-----------------------------------------------------------------------------  
// Name: getImpersonationToken()  
// Desc: The method acts as a potential workaround for the fact that the   
//       current thread may not have a token assigned to it, and if not, the   
//       process token is received.  
//-----------------------------------------------------------------------------  
BOOL CMiniDumper::GetImpersonationToken(HANDLE* phToken)  
{  
    *phToken = NULL;  
    if (!OpenThreadToken(GetCurrentThread(),   
                         TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,  
                         TRUE,  
                         phToken))  
    {  
        if (GetLastError() == ERROR_NO_TOKEN)  
        {  
            // No impersonation token for the current thread is available.   
            // Let's go for the process token instead.  
            if (!OpenProcessToken(GetCurrentProcess(),  
                                  TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,  
                                  phToken))  
                return FALSE;  
        }  
        else  
            return FALSE;  
    }  
  
    return TRUE;  
}  
  
//-----------------------------------------------------------------------------  
// Name: enablePrivilege()  
// Desc: Since a MiniDump contains a lot of meta-data about the OS and   
//       application state at the time of the dump, it is a rather privileged   
//       operation. This means we need to set the SeDebugPrivilege to be able   
//       to call MiniDumpWriteDump.  
//-----------------------------------------------------------------------------  
BOOL CMiniDumper::EnablePrivilege(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld)  
{  
    BOOL                bOk = FALSE;  
    TOKEN_PRIVILEGES    tp;  
  
    tp.PrivilegeCount = 1;  
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  
    bOk = LookupPrivilegeValue(0, pszPriv, &tp.Privileges[0].Luid);  
    if (bOk)  
    {  
        DWORD cbOld = sizeof(*ptpOld);  
        bOk = AdjustTokenPrivileges(hToken, FALSE, &tp, cbOld, ptpOld, &cbOld);  
    }  
  
    return (bOk && (ERROR_NOT_ALL_ASSIGNED != GetLastError()));  
}  
  
//-----------------------------------------------------------------------------  
// Name: restorePrivilege()  
// Desc:   
//-----------------------------------------------------------------------------  
BOOL CMiniDumper::RestorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld)  
{  
    BOOL bOk = AdjustTokenPrivileges(hToken, FALSE, ptpOld, 0, NULL, NULL);  
    return (bOk && (ERROR_NOT_ALL_ASSIGNED != GetLastError()));  
}  
  
//-----------------------------------------------------------------------------  
// Name: writeMiniDump()  
// Desc:   
//-----------------------------------------------------------------------------  
LONG CMiniDumper::WriteMiniDump(_EXCEPTION_POINTERS *pExceptionInfo)  
{  
    LONG    retval              = EXCEPTION_CONTINUE_SEARCH;  
    HANDLE  hImpersonationToken = NULL;  
  
    if (!GetImpersonationToken(&hImpersonationToken))  
        return FALSE;  
  
    // You have to find the right dbghelp.dll.   
    // Look next to the EXE first since the one in System32 might be old (Win2k)  
    HMODULE hDll                    = NULL;  
    if (GetModuleFileName(NULL, m_szAppPath, _MAX_PATH))  
    {  
        wchar_t szDir[_MAX_DIR]         = { 0 };  
        TCHAR   szDbgHelpPath[MAX_PATH] = { 0 };  
  
        _wsplitpath(m_szAppPath, NULL, szDir, NULL, NULL);  
        _tcscpy(szDbgHelpPath, szDir);        
        _tcscat(szDbgHelpPath, _T("DBGHELP.DLL"));  
  
        hDll = ::LoadLibrary(szDbgHelpPath);  
    }  
  
    if (hDll == NULL)  
    {  
        // If we haven't found it yet - try one more time.  
        hDll = ::LoadLibrary(_T("DBGHELP.DLL"));  
    }  
  
    if (hDll)  
    {  
        // Get the address of the MiniDumpWriteDump function, which writes   
        // user-mode mini-dump information to a specified file.  
        MINIDUMPWRITEDUMP MiniDumpWriteDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");  
        if (MiniDumpWriteDump != NULL)  
        {  
            SetMiniDumpFileName();  
  
            // Create the mini-dump file...  
            HANDLE hFile = ::CreateFile(m_szMiniDumpPath,  
                                        GENERIC_WRITE,  
                                        FILE_SHARE_WRITE,  
                                        NULL,  
                                        CREATE_ALWAYS,  
                                        FILE_ATTRIBUTE_NORMAL,  
                                        NULL);  
  
            if (hFile != INVALID_HANDLE_VALUE)  
            {  
                _MINIDUMP_EXCEPTION_INFORMATION ExInfo;  
  
                ExInfo.ThreadId          = ::GetCurrentThreadId();  
                ExInfo.ExceptionPointers = pExceptionInfo;  
                ExInfo.ClientPointers    = NULL;  
  
                MINIDUMP_CALLBACK_INFORMATION mci;  
                mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;  
                mci.CallbackParam = 0;  
  
                // We need the SeDebugPrivilege to be able to run MiniDumpWriteDump  
                TOKEN_PRIVILEGES tp;  
                BOOL bPrivilegeEnabled = EnablePrivilege(SE_DEBUG_NAME, hImpersonationToken, &tp);  
                BOOL bOk;  
  
                // DBGHELP.dll is not thread-safe, so we need to restrict access...  
                EnterCriticalSection(gs_pCriticalSection);  
                {  
                    // Write out the mini-dump data to the file...  
                    bOk = MiniDumpWriteDump(GetCurrentProcess(),  
                                            GetCurrentProcessId(),  
                                            hFile,  
                                            MiniDumpNormal,  
                                            (NULL == pExceptionInfo) ? (NULL) : (&ExInfo),  
                                            NULL,  
                                            &mci);  
                }  
                LeaveCriticalSection(gs_pCriticalSection);  
  
                // Restore the privileges when done  
                if (bPrivilegeEnabled)  
                    RestorePrivilege(hImpersonationToken, &tp);  
  
                if (bOk)  
                {  
                    retval = EXCEPTION_EXECUTE_HANDLER;                   
                }                 
  
                ::CloseHandle(hFile);  
            }             
        }         
    }  
  
    FreeLibrary(hDll);  

	/*char szFileName[_MAX_PATH];
	::GetModuleFileNameA(NULL, szFileName, _MAX_PATH);

	char szCmdLine[64] = { 0 };
	sprintf(szCmdLine, "%s Start", szFileName);

	WinExec(szCmdLine, SW_SHOW);*/

    if (NULL != pExceptionInfo)  
    {  
        TerminateProcess(GetCurrentProcess(), 0);  
    }  
  
    return retval;  
}  

// 写奔溃信息 
//  
LONG  CMiniDumper::WriteApplicationCrashInfo(EXCEPTION_POINTERS *pException)
{
	// 确保有足够的栈空间  
	//  
#ifdef _M_IX86  
	if (pException->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
	{
		static char TempStack[1024 * 128];
		__asm mov eax, offset TempStack[1024 * 128];
		__asm mov esp, eax;
	}
#endif  

	//设置错误报告名字
	time_t currentTime;
	time(&currentTime);
	tm tm1;
	localtime_s(&tm1, &currentTime);

	char szAppName[MAX_PATH] = { 0 };
	char szFileName[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szAppName, MAX_PATH);
	_splitpath(szAppName, NULL, NULL, szFileName, NULL);

	char szTime[64] = { 0 };
	strftime(szTime, 64, "%Y%m%d%H%M%S", &tm1);
	char szLogFimeName[64] = { 0 };
	sprintf_s(szLogFimeName, "%sCrashInfo-%s.log", szFileName, szTime);

	LOGGER::CLogger myLog(szLogFimeName);

	CrashInfo crashinfo = GetCrashInfo(pException->ExceptionRecord);

	// 输出Crash信息  
	//  
	//cout << "ErrorCode: " << crashinfo.ErrorCode << endl;
	//cout << "Address: " << crashinfo.Address << endl;
	//cout << "Flags: " << crashinfo.Flags << endl;

	myLog.TraceInfo("ErrorCode: %s", crashinfo.ErrorCode);
	myLog.TraceInfo("Address: %s", crashinfo.Address);
	myLog.TraceInfo("Flags: %s", crashinfo.Flags);

	std::vector<CallStackInfo> arrCallStackInfo = GetCallStack(pException->ContextRecord);

	// 输出CallStack  
	//  
	//cout << "CallStack: " << endl;
	myLog.TraceInfo("CallStack:");
	for (std::vector<CallStackInfo>::iterator i = arrCallStackInfo.begin(); i != arrCallStackInfo.end(); ++i)
	{
		CallStackInfo callstackinfo = (*i);

		//cout << callstackinfo.MethodName << "() : [" << callstackinfo.ModuleName << "] (File: " << callstackinfo.FileName << " @Line " << callstackinfo.LineNumber << ")" << endl;

		myLog.TraceInfo("%s():[%s](File: %s @Line %s)", callstackinfo.MethodName, callstackinfo.ModuleName, callstackinfo.FileName, callstackinfo.LineNumber);
	}

	// 这里弹出一个错误对话框并退出程序  
	//  
	//FatalAppExit(-1, _T("*** Unhandled Exception! ***"));

	return EXCEPTION_EXECUTE_HANDLER;
}

// 安全拷贝字符串函数  
// 
void CMiniDumper::SafeStrCpy(char* szDest, size_t nMaxDestSize, const char* szSrc)
{
	if (nMaxDestSize <= 0) return;
	if (strlen(szSrc) < nMaxDestSize)
	{
		strcpy_s(szDest, nMaxDestSize, szSrc);
	}
	else
	{
		strncpy_s(szDest, nMaxDestSize, szSrc, nMaxDestSize);
		szDest[nMaxDestSize - 1] = '\0';
	}
}

// 得到程序崩溃信息  
//  
CrashInfo CMiniDumper::GetCrashInfo(const EXCEPTION_RECORD *pRecord)
{
	CrashInfo crashinfo;
	SafeStrCpy(crashinfo.Address, MAX_ADDRESS_LENGTH, "N/A");
	SafeStrCpy(crashinfo.ErrorCode, MAX_ADDRESS_LENGTH, "N/A");
	SafeStrCpy(crashinfo.Flags, MAX_ADDRESS_LENGTH, "N/A");

	sprintf_s(crashinfo.Address, "%08X", pRecord->ExceptionAddress);
	sprintf_s(crashinfo.ErrorCode, "%08X", pRecord->ExceptionCode);
	sprintf_s(crashinfo.Flags, "%08X", pRecord->ExceptionFlags);

	return crashinfo;
}

// 得到CallStack信息  
//  
std::vector<CallStackInfo> CMiniDumper::GetCallStack(const CONTEXT *pContext)
{
	HANDLE hProcess = GetCurrentProcess();

	SymInitialize(hProcess, NULL, TRUE);

	std::vector<CallStackInfo> arrCallStackInfo;

	CONTEXT c = *pContext;

	STACKFRAME64 sf;
	memset(&sf, 0, sizeof(STACKFRAME64));
	DWORD dwImageType = IMAGE_FILE_MACHINE_I386;

	// 不同的CPU类型，具体信息可查询MSDN  
	//  
#ifdef _M_IX86  
	sf.AddrPC.Offset = c.Eip;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Offset = c.Esp;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Offset = c.Ebp;
	sf.AddrFrame.Mode = AddrModeFlat;
#elif _M_X64  
	dwImageType = IMAGE_FILE_MACHINE_AMD64;
	sf.AddrPC.Offset = c.Rip;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrFrame.Offset = c.Rsp;
	sf.AddrFrame.Mode = AddrModeFlat;
	sf.AddrStack.Offset = c.Rsp;
	sf.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64  
	dwImageType = IMAGE_FILE_MACHINE_IA64;
	sf.AddrPC.Offset = c.StIIP;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrFrame.Offset = c.IntSp;
	sf.AddrFrame.Mode = AddrModeFlat;
	sf.AddrBStore.Offset = c.RsBSP;
	sf.AddrBStore.Mode = AddrModeFlat;
	sf.AddrStack.Offset = c.IntSp;
	sf.AddrStack.Mode = AddrModeFlat;
#else  
#error "Platform not supported!"  
#endif  

	HANDLE hThread = GetCurrentThread();

	while (true)
	{
		// 该函数是实现这个功能的最重要的一个函数  
		// 函数的用法以及参数和返回值的具体解释可以查询MSDN  
		//  
		if (!StackWalk64(dwImageType, hProcess, hThread, &sf, &c, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
			break;
		}

		if (sf.AddrFrame.Offset == 0)
		{
			break;
		}

		CallStackInfo callstackinfo;
		SafeStrCpy(callstackinfo.MethodName, MAX_NAME_LENGTH, "N/A");
		SafeStrCpy(callstackinfo.FileName, MAX_NAME_LENGTH, "N/A");
		SafeStrCpy(callstackinfo.ModuleName, MAX_NAME_LENGTH, "N/A");
		SafeStrCpy(callstackinfo.LineNumber, MAX_NAME_LENGTH, "N/A");

		BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL64)+MAX_NAME_LENGTH];
		IMAGEHLP_SYMBOL64 *pSymbol = (IMAGEHLP_SYMBOL64*)symbolBuffer;
		memset(pSymbol, 0, sizeof(IMAGEHLP_SYMBOL64)+MAX_NAME_LENGTH);

		pSymbol->SizeOfStruct = sizeof(symbolBuffer);
		pSymbol->MaxNameLength = MAX_NAME_LENGTH;

		DWORD symDisplacement = 0;

		// 得到函数名  
		//  
		if (SymGetSymFromAddr64(hProcess, sf.AddrPC.Offset, NULL, pSymbol))
		{
			SafeStrCpy(callstackinfo.MethodName, MAX_NAME_LENGTH, pSymbol->Name);
		}

		IMAGEHLP_LINE64 lineInfo;
		memset(&lineInfo, 0, sizeof(IMAGEHLP_LINE64));

		lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		DWORD dwLineDisplacement;

		// 得到文件名和所在的代码行  
		//  
		if (SymGetLineFromAddr64(hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo))
		{
			SafeStrCpy(callstackinfo.FileName, MAX_NAME_LENGTH, lineInfo.FileName);
			sprintf_s(callstackinfo.LineNumber, "%d", lineInfo.LineNumber);
		}

		IMAGEHLP_MODULE64 moduleInfo;
		memset(&moduleInfo, 0, sizeof(IMAGEHLP_MODULE64));

		moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

		// 得到模块名  
		//  
		if (SymGetModuleInfo64(hProcess, sf.AddrPC.Offset, &moduleInfo))
		{
			SafeStrCpy(callstackinfo.ModuleName, MAX_NAME_LENGTH, moduleInfo.ModuleName);
		}

		arrCallStackInfo.push_back(callstackinfo);
	}

	SymCleanup(hProcess);

	return arrCallStackInfo;
}

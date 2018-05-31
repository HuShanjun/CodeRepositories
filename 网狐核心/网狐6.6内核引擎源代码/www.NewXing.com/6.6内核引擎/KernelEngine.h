//Download by http://www.NewXing.com
#ifndef SERVER_KERNEL_HEAD_FILE
#define SERVER_KERNEL_HEAD_FILE

//////////////////////////////////////////////////////////////////////////

//系统头文件
//包含文件

#include <Afxmt.h>
#include <Comutil.h>
#include <ICrsint.h>
#include <Process.h>
#include <WinSock2.h>


//平台文件
#include "..\..\模板库\Template.h"
#include "..\..\公共文件\Constant.h"
#include "..\..\公共文件\GlobalDef.h"
#include "..\..\共享组件\公共服务\ComService.h"
//////////////////////////////////////////////////////////////////////////

//ADO 导入库
#import "MSADO15.DLL" rename_namespace("ADOCG") rename("EOF","EndOfFile")
using namespace ADOCG;

//COM 错误类型
typedef _com_error					CComError;							//COM 错误

//////////////////////////////////////////////////////////////////////////
//公共宏定义

//模块定义
#ifdef _DEBUG
	#define SERVER_KERNEL_DLL_NAME	TEXT("KernelEngineD.dll")			//组件 DLL 名字
#else
	#define SERVER_KERNEL_DLL_NAME	TEXT("KernelEngine.dll")			//组件 DLL 名字
#endif

//常量宏定义
#define EVENT_LEVEL_COUNT			4									//事件等级
#define MAX_QUEUE_PACKET			10240								//最大队列
#define INDEX_ALL_SOCKET			0xFFFF								//所有连接
#define TIMER_REPEAT_TIMER			DWORD(-1)							//重复次数

//////////////////////////////////////////////////////////////////////////
//枚举定义

//输出等级
enum enTraceLevel
{
	Level_Normal					=0,									//普通消息
	Level_Warning					=1,									//警告消息
	Level_Exception					=2,									//异常消息
	Level_Debug						=3,									//调试消息
};

//数据库错误代码
enum enADOErrorType
{
	ErrorType_Nothing				=0,									//没有错误
	ErrorType_Connect				=1,									//连接错误
	ErrorType_Other					=2,									//其他错误
};

//////////////////////////////////////////////////////////////////////////
//事件定义

//事件标识
#define EVENT_CONTROL				0x0001								//控制消息
#define EVENT_TIMER					0x0002								//定时器引擎
#define EVENT_DATABASE				0x0003								//数据库请求
#define EVENT_SOCKET_ACCEPT			0x0004								//网络应答
#define EVENT_SOCKET_READ			0x0005								//网络读取
#define EVENT_SOCKET_CLOSE			0x0006								//网络关闭

//定时器事件
struct NTY_TimerEvent
{
	WORD							wTimerID;							//定时器 ID
	WPARAM							wBindParam;							//绑定参数
};

//数据库请求事件
struct NTY_DataBaseEvent
{
	WORD							wIndex;								//对象索引
	WORD							wRoundID;							//对象标识
	WORD							wRequestID;							//请求标识
};

//网络应答事件
struct NTY_SocketAcceptEvent
{
	WORD							wIndex;								//连接索引
	WORD							wRoundID;							//连接标识
	DWORD							dwClientIP;							//连接地址
};

//网络读取事件
struct NTY_SocketReadEvent
{
	WORD							wIndex;								//连接索引
	WORD							wRoundID;							//连接标识
	WORD							wDataSize;							//数据大小
	CMD_Command						Command;							//命令信息
};

//网络关闭事件
struct NTY_SocketCloseEvent
{
	WORD							wIndex;								//连接索引
	WORD							wRoundID;							//连接标识
	DWORD							dwClientIP;							//连接地址
	DWORD							dwConnectSecond;					//连接时间
};

//////////////////////////////////////////////////////////////////////////

#define VER_IADOError INTERFACE_VERSION(1,1)
static const GUID IID_IADOError={0x66463b5a,0x390c,0x42f9,0x85,0x19,0x13,0x31,0x39,0x36,0xfe,0x8f};

//数据库错误接口
interface IADOError : public IUnknownEx
{
	//错误描述
	virtual LPCTSTR __cdecl GetErrorDescribe()=NULL;
	//错误类型
	virtual enADOErrorType __cdecl GetErrorType()=NULL;
};

//////////////////////////////////////////////////////////////////////////

#define VER_IDataBase INTERFACE_VERSION(1,1)
static const GUID IID_IDataBase={0x9e962173,0x2a9f,0x4ebd,0x8e,0x98,0x40,0xe9,0x96,0x57,0x24,0xfb};

//数据库连接接口
interface IDataBase : public IUnknownEx
{
	//打开连接
	virtual bool __cdecl OpenConnection()=NULL;
	//关闭记录
	virtual bool __cdecl CloseRecordset()=NULL;
	//关闭连接
	virtual bool __cdecl CloseConnection()=NULL;
	//重新连接
	virtual bool __cdecl TryConnectAgain(bool bFocusConnect, CComError * pComError)=NULL;
	//设置信息
	virtual bool __cdecl SetConnectionInfo(LPCTSTR szIP, WORD wPort, LPCTSTR szData, LPCTSTR szName, LPCTSTR szPass)=NULL;
	//是否连接错误
	virtual bool __cdecl IsConnectError()=NULL;
	//是否打开
	virtual bool __cdecl IsRecordsetOpened()=NULL;
	//往下移动
	virtual void __cdecl MoveToNext()=NULL;
	//移到开头
	virtual void __cdecl MoveToFirst()=NULL;
	//是否结束
	virtual bool __cdecl IsEndRecordset()=NULL;
	//获取数目
	virtual long __cdecl GetRecordCount()=NULL;
	//获取大小
	virtual long __cdecl GetActualSize(LPCTSTR pszParamName)=NULL;
	//绑定对象
	virtual bool __cdecl BindToRecordset(CADORecordBinding * pBind)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, BYTE & bValue)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, WORD & wValue)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, INT & nValue)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, LONG & lValue)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, DWORD & ulValue)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, UINT & ulValue)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, DOUBLE & dbValue)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, __int64 & llValue)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, TCHAR szBuffer[], UINT uSize)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, COleDateTime & Time)=NULL;
	//获取参数
	virtual bool __cdecl GetFieldValue(LPCTSTR lpFieldName, bool & bValue)=NULL;
	//设置存储过程
	virtual void __cdecl SetSPName(LPCTSTR pszSpName)=NULL;
	//插入参数
	virtual void __cdecl AddParamter(LPCTSTR pszName, ADOCG::ParameterDirectionEnum Direction, ADOCG::DataTypeEnum Type, long lSize, _variant_t & vtValue)=NULL;
	//删除参数
	virtual void __cdecl ClearAllParameters()=NULL;
	//获得参数
	virtual void __cdecl GetParameterValue(LPCTSTR pszParamName, _variant_t & vtValue)=NULL;
	//获取返回数值
	virtual long __cdecl GetReturnValue()=NULL;
	//执行语句
	virtual bool __cdecl Execute(LPCTSTR pszCommand)=NULL;
	//执行命令
	virtual bool __cdecl ExecuteCommand(bool bRecordset)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IDataBaseSink INTERFACE_VERSION(1,1)
	static const GUID IID_IDataBaseSink={0x5852e135,0x18cf,0x4893,0xa2,0x19,0x49,0x51,0x99,0xa2,0xf4,0xa5};
#else
	#define VER_IDataBaseSink INTERFACE_VERSION(1,1)
	static const GUID IID_IDataBaseSink={0x4ba5fef5,0x0fe4,0x4b3f,0x8f,0x28,0x79,0x6a,0x16,0x24,0x5b,0xd6};
#endif

//数据库钩子接口
interface IDataBaseSink : public IUnknownEx
{
	//数据库模块启动
	virtual bool __cdecl StartService(IUnknownEx * pIUnknownEx)=NULL;
	//数据库模块关闭
	virtual bool __cdecl StopService(IUnknownEx * pIUnknownEx)=NULL;
	//数据操作处理
	virtual bool __cdecl OnDataBaseRequest(const NTY_DataBaseEvent & DataBaseEvent, void * pDataBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IDataBaseEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IDataBaseEngine={0x394928f5,0x0aa1,0x414a,0xbe,0xe2,0x0c,0x42,0xb1,0xa8,0x86,0x63};
#else
	#define VER_IDataBaseEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IDataBaseEngine={0xe6a05538,0xbf46,0x4661,0xa3,0xe1,0xa0,0x43,0x7a,0x18,0x22,0x7a};
#endif

//数据库引擎接口
interface IDataBaseEngine : public IUnknownEx
{
	//启动服务
	virtual bool __cdecl StartService()=NULL;
	//停止服务
	virtual bool __cdecl StopService()=NULL;
	//注册钩子
	virtual bool __cdecl SetDataBaseSink(IUnknownEx * pIUnknownEx)=NULL;
	//获取接口
	virtual void * __cdecl GetQueueService(const IID & Guid, DWORD dwQueryVer)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IQueueServiceSink INTERFACE_VERSION(1,1)
	static const GUID IID_IQueueServiceSink={0xbd0d29c2,0x5e92,0x4d8d,0x99,0x2c,0x74,0x3f,0xd6,0x7e,0x74,0x1f};
#else
	#define VER_IQueueServiceSink INTERFACE_VERSION(1,1)
	static const GUID IID_IQueueServiceSink={0x4d2c22dc,0xbcae,0x47e7,0x94,0x41,0x56,0x82,0x53,0x6d,0xa2,0xf0};
#endif

//数据队列类钩子接口
interface IQueueServiceSink : public IUnknownEx
{
	//通知回调函数
	virtual void __cdecl OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#define VER_IQueueService INTERFACE_VERSION(1,1)
static const GUID IID_IQueueService={0xcc5310b5,0x3709,0x40aa,0x85,0x24,0xd6,0xc5,0x87,0xb0,0x32,0x22};

//数据队列接口
interface IQueueService : public IUnknownEx
{
	//加入数据
	virtual bool __cdecl AddToQueue(WORD wIdentifier, void * const pBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IQueueServiceEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IQueueServiceEngine={0xba9e9d45,0x81c8,0x4a18,0x88,0x86,0x22,0xa7,0x19,0x1c,0x8b,0x54};
#else
	#define VER_IQueueServiceEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IQueueServiceEngine={0x3d205995,0x427b,0x4b30,0xaf,0xfe,0x7e,0x90,0xe4,0x77,0x80,0xc2};
#endif

//队列类引擎接口
interface IQueueServiceEngine : public IUnknownEx
{
	//开始服务
	virtual bool __cdecl StartService()=NULL;
	//停止服务
	virtual bool __cdecl StopService()=NULL;
	//设置接口
	virtual bool __cdecl SetQueueServiceSink(IUnknownEx * pIUnknownEx)=NULL;
	//负荷信息
	virtual bool __cdecl GetBurthenInfo(tagBurthenInfo & BurthenInfo)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IEventService INTERFACE_VERSION(1,1)
	static const GUID IID_IEventService={0x0ba26480,0xb171,0x4726,0xa7,0xff,0x9d,0xe3,0x06,0x46,0x01,0xec};
#else
	#define VER_IEventService INTERFACE_VERSION(1,1)
	static const GUID IID_IEventService={0xb6f84de2,0x4c44,0x4bb0,0xb8,0x2d,0xb2,0x0e,0x80,0x01,0x05,0xd8};
#endif

//服务引擎接口
interface IEventService : public IUnknownEx
{
	//设置句柄
	virtual bool __cdecl SetRichEditHwnd(HWND hRichEdit)=NULL;
	//设置级别
	virtual void __cdecl ConfigEventService(enTraceLevel TraceLevel, bool bShow)=NULL;
	//事件通知
	virtual void __cdecl ShowEventNotify(LPCTSTR pszString, enTraceLevel TraceLevel)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_ITimerEngine INTERFACE_VERSION(1,1)
	static const GUID IID_ITimerEngine={0x89255ac1,0x598f,0x4d67,0xb0,0x22,0xfc,0xaf,0x0a,0xeb,0x37,0x5d};
#else
	#define VER_ITimerEngine INTERFACE_VERSION(1,1)
	static const GUID IID_ITimerEngine={0xeeca6792,0x9508,0x4bf2,0xaf,0xda,0x4e,0x1b,0x0d,0x94,0x2e,0xc4};
#endif

//定时器引擎接口
interface ITimerEngine : public IUnknownEx
{
	//设置定时器
	virtual bool __cdecl SetTimer(WORD wTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM wParam)=NULL; 
	//删除定时器
	virtual bool __cdecl KillTimer(WORD wTimerID)=NULL;
	//删除定时器
	virtual bool __cdecl KillAllTimer()=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_ITimerEngineManager INTERFACE_VERSION(1,1)
	static const GUID IID_ITimerEngineManager={0xac341f6d,0x1106,0x4e4f,0xbc,0x8f,0x87,0xd0,0x63,0x5f,0xe1,0xbc};
#else
	#define VER_ITimerEngineManager INTERFACE_VERSION(1,1)
	static const GUID IID_ITimerEngineManager={0x26f7f428,0x7196,0x4517,0xb8,0x79,0x1e,0x43,0x01,0x74,0xe2,0x07};
#endif

//定时器引擎接口
interface ITimerEngineManager : public IUnknownEx
{
	//开始服务
	virtual bool __cdecl StartService()=NULL;
	//停止服务
	virtual bool __cdecl StopService()=NULL;
	//设置接口
	virtual bool __cdecl SetTimerEngineSink(IUnknownEx * pIUnknownEx)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_ITCPSocketEngine INTERFACE_VERSION(1,1)
	static const GUID IID_ITCPSocketEngine={0x1b9f8424,0x639f,0x47e1,0x91,0x4b,0x2f,0xa2,0x57,0x24,0xa8,0x22};
#else
	#define VER_ITCPSocketEngine INTERFACE_VERSION(1,1)
	static const GUID IID_ITCPSocketEngine={0x0c7b943d,0x2f69,0x454e,0xb6,0xf4,0x81,0x7a,0xf6,0xa1,0x83,0x15};
#endif

//TCP SOCKET 引擎接口
interface ITCPSocketEngine : public IUnknownEx
{
	//检测连接
	virtual bool __cdecl DetectSocket()=NULL;
	//发送函数
	virtual bool __cdecl SendData(WORD wIndex, WORD wRoundID, WORD wMainCmdID, WORD wSubCmdID)=NULL;
	//发送函数
	virtual bool __cdecl SendData(WORD wIndex, WORD wRoundID, WORD wMainCmdID, WORD wSubCmdID, void * pData, WORD wDataSize)=NULL;
	//批量发送
	virtual bool __cdecl SendDataBatch(WORD wMainCmdID, WORD wSubCmdID, void * pData, WORD wDataSize)=NULL;
	//关闭连接
	virtual bool __cdecl CloseSocket(WORD wIndex, WORD wRoundID)=NULL;
	//关闭连接
	virtual bool __cdecl ShutDownSocket(WORD wIndex, WORD wRoundID)=NULL;
	//允许群发
	virtual bool __cdecl AllowBatchSend(WORD wIndex, WORD wRoundID, bool bAllow)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_ITCPSocketEngineManager INTERFACE_VERSION(1,1)
	static const GUID IID_ITCPSocketEngineManager={0xc255c9e4,0x5539,0x4fd1,0xb5,0xaf,0x71,0x4c,0x66,0x94,0x21,0xc3};
#else
	#define VER_ITCPSocketEngineManager INTERFACE_VERSION(1,1)
	static const GUID IID_ITCPSocketEngineManager={0x43a86f7d,0x4f25,0x47aa,0x8b,0x5a,0x05,0x5d,0x3a,0xc8,0x39,0xf3};
#endif

//TCP SOCKET 引擎管理接口
interface ITCPSocketEngineManager : public IUnknownEx
{
	//设置接口
	virtual bool __cdecl SetSocketEngineSink(IUnknownEx * pIUnknownEx)=NULL;
	//设置端口
	virtual bool __cdecl SetServicePort(WORD wListenPort)=NULL;
	//设置数目
	virtual bool __cdecl SetMaxSocketItem(WORD wMaxSocketItem)=NULL;
	//启动服务
	virtual bool __cdecl StartService()=NULL;
	//停止服务
	virtual bool __cdecl StopService()=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IAttemperEngineSink INTERFACE_VERSION(1,1)
	static const GUID IID_IAttemperEngineSink={0x2a3aa509,0x1140,0x4dc3,0x88,0xbc,0xcb,0x6b,0xcd,0x7d,0x5e,0xcb};
#else
	#define VER_IAttemperEngineSink INTERFACE_VERSION(1,1)
	static const GUID IID_IAttemperEngineSink={0x12513790,0x36d0,0x48d2,0xa6,0xfa,0xc0,0x73,0x8c,0xb3,0xec,0xd5};
#endif

//调度模块钩子接口
interface IAttemperEngineSink : public IUnknownEx
{
	//管理接口
public:
	//调度模块启动
	virtual bool __cdecl StartService(IUnknownEx * pIUnknownEx)=NULL;
	//调度模块关闭
	virtual bool __cdecl StopService(IUnknownEx * pIUnknownEx)=NULL;
	//事件处理接口
	virtual bool __cdecl OnAttemperEvent(WORD wIdentifier, void * pBuffer, WORD wDataSize)=NULL;

	//事件接口
public:
	//定时器事件
	virtual bool __cdecl OnEventTimer(DWORD wTimerID, WPARAM wBindParam)=NULL;
	//数据库事件
	virtual bool __cdecl OnEventDataBase(void * pDataBuffer, WORD wDataSize, NTY_DataBaseEvent * pDataBaseEvent)=NULL;
	//网络应答事件
	virtual bool __cdecl OnEventSocketAccept(NTY_SocketAcceptEvent * pSocketAcceptEvent)=NULL;
	//网络读取事件
	virtual bool __cdecl OnEventSocketRead(CMD_Command Command, void * pDataBuffer, WORD wDataSize, NTY_SocketReadEvent * pSocketReadEvent)=NULL;
	//网络关闭事件
	virtual bool __cdecl OnEventSocketClose(NTY_SocketCloseEvent * pSocketCloseEvent)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IAttemperEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IAttemperEngine={0x3b328b10,0xe670,0x4eae,0xad,0xc5,0xf3,0xb0,0x2f,0xe2,0xa2,0x34};
#else
	#define VER_IAttemperEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IAttemperEngine={0x6351e550,0x5fc8,0x4620,0xbb,0xed,0xe3,0x5b,0x9b,0xe4,0x37,0x83};
#endif

//调度引擎接口
interface IAttemperEngine : public IUnknownEx
{
	//管理接口
public:
	//启动服务
	virtual bool __cdecl StartService()=NULL;
	//停止服务
	virtual bool __cdecl StopService()=NULL;
	//设置网络
	virtual bool __cdecl SetSocketEngine(IUnknownEx * pIUnknownEx)=NULL;
	//设置钩子
	virtual bool __cdecl SetAttemperEngineSink(IUnknownEx * pIUnknownEx)=NULL;
	//获取接口
	virtual void * __cdecl GetQueueService(const IID & Guid, DWORD dwQueryVer)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IServiceEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IServiceEngine={0xea0f0398,0xde5d,0x4c57,0xbf,0xce,0x48,0xc7,0xde,0x9a,0xeb,0x54};
#else
	#define VER_IServiceEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IServiceEngine={0x238751d4,0xa722,0x46e4,0x97,0x18,0xfc,0x19,0x0a,0x2c,0x59,0x76};
#endif

//服务引擎接口
interface IServiceEngine : public IUnknownEx
{
	//服务接口
public:
	//启动服务
	virtual bool __cdecl StartService()=NULL;
	//停止服务
	virtual bool __cdecl StopService()=NULL;

	//配置接口
public:
	//设置事件
	virtual bool __cdecl SetEventService(IUnknownEx * pIUnknownEx)=NULL;
	//设置钩子
	virtual bool __cdecl SetDataBaseSink(IUnknownEx * pIUnknownEx)=NULL;
	//设置钩子
	virtual bool __cdecl SetAttemperEngineSink(IUnknownEx * pIUnknownEx)=NULL;
	//注册钩子
	virtual bool __cdecl RegisterAsynchronismEngineSink(IUnknownEx * pIUnknownEx)=NULL;
	//设置网络引擎接口
	virtual bool __cdecl InitServiceEngine(WORD wListenPort, WORD wMaxSocketItem)=NULL;

	//服务查询
public:
	//获取定时器接口
	virtual void * __cdecl GetTimerEngine(const IID & Guid, DWORD dwQueryVer)=NULL;
	//获取数据库引擎接口
	virtual void * __cdecl GetDataBaseEngine(const IID & Guid, DWORD dwQueryVer)=NULL;
	//获取调度引擎接口
	virtual void * __cdecl GetAttemperEngine(const IID & Guid, DWORD dwQueryVer)=NULL;
	//获取网络引擎接口
	virtual void * __cdecl GetTCPSocketEngine(const IID & Guid, DWORD dwQueryVer)=NULL;
	//获取异步引擎接口
	virtual void * __cdecl GetAsynchronismEngine(const IID & Guid, DWORD dwQueryVer)=NULL;
	//获取接口
	virtual void * __cdecl GetDataBaseQueueService(const IID & Guid, DWORD dwQueryVer)=NULL;
	//获取接口
	virtual void * __cdecl GetAttemperQueueService(const IID & Guid, DWORD dwQueryVer)=NULL;

	//功能接口
public:
	//服务状态
	virtual bool __cdecl IsService()=NULL;
	//外部控制
	virtual bool __cdecl ControlService(void * pBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IAsynchronismEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IAsynchronismEngine={0x31ef54c0,0xe000,0x4935,0xb5,0xa3,0x56,0x4f,0x29,0xd0,0x23,0x0a};
#else
	#define VER_IAsynchronismEngine INTERFACE_VERSION(1,1)
	static const GUID IID_IAsynchronismEngine={0xe0ee6282,0x3140,0x4aba,0xa3,0x6f,0x79,0x88,0x32,0xfd,0xe8,0xcf};
#endif

//异步引擎接口
interface IAsynchronismEngine : public IUnknownEx
{
	//管理接口
public:
	//启动服务
	virtual bool __cdecl StartService()=NULL;
	//停止服务
	virtual bool __cdecl StopService()=NULL;
	//插入请求
	virtual bool __cdecl InsertRequest(WORD wRequestID, void * const pBuffer, WORD wDataSize, IUnknownEx * pIUnknownEx)=NULL;

	//功能接口
public:
	//注册钩子
	virtual bool __cdecl RegisterAsynchronismEngineSink(IUnknownEx * pIUnknownEx)=NULL;
	//取消注册
	virtual bool __cdecl UnRegisterAsynchronismEngineSink(IUnknownEx * pIUnknownEx)=NULL;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define VER_IAsynchronismEngineSink INTERFACE_VERSION(1,1)
	static const GUID IID_IAsynchronismEngineSink={0x623dc53d,0xfe20,0x40fd,0xb7,0x7b,0xd8,0xab,0x3d,0xf6,0x7e,0x27};
#else
	#define VER_IAsynchronismEngineSink INTERFACE_VERSION(1,1)
	static const GUID IID_IAsynchronismEngineSink={0x7404a759,0xe429,0x4253,0x86,0xc5,0x12,0x25,0x60,0x90,0x4f,0xf7};
#endif

//异步引擎钩子接口
interface IAsynchronismEngineSink : public IUnknownEx
{
	//启动事件
	virtual bool __cdecl OnAsynchronismEngineStart()=NULL;
	//停止事件
	virtual bool __cdecl OnAsynchronismEngineStop()=NULL;
	//异步请求
	virtual bool __cdecl OnAsynchronismRequest(WORD wRequestID, void * pBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//导出组件文件
#include "QueueServiceEvent.h"
#include "KernelEngineHelper.h"

//////////////////////////////////////////////////////////////////////////

#endif
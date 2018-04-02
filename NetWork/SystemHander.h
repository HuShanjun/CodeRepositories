#pragma once
#include <map>
#include <string>
#include "ScopeLock.h"
#include "HttpHander.h"
using namespace std;

struct VtnCodeData
{
	wstring mobilePhone;	//手机号
	wstring vtnCode;		//手机验证码
	DWORD expireTime;		//到期时间，秒为单位
};

class CSystemHander
{
public:
	static CSystemHander& GetInstance()											//唯一实例
	{
		static CSystemHander s_instance;
		return s_instance;
	}

public:
	CSystemHander(void);
	~CSystemHander(void);

public:
	void InitData(CString szFilePath);											//使用前，先调用初始化

	//手机验证码相关
	bool SendVerificationCode(wstring mobilePhone);								//发送手机验证码
	bool VerifyVerificationCode(wstring mobilePhone, wstring vtnCode);			//判断验证码是否有效
	void RemoveVerificationCode(wstring mobilePhone);							//清除验证码


	//提现相关
	wstring CreateOrderId();													//生成提款订单ID
	std::string NoticePhpCreateOrder(wstring orderId);							//通知PHP已生成订单

	//获取代理商列表
	bool GetBusinessesList(wstring &content);									//获取代理商列表

private:
	int GetRandNumber(int maxNumber = 10000, int minNumber = 1000);
	bool SendSMS(wstring mobilePhone, wstring vtnCode);							// 接能阿里云短信
	string CreateSMSUrl(wstring mobilePhone, wstring vtnCode);					//创建发送短信的url	
	string specialUrlEncode(string value);
	string StringSign(string accessSecret, string stringToSign) ;
	std::string  GetErrorMsg(std::string strSrc);								//如果请求发生错误，提取错误信息

private:
	map<wstring, VtnCodeData> m_VtnCodeMap;										//所有验证码、不进数据库

	static MyMutex s_instance_mutex;

private:
	int m_VerificationCodeValidity;												//单位为秒
	string m_AliyunSmsSignName;
	string m_AliyunSmsTemplateCode;
	string m_AliyunSmsTemplateParam;
	string m_AliyunSmsAccessKeyId;
	string m_AliyunSmsAccessSecret;

	string m_OrderNoticeUrl;													//上报订单URL
	string m_BusinessesUrl;														//获取代理商列表URL

};

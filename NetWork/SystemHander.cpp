#include "StdAfx.h"
#include "SystemHander.h"
#include "ToolFun.h"
#include <time.h>
#include "HMAC_SHA1.h"

MyMutex CSystemHander::s_instance_mutex;

CSystemHander::CSystemHander(void)
{
	srand((unsigned int)time(0)); 

	CHttpHander::InitCURL(); //CURL 初始化	
	m_VerificationCodeValidity = 10*60; //默认10分钟
}

CSystemHander::~CSystemHander(void)
{
	CHttpHander::CleanupCURL(); //目前只有这个用了CURL
}

void CSystemHander::InitData(CString szFilePath)
{
	wstring appName = L"AliyunSMS";
	m_VerificationCodeValidity = GetPrivateProfileInt(appName.c_str(), TEXT("VerificationCodeTime"), 10*60, szFilePath); //默认10分钟

	wchar_t szSignName[64]={0};
	GetPrivateProfileString(appName.c_str(), TEXT("SignName"), L"竹石", szSignName, sizeof(szSignName), szFilePath);
	m_AliyunSmsSignName = ToolFun::ConvertUnicodeToMultiBytes(szSignName);

	wchar_t szTemplateCode[64]={0};
	GetPrivateProfileString(appName.c_str(), TEXT("TemplateCode"), L"SMS_99265018", szTemplateCode, sizeof(szTemplateCode), szFilePath);
	m_AliyunSmsTemplateCode = ToolFun::ConvertUnicodeToMultiBytes(szTemplateCode);

	wchar_t szTemplateParam[64]={0};
	GetPrivateProfileString(appName.c_str(), TEXT("TemplateParam"), L"{\"code\":\"%s\"}", szTemplateParam, sizeof(szTemplateParam), szFilePath);
	m_AliyunSmsTemplateParam = ToolFun::ConvertUnicodeToMultiBytes(szTemplateParam);

	wchar_t szAccessKeyId[64]={0};
	GetPrivateProfileString(appName.c_str(), TEXT("AccessKeyId"), L"LTAIj1OxBcod5geL", szAccessKeyId, sizeof(szAccessKeyId), szFilePath);
	m_AliyunSmsAccessKeyId = ToolFun::ConvertUnicodeToMultiBytes(szAccessKeyId);

	wchar_t szAccessSecret[64]={0};
	GetPrivateProfileString(appName.c_str(), TEXT("AccessSecret"), L"ThJAZ3apd91T3U2oazsxKvpc7ijiXw", szAccessSecret, sizeof(szAccessSecret), szFilePath);
	m_AliyunSmsAccessSecret = ToolFun::ConvertUnicodeToMultiBytes(szAccessSecret);	

	wstring phpUrl = L"PHPUrl";
	wchar_t szOrderNoticeUrl[128]={0};

	GetPrivateProfileString(phpUrl.c_str(), TEXT("OrderNotice"), L"http://pay.gzzhushi.com/withdrawal/create?order_id=%s", szOrderNoticeUrl, sizeof(szOrderNoticeUrl), szFilePath);
	m_OrderNoticeUrl = ToolFun::ConvertUnicodeToMultiBytes(szOrderNoticeUrl);

	wchar_t szBusinessesUrl[128]={0};
	GetPrivateProfileString(phpUrl.c_str(), TEXT("Businesses"), L"http://game.gzzhushi.com/api/businesses/businesses", szBusinessesUrl, sizeof(szBusinessesUrl), szFilePath);
	m_BusinessesUrl = ToolFun::ConvertUnicodeToMultiBytes(szBusinessesUrl);	
}

//发送验证码
bool CSystemHander::SendVerificationCode(wstring mobilePhone)
{	
	MyScopeLock locker(s_instance_mutex);
	if (mobilePhone.empty()) return false;	

	int randCode = GetRandNumber();
	wchar_t vtnCode[16] = {0};
	_sntprintf(vtnCode,sizeof(vtnCode),TEXT("%d"),randCode);

	VtnCodeData vtnCodeData;
	vtnCodeData.mobilePhone = mobilePhone;
	vtnCodeData.vtnCode = vtnCode;
	vtnCodeData.expireTime = (DWORD)time(NULL);

	bool bRet = SendSMS(mobilePhone, vtnCode);
	if (bRet)
	{
		m_VtnCodeMap[mobilePhone] = vtnCodeData;
	}

	return bRet;
}

//校验验证码
bool CSystemHander::VerifyVerificationCode(wstring mobilePhone, wstring vtnCode)
{
	MyScopeLock locker(s_instance_mutex);

	if (m_VtnCodeMap.find(mobilePhone) != m_VtnCodeMap.end())
	{
		DWORD dwCurrentTime= (DWORD)time(NULL);		
		VtnCodeData vtnCodeData = m_VtnCodeMap[mobilePhone];

		if ((vtnCode == vtnCodeData.vtnCode)
			&& (dwCurrentTime - vtnCodeData.expireTime <= m_VerificationCodeValidity))
		{
			return true;
		}
	}

	return false;
}

//清除验证码
void CSystemHander::RemoveVerificationCode(wstring mobilePhone)
{
	MyScopeLock locker(s_instance_mutex);

	if (m_VtnCodeMap.find(mobilePhone) != m_VtnCodeMap.end())
	{
		m_VtnCodeMap.erase(mobilePhone);
	}
}

//生成提款订单ID
wstring CSystemHander::CreateOrderId()
{	
	return ToolFun::Format(L"T%ld%d", time(0), GetRandNumber());
}

//通知PHP已生成订单
std::string CSystemHander::NoticePhpCreateOrder(wstring orderId) 
{
	std::string url_str = ToolFun::Format(m_OrderNoticeUrl.c_str(), ToolFun::ConvertUnicodeToMultiBytes(orderId).c_str());
	CHttpHander http;
	std::string strMsg = http.get_value(url_str);
	
	return strMsg;
}

//获取代理商列表
bool CSystemHander::GetBusinessesList(wstring &content)
{
	CHttpHander http;
	string tempData = http.get_value(m_BusinessesUrl, 100);
	if (tempData.empty())
	{
		return false;
	}
	content = ToolFun::ConvertMultiBytesToUnicode(tempData, CP_UTF8);
	return true;
}


//下面是内部工具函数
//---------------------------------------------------------------------------------------------------------------------

int CSystemHander::GetRandNumber(int maxNumber, int minNumber) {

	return (int)(((double)rand()) / (RAND_MAX+1)*(maxNumber-minNumber) + minNumber);
}

bool CSystemHander::SendSMS(wstring mobilePhone, wstring vtnCode)
{	
	std::string url_str = CreateSMSUrl(mobilePhone, vtnCode);
	CHttpHander http;
	http.get_value(url_str);
	return true;
}

string CSystemHander::specialUrlEncode(string _value)
{	
	string tempValue = ToolFun::ConvertUnicodeToMultiBytes(ToolFun::UrlEncode(ToolFun::ConvertMultiBytesToUnicode(_value)), CP_UTF8);

	tempValue = ToolFun::ReplaceAll(tempValue, "+", "%20");
	tempValue = ToolFun::ReplaceAll(tempValue, "*", "%2A");
	tempValue = ToolFun::ReplaceAll(tempValue, "%7E", "~");

	return tempValue;
}

string CSystemHander::StringSign(string accessSecret, string stringToSign) 
{
	accessSecret = ToolFun::ConvertUnicodeToMultiBytes(ToolFun::ConvertMultiBytesToUnicode(accessSecret, CP_UTF8), CP_UTF8);
	stringToSign = ToolFun::ConvertUnicodeToMultiBytes(ToolFun::ConvertMultiBytesToUnicode(stringToSign, CP_UTF8), CP_UTF8);

	CHMAC_SHA1 chmacSha1;
	unsigned char outUChar[1024] = {0};	
	chmacSha1.HMAC_SHA1((unsigned char *)(stringToSign.c_str()), (int)(stringToSign.length()), (unsigned char *)(accessSecret.c_str()), (int)(accessSecret.length()), outUChar);

	return ToolFun::EncodeBase64(outUChar, 20);
}

//获取配置参数，生成SMSUrl
string CSystemHander::CreateSMSUrl(wstring mobilePhone, wstring vtnCode) 
{
	//可配置
	string signName = m_AliyunSmsSignName;
	string TemplateCode = m_AliyunSmsTemplateCode;

	string TemplateParam = ToolFun::Format(m_AliyunSmsTemplateParam.c_str(), ToolFun::ConvertUnicodeToMultiBytes(vtnCode).c_str());
	string accessKeyId = m_AliyunSmsAccessKeyId;
	string accessSecret = m_AliyunSmsAccessSecret;

	//不过配置
	map<string, string> sortParas;   //参数KEY排序

	// 1. 系统参数
	sortParas["SignatureMethod"] = "HMAC-SHA1";
	sortParas["SignatureNonce"] = ToolFun::GetRandUUid(); //"cba60810-fd16-4796-9c31-8f558ce8c900";
	sortParas["AccessKeyId"] = accessKeyId;
	sortParas["SignatureVersion"] = "1.0";
	sortParas["Timestamp"] = ToolFun::getCurrentGmtTime("%Y-%m-%dT%H:%M:%SZ"); //"2017-09-26T02:37:56Z";
	sortParas["Format"] = "XML";
	// 2. 业务API参数
	sortParas["Action"] = "SendSms";
	sortParas["Version"] = "2017-05-25";
	sortParas["RegionId"] = "cn-hangzhou";
	sortParas["PhoneNumbers"] = ToolFun::ConvertUnicodeToMultiBytes(mobilePhone);
	sortParas["SignName"] = signName;
	sortParas["TemplateParam"] = TemplateParam;
	sortParas["TemplateCode"] = TemplateCode;

	string sortQueryStringTmp;
	for (map<string, string>::const_iterator iter = sortParas.begin(); 
		iter != sortParas.end(); ++ iter)
	{
		string _key = iter->first;
		string _value = iter->second;
		sortQueryStringTmp += "&" + specialUrlEncode(_key) + "="  + specialUrlEncode(_value);

	}

	string sortedQueryString = sortQueryStringTmp.substr(1); // 去除第一个多余的&符号

	string stringToSign;
	stringToSign += "GET&";
	stringToSign += specialUrlEncode("/") + "&";
	stringToSign += specialUrlEncode(sortedQueryString);

	string sign = StringSign(accessSecret+"&", stringToSign);    //签名

	string signature = specialUrlEncode(sign);  //签名最后也要做特殊URL编码


	return "http://dysmsapi.aliyuncs.com/?Signature=" + signature + sortQueryStringTmp;
}

std::string  CSystemHander::GetErrorMsg(std::string strSrc)
{
	string strRet;
	int nIndex = 0;
	int n = strSrc.find(',');		//第一个逗号的位置
	int nState = strSrc.find('0');
	if (nState > 0 && nState < n)	//如果状态值为0
	{
		return strRet;
	}
	for (int i = 0; i != strSrc.size(); ++i)
	{
		if (':' == strSrc[i])
		{
			++nIndex;
		}
		else if ('}' == strSrc[i])
		{
			break;
		}
		if (2 == nIndex)//获取错误信息
		{
			if (':' == strSrc[i] || '\"' == strSrc[i] || ' ' == strSrc[i]) continue;

			strRet.push_back(strSrc[i]);
		}
	}

	return strRet;
}
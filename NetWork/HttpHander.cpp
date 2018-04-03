#include "StdAfx.h"
#include <curl/curl.h>
#include "HttpHander.h"

CHttpHander::CHttpHander(void)
{
}

CHttpHander::~CHttpHander(void)
{
}

void CHttpHander::InitCURL()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

void CHttpHander::CleanupCURL()
{
	curl_global_cleanup();
}

size_t CHttpHander_OnHttpReqReceive(void *buffer, size_t size, size_t nmemb, void*userp)
{
	std::string tempRes = (char *) buffer;
	tempRes = tempRes.substr(0, nmemb);
	CHttpHander * hander = (CHttpHander *)userp;
	hander->SetReqReceive(tempRes);	
	return size * nmemb;
}

void CHttpHander::SetReqReceive(std::string str)
{
	m_Res = m_Res + str;
}

std::string CHttpHander::get_value(std::string url, int timeOut)
{	
	m_Res = "";	

	CURL *curl;
	CURLcode curl_code;
	curl = curl_easy_init();
	if ( !curl ) {
		return m_Res;
	}

	curl_easy_setopt( curl, CURLOPT_URL, url.c_str());	 
	curl_easy_setopt( curl,  CURLOPT_WRITEDATA, (void*)this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);	 
	curl_code = curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CHttpHander_OnHttpReqReceive);
	curl_code = curl_easy_perform( curl );
	if (curl_code != CURLE_OK)
	{				
	}
	curl_easy_cleanup(curl);
	return m_Res;
}
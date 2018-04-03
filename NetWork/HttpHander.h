#pragma once
#include <map>
#include <string>
using namespace std;

class CHttpHander
{
public:
	CHttpHander(void);
	~CHttpHander(void);

	static void InitCURL();
	static void CleanupCURL();

	std::string get_value(std::string url, int timeOut = 1000);		//ƒ¨»œ10∫¡√Î

	void SetReqReceive(std::string str);

private:

	std::string m_Res;
	std::string		m_strUrl; 
};

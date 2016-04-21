#include "stdafx.h"
#include "DDEControl.h"
#include <map>


std::map<HCONV, CDDEControl*> g_HCONVMap;



HDDEDATA CALLBACK DdeCallback(
	_In_ UINT      uType,
	_In_ UINT      uFmt,
	_In_ HCONV     hconv,
	_In_ HSZ       hsz1,
	_In_ HSZ       hsz2,
	_In_ HDDEDATA  hdata,
	_In_ ULONG_PTR dwData1,
	_In_ ULONG_PTR dwData2
	)
{
	TRACE("DdeCallback, type=%d\n", uType);
	switch (uType)
	{
	case XTYP_REGISTER: {
#if 0
		WCHAR szAPP[256] = { 0 };
		UINT r2 = DdeQueryString(idInst, hsz1, szAPP, 256, 0);
		TRACE(L"service name=%s", szAPP);
		r2 = DdeQueryString(idInst, hsz2, szAPP, 256, 0);
		TRACE(L"item name=%s", szAPP);
#endif
	} return (HDDEDATA)NULL;
	case XTYP_UNREGISTER:

		return (HDDEDATA)NULL;

	case XTYP_ADVDATA: {
		g_HCONVMap[hconv]->onAdvice(hsz1, hsz2, hdata);

	}return (HDDEDATA)DDE_FACK;

	case XTYP_XACT_COMPLETE:

		// 

		return (HDDEDATA)NULL;

	case XTYP_DISCONNECT:

		// 

		return (HDDEDATA)NULL;

	default:
		return (HDDEDATA)NULL;
	}
}


CDDEControl::CDDEControl()
{
	idInst = 0;
	hConv = 0;
	m_pNotifier = NULL;
}


CDDEControl::~CDDEControl()
{
	if (idInst)
	{
		DdeUninitialize(idInst);
		idInst = 0;
	}

	if (hConv)
	{
		g_HCONVMap.erase(hConv);
		DdeDisconnect(hConv);
		hConv = 0;
	}

}



bool CDDEControl::Init()
{
	UINT r = DdeInitialize(&idInst,         // receives instance identifier 
		(PFNCALLBACK)DdeCallback, // pointer to callback function 
		APPCLASS_STANDARD, 0);

	TRACE(L"init result = %d, no error=%d, idInst=%d\r\n", r, DMLERR_NO_ERROR, idInst);
	return (r == DMLERR_NO_ERROR);
}


bool CDDEControl::Connect(CString szServer, CString szTopic)
{
	HSZ hszService = DdeCreateStringHandle(
		idInst,         // instance identifier 
		(LPCTSTR)szServer,     // string to register 
		0);    // Windows ANSI code page   

	HSZ hszTopic = DdeCreateStringHandle(
		idInst,         // instance identifier 
		(LPCTSTR)szTopic,     // string to register 
		0);    // Windows ANSI code page 

	hConv = DdeConnect(
		idInst,
		hszService,
		hszTopic,
		NULL
		);

	if (hszService)
	{
		DdeFreeStringHandle(idInst, hszService);
	}

	if (hszTopic)
	{
		DdeFreeStringHandle(idInst, hszService);
	}

	if (hConv == 0)
	{
		UINT err = DdeGetLastError(idInst);
		TRACE(L"error code =%X\r\n", err);
		return false;
	}

	g_HCONVMap[hConv] = this;

	TRACE(L"hConv=%d\r\n", hConv);
	return true;
}


bool CDDEControl::StartAdvice(CString szItem)
{
	HSZ hszItem = DdeCreateStringHandle(idInst, (LPCTSTR)szItem, 0);
	HDDEDATA dd = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
		XTYP_ADVSTART, 5000, NULL);

	if (hszItem)
	{
		DdeFreeStringHandle(idInst, hszItem);
	}
	TRACE(L"ADVSTART result=%X\r\n", dd);

	if (dd == NULL)
	{
		UINT err = DdeGetLastError(idInst);
		TRACE(L"ADVSTART error code =%X\r\n", err);
		return false;
	}
	return true;
}


bool CDDEControl::StopAdvice(CString szItem)
{
	HSZ hszItem = DdeCreateStringHandle(idInst, (LPCTSTR)szItem, 0);
	HDDEDATA dd = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
		XTYP_ADVSTOP, 5000, NULL);

	if (hszItem)
	{
		DdeFreeStringHandle(idInst, hszItem);
	}

	TRACE(L"ADVSTOP result=%X\r\n", dd);
	
	if (dd == NULL)
	{
		UINT err = DdeGetLastError(idInst);
		TRACE(L"ADVSTOP error code =%X\r\n", err);
		return false;
	}
	return true;
}


bool CDDEControl::Request(CString szItem)
{
	HSZ hszItem = DdeCreateStringHandle(idInst, (LPCTSTR)szItem, 0);
	HDDEDATA hData = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
		XTYP_REQUEST, 5000, NULL);

	if (hszItem)
	{
		DdeFreeStringHandle(idInst, hszItem);
	}

	if (hData == NULL)
	{
		TRACE("Request failed: %s\n", (LPCTSTR)szItem);
		return false;
	}

	char szResult[255];
	DWORD r = DdeGetData(hData, (unsigned char *)szResult, 255, 0);
	szResult[r] = 0;

	TRACE("result %s\n", szResult);

	return true;
}

void CDDEControl::onAdvice(HSZ hsz1, HSZ hsz2, HDDEDATA hData)
{
	char szResult[1024];
	szResult[0] = 0;
	DWORD r = DdeGetData(hData, (unsigned char *)szResult, 1024, 0);
	szResult[r] = 0;

	TCHAR szItem[1024] = { 0 };
	UINT r2 = DdeQueryString(idInst, hsz2, szItem, 1024, 0);
	TRACE(L"item name=%s\n", szItem);
	TRACE("XTYP_ADVDATA =%s\r\n", szResult);

	if (m_pNotifier)
	{
		m_pNotifier->onAdvice(szItem, CString(szResult));
	}
}
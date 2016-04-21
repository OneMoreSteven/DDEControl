#pragma once
#include "ddeml.h"

class CDDEControl
{
public:
	CDDEControl();
	~CDDEControl();
	bool Init();

	class Notifier
	{
	public:
		virtual void onAdvice(CString szItem, CString szData) = 0;
	};
protected:
	DWORD idInst;
	HCONV hConv;
	Notifier* m_pNotifier;

public:
	bool Connect(CString szServer, CString szTopic);
	bool StartAdvice(CString szItem);
	bool StopAdvice(CString szItem);
	bool Request(CString szItem);
	void SetNotifier(Notifier *pNotifier) { m_pNotifier = pNotifier; };
	void onAdvice(HSZ hsz1,HSZ hsz2, HDDEDATA hData);
};



// SJTU_AGV_Dect.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSJTU_AGV_DectApp: 
// �йش����ʵ�֣������ SJTU_AGV_Dect.cpp
//

class CSJTU_AGV_DectApp : public CWinApp
{
public:
	CSJTU_AGV_DectApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSJTU_AGV_DectApp theApp;
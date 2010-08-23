// StarsWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Stars.h"
#include "StarsWnd.h"
#include "jstd/Threading.h"
#include "boost/bind.hpp"


// CStarsWnd

IMPLEMENT_DYNAMIC(CStarsWnd, CWnd)

CStarsWnd::CStarsWnd()
:	m_Rect_Bm(0,0,0,0),
	m_bStopped(true),
	m_bStop(false)
{

}

CStarsWnd::~CStarsWnd()
{
	Stop();
}


BEGIN_MESSAGE_MAP(CStarsWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CStarsWnd message handlers

int CStarsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	Start();

	return 0;
}



void CStarsWnd::OnPaint()
{
	CPaintDC dc(this);

	CRect W_Rect_Client;
	GetClientRect(W_Rect_Client);

	if(W_Rect_Client != m_Rect_Bm)
	{
		m_Bm_Mem.DeleteObject();
		m_Bm_Mem.CreateCompatibleBitmap(&dc, W_Rect_Client.Width(), W_Rect_Client.Height());

		m_Rect_Bm = W_Rect_Client;
	}

	CDC W_Dc;
	W_Dc.CreateCompatibleDC(&dc);
	//W_Dc.CreateCompatibleDC(&dc);
	CBitmap* W_OldBmPtr = W_Dc.SelectObject(&m_Bm_Mem);

	//Hier tekeken




	W_Dc.FillSolidRect(&W_Rect_Client, RGB(0,0,0));

	//Hier klaar
	dc.BitBlt(0,0,W_Rect_Client.Width(), W_Rect_Client.Height(), &W_Dc, 0, 0, SRCCOPY);
	W_Dc.SelectObject(W_OldBmPtr);


	
}

void CStarsWnd::Start()
{
	m_bStop = false;
	if(Threading::ExecAsync(boost::bind(&CStarsWnd::AsyncRun, this)) == 0)
		return;
	m_bStopped = false;

}

void CStarsWnd::Stop()
{
	m_bStop = true;
	while(!m_bStopped)
		Sleep(20);
}

void CStarsWnd::AsyncRun()
{
	while(!m_bStop)
	{

	}
	

	m_bStopped = true;
}

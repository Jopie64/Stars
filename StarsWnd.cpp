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
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CStarsWnd message handlers

int CStarsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	m_vStar.resize(100000);

	RandomInit();

	Start();

	SetTimer(eT_Invalidate, 5, NULL);

	return 0;
}

double RandF()
{
	return 1.0 * (rand() % 0x8000) / 0x8000;
}

void CStarsWnd::RandomInit()
{
	double velocity = 2;
	for(CvStar::iterator i = m_vStar.begin(); i != m_vStar.end(); ++i)
	{
		i->m_vx = velocity * (RandF() - 0.5);
		i->m_vy = velocity * (RandF() - 0.5);
	}
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

	CPoint W_pt_Center = W_Rect_Client.CenterPoint();

	for(CvStar::iterator i = m_vStar.begin(); i != m_vStar.end(); ++i)
	{
		CPoint W_pt((int)i->m_x + W_pt_Center.x, (int)i->m_y + W_pt_Center.y);
		if(!W_Rect_Client.PtInRect(W_pt))
			continue;
		W_Dc.FillSolidRect(CRect(W_pt, CSize(1,1)), RGB(255,255,255));
	}




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
		for(CvStar::iterator i = m_vStar.begin(); i != m_vStar.end(); ++i)
		{
			i->m_vy += 0.001;


			i->m_x += i->m_vx;
			i->m_y += i->m_vy;
		}
	}
	

	m_bStopped = true;
}

void CStarsWnd::OnTimer(UINT_PTR nIDEvent)
{
	CWnd::OnTimer(nIDEvent);
	switch(nIDEvent)
	{
	case eT_Invalidate:	Invalidate(FALSE); break;
	}
}

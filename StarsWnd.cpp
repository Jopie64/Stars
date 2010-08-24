// StarsWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Stars.h"
#include "StarsWnd.h"
#include "jstd/Threading.h"
#include "boost/bind.hpp"
#include <cmath>

using namespace Threading;

const int G_NbStars = 50000;
const double G_RandomInitAllVelocity = 2;
const double G_RandomInitSingleVelocity = 0.1;
const double G_DownwardGravitation = 0.001;
const int G_PullerMass = 2;

CPoint CFPoint::ToScreen(const CPoint& P_pt_Center)const
{
	return CPoint((int)m_x + P_pt_Center.x, 
		          P_pt_Center.y - (int)m_y);
}

CFPoint CFPoint::ToStar(const CPoint& P_pt_Center, const CPoint& P_pt_Screen)
{
	return CFPoint( P_pt_Screen.x - P_pt_Center.x,
					P_pt_Center.y - P_pt_Screen.y);
}

CStar::CStar()
:	m_iIxCur(0)
{
	//m_vPos.resize(G_NbPrevLoc);
}

void CStar::Pos(const CFPoint& P_Pos)
{
	++m_iIxCur;
	if(m_iIxCur == G_NbPrevLoc)
		m_iIxCur = 0;
	m_vPos[m_iIxCur] = P_Pos;
}

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
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CStarsWnd message handlers

int CStarsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_vStarShared.resize(G_NbStars);

	//RandomInit();

	Start();

	//SetTimer(eT_Invalidate, 40, NULL);

	return 0;
}

double RandF()
{
	return 1.0 * (rand() % 0x8000) / 0x8000;
}


void CStarsWnd::RandomInit(CStar& P_Star, double P_Velocity)
{
	P_Star.Velocity(CFPoint(P_Velocity * (RandF() - 0.5), 
		                    P_Velocity * (RandF() - 0.5)));
}


void CStarsWnd::RandomInit(CvStar& P_vStar, double P_Velocity)
{
	for(CvStar::iterator i = P_vStar.begin(); i != P_vStar.end(); ++i)
	{
		RandomInit(*i, P_Velocity);
	}
}

void CStarsWnd::SetPullerPos()
{
	CPoint W_pt_Mouse;// = GetCurrentMessage()->pt;
	GetCursorPos(&W_pt_Mouse);
	ScreenToClient(&W_pt_Mouse);

	CRect W_Rect_Client;
	GetClientRect(W_Rect_Client);
	m_Puller.Pos(CFPoint::ToStar(W_Rect_Client.CenterPoint(), W_pt_Mouse));
}


void CStarsWnd::OnPaint()
{
	CPaintDC dc(this);

	{
		CScopeLock lock(m_Cs);
		SetPullerPos();
		m_vStarMain.swap(m_vStarShared);
	}
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

	DrawStars(W_Dc, m_vStarMain);

	CRect W_Rect_Puller(m_Puller.Pos().ToScreen(W_Rect_Client.CenterPoint()), CSize(4,4));
	W_Dc.FillSolidRect(W_Rect_Puller, RGB(255,0,0));





	//Hier klaar
	dc.BitBlt(0,0,W_Rect_Client.Width(), W_Rect_Client.Height(), &W_Dc, 0, 0, SRCCOPY);
	W_Dc.SelectObject(W_OldBmPtr);
}

void CStarsWnd::DrawStars(CDC& P_Dc, CvStar& P_vStar)
{
	CRect W_Rect_Client;
	GetClientRect(W_Rect_Client);

	CPoint W_pt_Center = W_Rect_Client.CenterPoint();

	for(CvStar::iterator i = P_vStar.begin(); i != P_vStar.end(); ++i)
	{
		CPoint W_pt(i->Pos().ToScreen(W_pt_Center));
		if(!W_Rect_Client.PtInRect(W_pt))
		{
			//*i = CStar();
			//RandomInit(*i, G_RandomInitSingleVelocity);
			continue;
		}
		P_Dc.FillSolidRect(CRect(W_pt, CSize(1,1)), RGB(255,255,255));
	}
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
	m_StarMoveTd.PostQuitMessage();
	while(!m_bStopped)
		Sleep(20);
}

void CStarsWnd::AsyncRun()
{
	m_StarMoveTd.Register();
	int count = 0;
	CvStar W_vStarTemp;
	W_vStarTemp.resize(G_NbStars);
	RandomInit(W_vStarTemp, G_RandomInitAllVelocity);

	CStar W_Puller;
	W_Puller.Pos(CFPoint(10,10));
	while(!m_StarMoveTd.CallMessages())
	{
		//if(count++ % 3 == 0)
		//	Sleep(1);


		for(int bla=0; bla<4; ++bla)
		for(CvStar::iterator i = W_vStarTemp.begin(); i != W_vStarTemp.end(); ++i)
		{
//			if(i->m_x == 0 && i->m_y == 0)
//				int i=0;
			CFPoint W_Velocity = i->Velocity();
			CFPoint W_Pos = i->Pos();
			W_Velocity.m_y -= G_DownwardGravitation;

			double xoff = W_Pos.m_x - W_Puller.Pos().m_x;
			double yoff = W_Pos.m_y - W_Puller.Pos().m_y;
			double distSqr = xoff * xoff + yoff * yoff;
			double dist = sqrt(distSqr);
			double force = G_PullerMass / distSqr;
			double factor = force / dist;


			W_Velocity.m_x -= xoff * factor;
			W_Velocity.m_y -= yoff * factor;


			W_Pos.m_x += W_Velocity.m_x;
			W_Pos.m_y += W_Velocity.m_y;

			i->Velocity(W_Velocity);
			i->Pos(W_Pos);
		}
		{
			CScopeLock lock(m_Cs);
			m_vStarShared = W_vStarTemp;
			W_Puller = m_Puller;
			Invalidate(FALSE);
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

void CStarsWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	{
		CScopeLock lock(m_Cs);
		SetPullerPos();
	}
	Stop();
	Start();
	//RandomInit();
}

// StarsWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Stars.h"
#include "StarsWnd.h"
#include "jstd/Threading.h"
#include "boost/bind.hpp"
#include <cmath>

using namespace Threading;

const double G_Precision = 0.01;
const int G_NbStars = 5000;
const double G_RandomInitAllVelocity = 2 * G_Precision;
const double G_RandomInitSingleVelocity = 0.1 * G_Precision;
const double G_DownwardGravitation = 0.00005 * G_Precision;
const double G_PullerMass = 2 * G_Precision;
const double G_ResetY_Fixed = 3 * G_Precision;
const double G_ResetY_Random = 0.01 * G_Precision;

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
:	m_iIxCur(0),
	m_iPosSkip(0)
{
	//m_vPos.resize(G_NbPrevLoc);
}

void CStar::Pos(const CFPoint& P_Pos)
{
	if(m_iPosSkip <= 0)
	{
		m_iPosSkip = G_NbPrevLoc_Skip;
		++m_iIxCur;
		if(m_iIxCur == G_NbPrevLoc)
			m_iIxCur = 0;
	}
	else
		--m_iPosSkip;
	m_vPos[m_iIxCur] = P_Pos;
}

// CStarsWnd

IMPLEMENT_DYNAMIC(CStarsWnd, CWnd)

CStarsWnd::CStarsWnd()
:	m_Rect_Bm(0,0,0,0),
	m_bStopped(true),
	m_bStop(false),
	m_bDoRandomInit(false)
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
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
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

void PutPixel(CDC& P_Dc, const CPoint& P_Pt, unsigned char P_cBrightness)
{
	//P_Dc.SetPixel(P_Pt, RGB(255,255,255));
	P_Dc.FillSolidRect(CRect(P_Pt, CSize(1,1)), RGB(P_cBrightness,P_cBrightness,P_cBrightness));
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
			ResetStar(i - P_vStar.begin());
			continue;
		}
		//P_Dc.SetPixel(W_pt, RGB(255,255,255));
		//P_Dc.FillSolidRect(CRect(W_pt, CSize(1,1)), RGB(255,255,255));
		int bright = 255 - G_NbPrevLoc * 10;
		for(int ix = i->m_iIxCur + 1; ix != i->m_iIxCur; ++ix >= G_NbPrevLoc ? ix = 0 : ix = ix)
		{
			bright += 10;
			//if(W_iX >= G_NbPrevLoc)
			//	W_iX = 0;
			PutPixel(P_Dc, i->m_vPos[ix].ToScreen(W_pt_Center),bright);
		}
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
	m_vStarWork.clear();
	m_vStarWork.resize(G_NbStars);
	RandomInit(m_vStarWork, G_RandomInitAllVelocity);

	CStar W_Puller;
	W_Puller.Pos(CFPoint(10,10));
	while(!m_StarMoveTd.CallMessages())
	{
		//if(count++ % 3 == 0)
		//	Sleep(1);
		if(m_bDoRandomInit)
		{
			m_bDoRandomInit = false;
			m_vStarWork.clear();
			m_vStarWork.resize(G_NbStars);
			RandomInit(m_vStarWork, G_RandomInitAllVelocity);
		}


		for(int bla=0; bla<4; ++bla)
		for(CvStar::iterator i = m_vStarWork.begin(); i != m_vStarWork.end(); ++i)
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
			m_vStarShared = m_vStarWork;
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

void CStarsWnd::OnDestroy()
{
	CWnd::OnDestroy();

	Stop();
}

void CStarsWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	bool W_bHandled = true;
	switch(nChar)
	{
	case 'X': m_bDoRandomInit = true; break;

	default:
		W_bHandled = false;
	}
	if(!W_bHandled)
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CStarsWnd::ResetStar(int P_iIx)
{
	if(!m_StarMoveTd.IsThisThread())
	{
		m_StarMoveTd.PostCallback(simplebind(&CStarsWnd::ResetStar, this, P_iIx));
		return;
	}
	m_vStarWork[P_iIx].Pos(CFPoint(0,0));
	m_vStarWork[P_iIx].Velocity(CFPoint(0,G_ResetY_Fixed + G_ResetY_Random * RandF()));

}

// StarsWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Stars.h"
#include "StarsWnd.h"
#include "jstd/Threading.h"
#include "JOpenGl.h"
#include <functional>
#include <cmath>
#include <gl/GLU.h>

using namespace JStd::Threading;
using namespace JStd::Wnd;

const double G_Precision = 0.05;
const int G_NbStars = 50000;
const double G_RandomInitAllVelocity = 2 * G_Precision;
const double G_RandomInitSingleVelocity = 0.1 * G_Precision;
const double G_DownwardGravitation = 0.00005 * G_Precision;
const double G_PullerMass = 2 * G_Precision;
const double G_ResetY_Fixed = 3 * G_Precision;
const double G_ResetY_Random = 0.01 * G_Precision;

Point CFPoint::ToScreen(const Point& P_pt_Center)const
{
	Point ptThis(*this);
	ptThis.y = -ptThis.y;
	ptThis += P_pt_Center;
	return ptThis;
}

CFPoint CFPoint::ToStar(Point P_pt_Center, Point P_pt_Screen)
{
	P_pt_Screen.y = -P_pt_Screen.y;
	P_pt_Center.x = -P_pt_Center.x;
	P_pt_Screen += P_pt_Center;
	return CFPoint(P_pt_Screen.x, P_pt_Screen.y);
}

CStar::CStar()
:	m_iIxCur(0),
	m_iPosSkip(0)
{
	//m_vPos.resize(G_NbPrevLoc);
}

void CStar::Pos(const CFPoint& P_Pos)
{
/*	if(m_iPosSkip <= 0)
	{
		m_iPosSkip = G_NbPrevLoc_Skip;
		++m_iIxCur;
		if(m_iIxCur == G_NbPrevLoc)
			m_iIxCur = 0;
	}
	else
		--m_iPosSkip;
	m_vPos[m_iIxCur] = P_Pos;
*/
	m_Pos = P_Pos;
}

// CStarsWnd

CStarsWnd::CStarsWnd()
:	m_Rect_Bm(0,0,0,0),
	m_bStopped(true),
	m_bStop(false),
	m_bDoRandomInit(false),
	m_timeLastMove(std::chrono::steady_clock::now())
{
	m_Puller.m_Color.g = 0.6f;
	m_Puller.m_Color.b = 0.6f;
}

CStarsWnd::~CStarsWnd()
{
	Stop();
}

void CStarsWnd::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
//	case WM_PAINT:	OnPaint(); break;
	case WM_CREATE:	OnCreate((LPCREATESTRUCT)lParam); break;
	case WM_TIMER:	OnTimer(wParam); break;
	case WM_SIZE:	OnSize(wParam, LOWORD(lParam), HIWORD(lParam)); break;
	case WM_DESTROY:OnDestroy(); break;
	case WM_KEYDOWN:OnKeyDown(wParam, LOWORD(lParam), HIWORD(lParam)); break;
	}
}


// CStarsWnd message handlers

void CStarsWnd::InitAndRun()
{
	m_vStar.resize(G_NbStars);

	Start();
}

int CStarsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	InitAndRun();
	return 0;
}

double RandF()
{
	const double small = (1.0 / 0x8000) / 0x8000;
	double rand1 = small * (rand() % 0x8000);
	return rand1 + 1.0 * (rand() % 0x8000) / 0x8000;
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
	POINT W_pt_Mouse;// = GetCurrentMessage()->pt;
	GetCursorPos(&W_pt_Mouse);
	::ScreenToClient(H(), &W_pt_Mouse);

	W_pt_Mouse.y = GetClientRect().br.y - W_pt_Mouse.y;

	GLdouble modelMatrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	GLdouble projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	CFPoint newPos;
	double dummy;
	gluUnProject(W_pt_Mouse.x, W_pt_Mouse.y, 0, modelMatrix, projMatrix, viewPort, &newPos.x, &newPos.y, &dummy);

	//m_Puller.Pos(CFPoint::ToStar(GetClientRect().CenterPoint(), JStd::Wnd::ToPoint(W_pt_Mouse)));
	m_Puller.Pos(newPos);
}

void CStarsWnd::RenderStars(size_t size, CStar* stars)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	//m_vStar
	//glTranslatef(0.0f, 0.0f, -1.0f);

	glVertexPointer(2, GL_DOUBLE, sizeof(CStar), ((char*) &stars[0]) + offsetof(CStar, m_Pos));
	glColorPointer(3, GL_FLOAT, sizeof(CStar), ((char*) &stars[0]) + offsetof(CStar, m_Color));
	glDrawArrays(GL_POINTS, 0, size);


	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void CStarsWnd::RenderStars()
{
	RenderStars(m_vStar.size(), &m_vStar[0]);
}

void CStarsWnd::RenderPuller()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	RenderStars(1, &m_Puller);
}


void CStarsWnd::RenderFrame()
{
	SetPullerPos();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	RenderStars();
	RenderPuller();

	glPopMatrix();

}

void CStarsWnd::Start()
{
	SetPullerPos();
	m_bStop = false;
	if (JStd::Threading::ExecAsync([=](){ AsyncRun(); }) == 0)
		return;
	m_bStopped = false;

}

void CStarsWnd::Stop()
{
	m_StarMoveTd.PostQuitMessage();
	while(!m_bStopped)
		Sleep(20);
}

bool IsOOB(const CStar& star)
{
	return abs(star.m_Pos.x) >= 1.0 && abs(star.m_Pos.y) >= 1.0;
}

void CStarsWnd::AsyncRun()
{
	using namespace std::chrono;
	m_StarMoveTd.Register();
	int count = 0;
//	m_vStar.resize(G_NbStars);
	RandomInit(m_vStar, G_RandomInitAllVelocity);

	CStar W_Puller;
	W_Puller.Pos(CFPoint(10,10));
	while(!m_StarMoveTd.CallMessages())
	{
		steady_clock::time_point now = steady_clock::now();
		double moveSeconds = duration_cast<duration<double>>(now - m_timeLastMove).count();
		m_timeLastMove = now;
		//if(count++ % 3 == 0)
		//	Sleep(1);
		if(m_bDoRandomInit)
		{
			m_bDoRandomInit = false;
			m_vStar.resize(G_NbStars);
			RandomInit(m_vStar, G_RandomInitAllVelocity);
		}


		for(int bla=0; bla<4; ++bla)
		for(CvStar::iterator i = m_vStar.begin(); i != m_vStar.end(); ++i)
		{
//			if(i->x == 0 && i->y == 0)
//				int i=0;
			CFPoint W_Velocity = i->Velocity();
			CFPoint W_Pos = i->Pos();
			W_Velocity.y -= G_DownwardGravitation * moveSeconds;

			double xoff = W_Pos.x - W_Puller.Pos().x;
			double yoff = W_Pos.y - W_Puller.Pos().y;
			double distSqr = xoff * xoff + yoff * yoff;
			double dist = sqrt(distSqr);
			double force = G_PullerMass / distSqr;
			double factor = moveSeconds * force / dist;


			W_Velocity.x -= xoff * factor;
			W_Velocity.y -= yoff * factor;


			W_Pos.x += W_Velocity.x * moveSeconds;
			W_Pos.y += W_Velocity.y * moveSeconds;

			i->Velocity(W_Velocity);
			i->Pos(W_Pos);
			if (IsOOB(*i))
				ResetStar(i - m_vStar.begin());
		}
		{
			CScopeLock lock(m_Cs);
			W_Puller = m_Puller;
			//Invalidate(FALSE);
		}
	}
	

	m_bStopped = true;
}

void CStarsWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case eT_Invalidate:	Invalidate(FALSE); break;
	}
}

void CStarsWnd::OnSize(UINT nType, int cx, int cy)
{
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
}

void CStarsWnd::ResetStar(int P_iIx)
{
	if(!m_StarMoveTd.IsThisThread())
	{
		m_StarMoveTd.PostCallback([=](){ ResetStar(P_iIx); });
		return;
	}
	m_vStar[P_iIx].Pos(CFPoint(0,0));
	m_vStar[P_iIx].Velocity(CFPoint(0,G_ResetY_Fixed + G_ResetY_Random * RandF()));

}

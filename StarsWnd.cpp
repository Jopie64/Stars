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
#include <sstream>

using namespace JStd::Threading;
using namespace JStd::Wnd;

const double G_Precision = 0.005;
const int G_NbStars = 1000000;
const double G_RandomInitAllVelocity = 2 * G_Precision;
const double G_RandomInitSingleVelocity = 0.1 * G_Precision;
const double G_DownwardGravitation = 0.00005 * G_Precision;
const double G_PullerMass = G_Precision;
const double G_ResetY_Fixed = 3 * G_Precision;
const double G_ResetY_Random = 0.01 * G_Precision;
const int G_NbThreads = 3;

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

CStarsWnd::CStarsWnd(JStd::GL::PGlWnd P_pWnd)
:	m_Rect_Bm(0,0,0,0),
	m_bStop(false),
	m_bDoRandomInit(false),
	m_pWnd(P_pWnd)
{
	m_Puller.m_Color.g = 0.6f;
	m_Puller.m_Color.b = 0.6f;

	Attach(P_pWnd);

	m_pWnd->GetMainDC().Select(GetStockObject(SYSTEM_FONT));
	wglUseFontBitmaps(m_pWnd->GetMainDC().H(), 0, 255, 1000);
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
	glPushMatrix();
	//glTranslatef(0.0f, 0.0f, 0.1f);
	//glRotatef(0.0f, 0.0f, 1.0f, 0.0f);

	glVertexPointer(2, GL_DOUBLE, sizeof(CStar), ((char*) &stars[0]) + offsetof(CStar, m_Pos));
	glColorPointer (3, GL_FLOAT,  sizeof(CStar), ((char*) &stars[0]) + offsetof(CStar, m_Color));
	glDrawArrays(GL_POINTS, 0, size);
	glPopMatrix();


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

void CStarsWnd::RenderText()
{
	glListBase(1000);
	glPushMatrix();
	glCallLists(24, GL_UNSIGNED_BYTE, "Hello Windows OpenGL World");
	glPopMatrix();
}


void CStarsWnd::RenderFrame()
{
	SetPullerPos();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

	RenderStars();
	RenderPuller();
	RenderText();

	glPopMatrix();

}

void CStarsWnd::Start()
{
	SetPullerPos();
	size_t starsPerThread = m_vStar.size() / G_NbThreads;
	m_bStop = false;
	size_t firstStar = 0;
	for (size_t td = 0; td < G_NbThreads; ++td)
	{
		m_StarMoveTdList.emplace_back();
		if (starsPerThread + firstStar > m_vStar.size())
			starsPerThread = firstStar - m_vStar.size();
		if (JStd::Threading::ExecAsync([=](){ AsyncRun(&m_StarMoveTdList.back(), firstStar, starsPerThread); }) == 0)
			return;
		firstStar += starsPerThread;
	}
}

void CStarsWnd::Stop()
{
	for (auto &i : m_StarMoveTdList)
		i.PostQuitMessage();

	bool stopped = true;
	do
	{
		Sleep(20);
		stopped = true;
		for (auto &i : m_StarMoveTdList)
			if (i.IsRegistered())
				stopped = false;
	}
	while (!stopped);
}

bool IsOOB(const CStar& star)
{
	return abs(star.m_Pos.x) >= 1.0 && abs(star.m_Pos.y) >= 1.0;
}

void CStarsWnd::AsyncRun(JStd::Threading::CMsgThread* ptd, size_t starBegin, size_t starCount)
{
	using namespace std::chrono;
	ptd->Register();
//	m_vStar.resize(G_NbStars);
	RandomInit(m_vStar, G_RandomInitAllVelocity);

	__int64 count = 0;

	steady_clock::time_point timeLastMove = steady_clock::now();
	CStar W_Puller;
	W_Puller.Pos(CFPoint(10,10));
	while (!ptd->CallMessages())
	{
		steady_clock::time_point now = steady_clock::now();
		double moveSeconds = duration_cast<duration<double>>(now - timeLastMove).count();
		timeLastMove = now;
		if (count++ % 100 == 0)
		{
			std::wostringstream os;
			os << L"Seconds: " << moveSeconds;
//			MessageBox(NULL, os.str().c_str(), L"Timing", MB_ICONINFORMATION);
		}
		if(m_bDoRandomInit)
		{
			m_bDoRandomInit = false;
			m_vStar.resize(G_NbStars);
			RandomInit(m_vStar, G_RandomInitAllVelocity);
		}

		CvStar::iterator iBegin = m_vStar.begin() + starBegin;
		CvStar::iterator iEnd = iBegin + starCount;
		for (int bla = 0; bla < 4; ++bla)
		for (CvStar::iterator i = iBegin; i != iEnd; ++i)
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
		}
	}

	ptd->Unregister();
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
//	if(!m_StarMoveTd.IsThisThread())
//	{
//		m_StarMoveTd.PostCallback([=](){ ResetStar(P_iIx); });
//		return;
//	}
	m_vStar[P_iIx].Pos(CFPoint(0,0));
	m_vStar[P_iIx].Velocity(CFPoint(0,G_ResetY_Fixed + G_ResetY_Random * RandF()));

}

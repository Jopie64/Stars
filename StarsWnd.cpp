// StarsWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Stars.h"
#include "StarsWnd.h"
#include "jstd/Threading.h"
#include "boost/bind.hpp"
#include <cmath>
#include <gl/gl.h>
#include <gl/glu.h>
//#include <gl/glaux.h>

using namespace Threading;

const double G_Precision = 0.01;
const int G_NbStars = 5000;
const double G_RandomInitAllVelocity = 2 * G_Precision;
const double G_RandomInitSingleVelocity = 0.1 * G_Precision;
const double G_DownwardGravitation = 0.00005 * G_Precision;
const double G_PullerMass = 20000000000 * G_Precision;
const double G_ResetY_Fixed = 3 * G_Precision;
const double G_ResetY_Random = 0.01 * G_Precision;

const int G_xBound = 1500;
const int G_yBound = 1000;

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

class CGlMatrixScope
{
public:
	CGlMatrixScope(){glPushMatrix();}
	~CGlMatrixScope(){glPopMatrix();}
};

class CGlMode
{
public:
	CGlMode(GLenum P_eMode){glBegin(P_eMode);}
	~CGlMode(){glEnd();}
};

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

void CStar::Reset(const CFPoint& P_Pos)
{
	for(int i=0; i<G_NbPrevLoc; ++i)
		m_vPos[i] = P_Pos;
}


void DrawCube(float xPos, float yPos, float zPos)
{
	//CGlMatrixScope W_Matrix;
	//glBegin(GL_POLYGON);
	CGlMode W_Mode(GL_POLYGON);

	/*      This is the top face*/
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-1.0f, 0.0f, -1.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);

	/*      This is the front face*/
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, 0.0f);

	/*      This is the right face*/
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, -1.0f);
	glVertex3f(0.0f, 0.0f, -1.0f);

	/*      This is the left face*/
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);

	/*      This is the bottom face*/
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);

	/*      This is the back face*/
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(0.0f, -1.0f, -1.0f);

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

void SetupPixelFormat(HDC hDC)
{
	/*      Pixel format index
	*/
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),          //size of structure
			1,                                      //default version
			PFD_DRAW_TO_WINDOW |                    //window drawing support
			PFD_SUPPORT_OPENGL |                    //opengl support
			PFD_DOUBLEBUFFER,                       //double buffering support
			PFD_TYPE_RGBA,                          //RGBA color mode
			32,                                     //32 bit color mode
			0, 0, 0, 0, 0, 0,                       //ignore color bits
			0,                                      //no alpha buffer
			0,                                      //ignore shift bit
			0,                                      //no accumulation buffer
			0, 0, 0, 0,                             //ignore accumulation bits
			16,                                     //16 bit z-buffer size
			0,                                      //no stencil buffer
			0,                                      //no aux buffer
			PFD_MAIN_PLANE,                         //main drawing plane
			0,                                      //reserved
			0, 0, 0 };                              //layer masks ignored

			/*      Choose best matching format*/
			nPixelFormat = ChoosePixelFormat(hDC, &pfd);

			/*      Set the pixel format to the device context*/
			SetPixelFormat(hDC, nPixelFormat, &pfd);
}

// CStarsWnd message handlers

int CStarsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//CPaintDC dc(this);
	m_DC.Attach(GetDC()->m_hDC);

	SetupPixelFormat(m_DC.m_hDC);
	m_hRC = wglCreateContext(m_DC.m_hDC);
	wglMakeCurrent(m_DC.m_hDC, m_hRC);



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

void CStarsWnd::RenderStars(CvStar& P_vStar)
{
 	for(CvStar::iterator i = P_vStar.begin(); i != P_vStar.end(); ++i)
		RenderStar(*i);
}

void CStarsWnd::RenderStar(CStar& P_Star)
{
//	CGlMatrixScope W_GlMat;
//	glTranslated(P_Star.Pos().m_x, P_Star.Pos().m_y, 0);


	CGlMode W_GlMode(GL_LINE_STRIP);
	//glVertex3f(P_Star.Pos().m_x, P_Star.Pos().m_y, 0);
	int bright = 255 - G_NbPrevLoc * 10;
	for(int ix = P_Star.m_iIxCur + 1; ix != P_Star.m_iIxCur; ++ix)
	{
		if(ix >= G_NbPrevLoc)
		{
			ix = 0;
			if(P_Star.m_iIxCur == ix)
				break;
		}
		bright += 10;
		float brightf = 1.0 * bright / 256;
		//if(W_iX >= G_NbPrevLoc)
		//	W_iX = 0;
		glColor3f(brightf, brightf, brightf);
		glVertex3f(P_Star.m_vPos[ix].m_x, P_Star.m_vPos[ix].m_y, 0);
	}

	//glVertex3f(0, 0, 0.0f);
}

void CStarsWnd::RenderPuller(CStar& P_Puller)
{
	CGlMatrixScope W_GlMat;
	glTranslated(P_Puller.Pos().m_x, P_Puller.Pos().m_y, 0);

	glScalef(10.0f, 10.0f, 0.0f);
	
	CGlMode W_GlMode(GL_POLYGON);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
}

void CStarsWnd::Render()
{
    /*      Enable depth testing
    */
    //glEnable(GL_DEPTH_TEST);

    /*      Heres our rendering. Clears the screen
            to black, clear the color and depth
            buffers, and reset our modelview matrix.
    */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

	{

		//glLoadIdentity();
        glTranslatef(0.0f, 0.0f,-1000.0f);
		RenderStars(m_vStarMain);
		RenderPuller(m_Puller);

	}

    glFlush();

    /*      Bring back buffer to foreground
    */
	SwapBuffers(m_DC.m_hDC);
}

void CStarsWnd::OnPaint()
{
	//CPaintDC dc(this);
	{
		CScopeLock lock(m_Cs);
		SetPullerPos();
		m_vStarMain.swap(m_vStarShared);
	}

	Render();return;


	CDC& dc = m_DC;

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
			//double factor = force / dist;
			double factor = dist / force;
			//double factor = 0.00000001 * dist / force;


			W_Velocity.m_x -= xoff * factor;
			W_Velocity.m_y -= yoff * factor;


			W_Pos.m_x += W_Velocity.m_x;
			W_Pos.m_y += W_Velocity.m_y;

			if(abs(W_Pos.m_x) > G_xBound || abs(W_Pos.m_y) > G_yBound)
				ResetStar(i - m_vStarWork.begin());
			else
			{
				i->Velocity(W_Velocity);
				i->Pos(W_Pos);
			}
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

	if(cy == 0)
		cy = 1; //prevent div by zero

	glViewport(0, 0, cx, cy);

	/*      Set current Matrix to projection*/
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); //reset projection matrix

	/*      Time to calculate aspect ratio of
			our window.
	*/
	gluPerspective(54.0f, (GLfloat)cx/(GLfloat)cy, 1.0f, 1000.0f);

	glMatrixMode(GL_MODELVIEW); //set modelview matrix
	glLoadIdentity(); //reset modelview matrix

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
	wglMakeCurrent(m_DC.m_hDC, NULL);
	wglDeleteContext(m_hRC);
	m_hRC = NULL;

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
	m_vStarWork[P_iIx].Reset(CFPoint(0,0));
	m_vStarWork[P_iIx].Velocity(CFPoint(0,G_ResetY_Fixed + G_ResetY_Random * RandF()));

}

#pragma once

#include <vector>
#include <list>
#include <chrono>
#include "JStd/Threading.h"
#include "jstd/JGraphics.h"
#include "jstd/JOpenGl.h"

const int G_NbPrevLoc = 10;
const int G_NbPrevLoc_Skip = 80;
using JStd::Wnd::DC;
using JStd::Wnd::Bitmap;
using JStd::Wnd::Rect;
using JStd::Wnd::Point;

// CStarsWnd
class CFPoint : public JStd::Graphics::Point2d<double>
{
public:
	CFPoint(){}
	CFPoint(double P_x, double P_y):Point2d(P_x, P_y){}

	static CFPoint	ToStar(Point P_pt_Center, Point P_pt_Screen);


};

class GlColor
{
public:
	GlColor() : r(1.0f), g(1.0f), b(1.0f) {}
	float r;
	float g;
	float b;
};

class CStar
{
public:
	typedef std::vector<CFPoint> CvPoint;
	CStar();

//	const CFPoint& Pos()const{return m_vPos[m_iIxCur];}
	const CFPoint& Pos()const{ return m_Pos; }
	const CFPoint& Velocity()const{ return m_Velocity; }
	
	void Pos(const CFPoint& P_Pos);
	void Velocity(const CFPoint& P_Velocity){m_Velocity = P_Velocity;}


	//CvPoint m_vPos;
	///CFPoint m_vPos[G_NbPrevLoc];
	CFPoint m_Pos;
	int		m_iIxCur;
	int		m_iPosSkip;
	CFPoint m_Velocity;

	GlColor	m_Color;
};

typedef std::vector<CStar> CvStar;

class CStarsWnd : public JStd::Wnd::WndSubclass
{
public:
	CStarsWnd(JStd::GL::PGlWnd P_pWnd);
	virtual ~CStarsWnd();

	enum eTimer
	{
		eT_Invalidate
	};

public:
//	void OnPaint();

	void		SetPullerPos();
//	void		DrawStars(DC& P_Dc, CvStar& P_vStar);

	void		RandomInit(CvStar& P_vStars, double P_Velocity);
	void		RandomInit(CStar& P_Star, double P_Velocity);
	void		InitAndRun();

	void		Start();
	void		Stop();

	void		RenderFrame();
	void		RenderStars();
	void		RenderStars(size_t size, CStar* stars);
	void		RenderPuller();
	void		RenderText();

	virtual void	WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
	void		AsyncRun(JStd::Threading::CMsgThread* ptd, size_t starBegin, size_t starCount);

	Bitmap	m_Bm_Mem;
	Rect	m_Rect_Bm;

	bool	m_bStop;

	JStd::Threading::CCritSect m_Cs;

	bool	m_bDoRandomInit;

	void	ResetStar(int P_iIx);


//Star
	CStar	m_Puller;
	CvStar	m_vStar;

	std::list<JStd::Threading::CMsgThread> m_StarMoveTdList;

	JStd::GL::PGlWnd m_pWnd;


	int OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	void OnTimer(UINT_PTR nIDEvent);
	void OnSize(UINT nType, int cx, int cy);
	void OnDestroy();
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};



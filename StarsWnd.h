#pragma once

#include <vector>
#include "JStd/Threading.h"

const int G_NbPrevLoc = 10;
const int G_NbPrevLoc_Skip = 80;

// CStarsWnd
class CFPoint
{
public:
	CFPoint():m_x(0), m_y(0){}
	CFPoint(double P_x, double P_y):m_x(P_x), m_y(P_y){}

	CPoint			ToScreen(const CPoint& P_pt_Center)const;
	static CFPoint	ToStar(const CPoint& P_pt_Center, const CPoint& P_pt_Screen);

	double m_x;
	double m_y;
};

class CStar
{
public:
	typedef std::vector<CFPoint> CvPoint;
	CStar();

	const CFPoint& Pos()const{return m_vPos[m_iIxCur];}
	const CFPoint& Velocity()const{return m_Velocity;}
	
	void Pos(const CFPoint& P_Pos);
	void Velocity(const CFPoint& P_Velocity){m_Velocity = P_Velocity;}


	//CvPoint m_vPos;
	CFPoint m_vPos[G_NbPrevLoc];
	int		m_iIxCur;
	int		m_iPosSkip;
	CFPoint m_Velocity;
};

typedef std::vector<CStar> CvStar;

class CStarsWnd : public CWnd
{
	DECLARE_DYNAMIC(CStarsWnd)

public:
	CStarsWnd();
	virtual ~CStarsWnd();

	enum eTimer
	{
		eT_Invalidate
	};

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	void		Render();

	void		SetPullerPos();
	void		DrawStars(CDC& P_Dc, CvStar& P_vStar);

	void		RandomInit(CvStar& P_vStars, double P_Velocity);
	void		RandomInit(CStar& P_Star, double P_Velocity);

	void		Start();
	void		Stop();
private:
	void		AsyncRun();

	CBitmap m_Bm_Mem;
	CRect	m_Rect_Bm;

	bool	m_bStop;
	bool	m_bStopped;

	Threading::CCritSect m_Cs;

	bool	m_bDoRandomInit;

	void	ResetStar(int P_iIx);


//Star
	CStar	m_Puller;
	CvStar	m_vStarShared;
	CvStar	m_vStarMain;
	CvStar	m_vStarWork;
	
	HGLRC	m_hRC;
	CDC		m_DC;

	Threading::CMsgThread m_StarMoveTd;


	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};



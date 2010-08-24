#pragma once

#include <vector>

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
	CStar(){}

	const CFPoint& Pos()const{return m_Pos;}
	const CFPoint& Velocity()const{return m_Velocity;}
	
	void Pos(const CFPoint& P_Pos){m_Pos = P_Pos;}
	void Velocity(const CFPoint& P_Velocity){m_Velocity = P_Velocity;}


	CFPoint m_Pos;
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

	void		SetPullerPos();
	void		DrawStars(CDC& P_Dc);

	void		RandomInit();
	void		RandomInit(CStar& P_Star, double P_Velocity);

	void		Start();
	void		Stop();
private:
	void		AsyncRun();

	CBitmap m_Bm_Mem;
	CRect	m_Rect_Bm;

	bool	m_bStop;
	bool	m_bStopped;



//Star
	CStar	m_Puller;
	CvStar	m_vStar;


	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};



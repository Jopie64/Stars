#pragma once

#include <vector>

// CStarsWnd
class CStar
{
public:
	CStar():m_x(0), m_y(0), m_vx(0), m_vy(0){}

	CPoint  ToScreen(const CPoint& P_pt_Center);
	void	ToStar(const CPoint& P_pt_Center, const CPoint& P_pt_Screen);

	double m_x;
	double m_y;

	double m_vx;
	double m_vy;
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



#pragma once


// CStarsWnd

class CStarsWnd : public CWnd
{
	DECLARE_DYNAMIC(CStarsWnd)

public:
	CStarsWnd();
	virtual ~CStarsWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();

	void		Start();
	void		Stop();
private:
	void		AsyncRun();

	CBitmap m_Bm_Mem;
	CRect	m_Rect_Bm;

	bool	m_bStop;
	bool	m_bStopped;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};



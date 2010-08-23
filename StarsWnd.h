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

	CBitmap m_Bm_Mem;
	CRect	m_Rect_Bm;
};



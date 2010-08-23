// StarsWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Stars.h"
#include "StarsWnd.h"


// CStarsWnd

IMPLEMENT_DYNAMIC(CStarsWnd, CWnd)

CStarsWnd::CStarsWnd()
:	m_Rect_Bm(0,0,0,0)
{

}

CStarsWnd::~CStarsWnd()
{
}


BEGIN_MESSAGE_MAP(CStarsWnd, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CStarsWnd message handlers



void CStarsWnd::OnPaint()
{
	CPaintDC dc(this);

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

	//Hier klaar
	dc.BitBlt(0,0,W_Rect_Client.Width(), W_Rect_Client.Height(), &W_Dc, 0, 0, SRCCOPY);
	W_Dc.SelectObject(W_OldBmPtr);


	
}

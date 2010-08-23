
// StarsDlg.h : header file
//

#pragma once

#include "StarsWnd.h"

// CStarsDlg dialog
class CStarsDlg : public CDialog
{
// Construction
public:
	CStarsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_STARS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	CStarsWnd m_StarsWnd;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

	void RelocateControls();
};

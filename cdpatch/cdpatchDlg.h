// cdpatchDlg.h : header file
//

#pragma once


// CcdpatchDlg dialog
class CcdpatchDlg : public CDialog
{
// Construction
public:
	CcdpatchDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CDPATCH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedBrowse();
   afx_msg void OnEnChangePath();
   afx_msg void OnBnClickedPatch();
};

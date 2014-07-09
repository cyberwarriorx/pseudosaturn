// cdpatchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cdpatch.h"
#include "cdpatchDlg.h"
#include "ImgUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CcdpatchDlg dialog




CcdpatchDlg::CcdpatchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CcdpatchDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CcdpatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CcdpatchDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
   ON_BN_CLICKED(IDC_BROWSE, &CcdpatchDlg::OnBnClickedBrowse)
   ON_EN_CHANGE(IDC_PATH, &CcdpatchDlg::OnEnChangePath)
   ON_BN_CLICKED(IDC_PATCH, &CcdpatchDlg::OnBnClickedPatch)
END_MESSAGE_MAP()


// CcdpatchDlg message handlers

BOOL CcdpatchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
   GetDlgItem(IDC_PATCH)->EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CcdpatchDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CcdpatchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CcdpatchDlg::OnBnClickedBrowse()
{
   TCHAR strFilter[] = { _T("All supported types|*.iso;*.cue|ISO Files (*.iso)|*.iso|CUE Files (*.cue)|*.cue|All Files (*.*)|*.*||") };

   CFileDialog FileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, strFilter);

   if( FileDlg.DoModal() == IDOK )
   {
      CString path = FileDlg.GetPathName();
      SetDlgItemText(IDC_PATH, path);
   }
   else
      return;

}

void CcdpatchDlg::OnEnChangePath()
{
   CString path;

   GetDlgItemText(IDC_PATH, path);
   if (path.GetLength() != 0)
      GetDlgItem(IDC_PATCH)->EnableWindow(TRUE);
}

void CcdpatchDlg::OnBnClickedPatch()
{
   CString path;
	DWORD ret, ret2;

   GetDlgItemText(IDC_PATH, path);

   ImgUtil img(path);

	if ((ret = img.detectImage()) == ERROR_SUCCESS && (ret2 = img.patch(0, 0, (unsigned char *)"PSEUDO SATURN   ", 16)) == ERROR_SUCCESS)
	   MessageBox(_T("Successfully completed"), _T("Notice"), MB_OK);
	else
	{
		img.error(GetSafeHwnd(), ret != ERROR_SUCCESS ? _T("detecting image") : _T("patching image"), ret != ERROR_SUCCESS ? ret : ret2);
	}
}

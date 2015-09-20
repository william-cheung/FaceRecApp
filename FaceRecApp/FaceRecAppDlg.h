
// FaceRecAppDlg.h : header file
//

#pragma once

#include <opencv2\opencv.hpp>

// CFaceRecAppDlg dialog
class CFaceRecAppDlg : public CDialogEx
{
// Construction
public:
	CFaceRecAppDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FACERECAPP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CString m_output;
	CEdit m_editControl;

public:
	afx_msg void OnBnClickedBtnExit();
	afx_msg void OnBnClickedBtnOpenc();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	afx_msg void OnBnClickedBtnLogin();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void DrawPicToHDC(IplImage* image, UINT ID);
	void ShowTextToOutput(CString text);
	afx_msg void OnBnClickedBtnAbout();
	afx_msg void OnBnClickedBtnSignup();
	afx_msg void OnBnClickedBtnViewr();
	CSpinButtonCtrl m_spin;
//	float m_threshold;
	double m_threshold;
	afx_msg void OnDeltaposSpinThres(NMHDR *pNMHDR, LRESULT *pResult);
}; // end FaceRecAppDlg.h

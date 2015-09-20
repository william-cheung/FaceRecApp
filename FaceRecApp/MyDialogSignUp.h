#pragma once

#include <opencv2\opencv.hpp>

// CMyDialogSignUp dialog

class CMyDialogSignUp : public CDialogEx
{
	DECLARE_DYNAMIC(CMyDialogSignUp)

public:
	CMyDialogSignUp(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMyDialogSignUp();

// Dialog Data
	enum { IDD = IDD_DIALOG_SIGNUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	IplImage*	m_userFace;
	CStatic		m_staticTextControl;
	CString		m_name;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
private:
	void DrawPicToHDC(const IplImage* image, UINT ID);
public:
//	afx_msg void OnBnClickedOk();
//	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
};

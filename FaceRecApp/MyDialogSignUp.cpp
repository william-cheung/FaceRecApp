// MyDialogSignUp.cpp : implementation file
//

#include "stdafx.h"
#include "FaceRecApp.h"
#include "MyDialogSignUp.h"
#include "afxdialogex.h"

#include "CvvImage.h"
#include "facerec.h"
#include "Person.h"

// Global Variables
extern IplImage* faceImage;
extern vector<CPerson> persons;

// CMyDialogSignUp dialog

IMPLEMENT_DYNAMIC(CMyDialogSignUp, CDialogEx)

CMyDialogSignUp::CMyDialogSignUp(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMyDialogSignUp::IDD, pParent)
{
	m_userFace = NULL;
	m_name = _T("");
}

CMyDialogSignUp::~CMyDialogSignUp()
{
	cvReleaseImage(&m_userFace);
}

void CMyDialogSignUp::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_NAME, m_staticTextControl);
	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
}


BEGIN_MESSAGE_MAP(CMyDialogSignUp, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDOK, &CMyDialogSignUp::OnBnClickedOk)
END_MESSAGE_MAP()


// CMyDialogSignUp message handlers


BOOL CMyDialogSignUp::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CRect rect;
	GetDlgItem(IDC_PICBOX_FACE)->GetClientRect(&rect);
	CvSize picBoxSize = cvSize(rect.Width(), rect.Height());
	if (faceImage != NULL)
	{
		if (m_userFace != NULL)
			cvReleaseImage(&m_userFace);
		m_userFace = resizeImage(faceImage, picBoxSize);
	} 

	m_staticTextControl.SetWindowText(_T("Name:"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMyDialogSignUp::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	if (m_userFace != NULL)
		DrawPicToHDC(m_userFace, IDC_PICBOX_FACE);

	// Do not call CDialogEx::OnPaint() for painting messages
}


void CMyDialogSignUp::DrawPicToHDC(const IplImage* image, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	if (image == NULL)
	{
		pDC->FillSolidRect(&rect, RGB(240, 240, 240));
		pDC->DrawEdge(&rect, EDGE_BUMP, BF_RECT);
	}
	else
	{
		CvvImage cimg;
		cimg.CopyOf(image);
		cimg.DrawToHDC(hDC, &rect);
	}
	ReleaseDC(pDC);
}

#ifdef UNICODE 
	#define IS_ALPHA(C)	iswalpha(C)
	#define IS_SPACE(C)	iswspace(C)
#else
	#define IS_ALPHA(C)	isalpha(C)
	#define IS_SPACE(C)	isspace(C)
#endif

CString strtrim(const CString& src)
{
	int len = src.GetLength();
	int i = 0, j = len - 1;
	while (i < len && IS_SPACE(src[i]))
		i++;
	while (j > 0 && IS_SPACE(src[j]))
		j--;

	CString dst = _T("");
	for (int k = i; k <= j; k++)
	{
		if (IS_ALPHA(src[k]) || IS_SPACE(src[k]))
			dst.AppendChar(src[k]);
		else
		{
			dst = _T("");
			break;
		}
	}
	return dst;
}

void CMyDialogSignUp::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString personName = strtrim(m_name); 
	if (m_userFace == NULL || personName == _T(""))
	{
		MessageBox(_T("Illegal 'Face' or 'Info'!"), _T("Error!"));
		return;
	}
	UpdateData(FALSE);

	CPerson person(personName);
	persons.push_back(person);
	if (!train(m_userFace))
		MessageBox(_T("Can't sign up!"), _T("Error!"));

	CDialogEx::OnOK();
}

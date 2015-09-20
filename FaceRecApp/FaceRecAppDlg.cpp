
// FaceRecAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FaceRecApp.h"
#include "FaceRecAppDlg.h"
#include "afxdialogex.h"

#include "MyDialogSignUp.h"

#include "CvvImage.h"
#include "facerec.h"
#include "Person.h"
#include "Html.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define		TIMER_INTERVAL		50  // fps = 20

// Global Variables
IplImage* faceImage = NULL;
vector<CPerson> persons;

// Variables visible only in this file
static CvCapture*	capture = NULL;
static CvHaarClassifierCascade* faceCascade = NULL;


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFaceRecAppDlg dialog




CFaceRecAppDlg::CFaceRecAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFaceRecAppDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_output = _T("");
	m_threshold = 0.75;
}

void CFaceRecAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_OUTPUT, m_output);
	//  DDX_Control(pDX, IDC_EDIT_OUTPUT, m_message);
	//  DDX_Control(pDX, IDC_EDIT_OUTPUT, m_editcontrol);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_editControl);
	//  DDX_Text(pDX, IDC_EDIT_THRES, m_threshold);
	//  DDV_MinMaxFloat(pDX, m_threshold, 0.00, 1.00);
	DDX_Text(pDX, IDC_EDIT_THRES, m_threshold);
	DDV_MinMaxDouble(pDX, m_threshold, 0.00, 1.00);
}

BEGIN_MESSAGE_MAP(CFaceRecAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_EXIT, &CFaceRecAppDlg::OnBnClickedBtnExit)
	ON_BN_CLICKED(IDC_BTN_OPENC, &CFaceRecAppDlg::OnBnClickedBtnOpenc)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_ABOUT, &CFaceRecAppDlg::OnBnClickedBtnAbout)
	ON_BN_CLICKED(IDC_BTN_SIGNUP, &CFaceRecAppDlg::OnBnClickedBtnSignup)
	ON_BN_CLICKED(IDC_BTN_VIEWR, &CFaceRecAppDlg::OnBnClickedBtnViewr)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_THRES, &CFaceRecAppDlg::OnDeltaposSpinThres)
END_MESSAGE_MAP()


// CFaceRecAppDlg message handlers

BOOL CFaceRecAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	GetDlgItem(IDC_BTN_SIGNUP)->EnableWindow(FALSE);

	// Load PCA database
	m_output += _T("loading training data from training database file 'facedata.xml' ... ");
	ShowTextToOutput(m_output);
	if (loadTrainingData())
		m_output += _T("successful\r\n");
	else // loading database failed, disable 'Open Camera' (button) and 'View Records' (button)
	{
		m_output += _T("failed\r\n");
		GetDlgItem(IDC_BTN_OPENC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_VIEWR)->EnableWindow(FALSE);
	}
	ShowTextToOutput(m_output);

	
	m_output += _T("loading face detector ... ");
	ShowTextToOutput(m_output);

	// Load face detector
	const char * faceCascadeFileName = "haarcascade_frontalface_alt2.xml"; // Haar Cascade file, used for Face Detection
	faceCascade = (CvHaarClassifierCascade*)cvLoad(faceCascadeFileName, 0, 0, 0);
	if (!faceCascade)
		m_output += _T("failed\r\n");
	else
		m_output += _T("successful\r\n");
	ShowTextToOutput(m_output);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFaceRecAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFaceRecAppDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFaceRecAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
} 

void CFaceRecAppDlg::OnBnClickedBtnExit()
{
	// TODO: Add your control notification handler code here
	releaseTrainingData(); // PCA
	cvReleaseCapture(&capture);  
	cvReleaseHaarClassifierCascade(&faceCascade);
	CDialog::OnOK();
}


void CFaceRecAppDlg::OnBnClickedBtnOpenc()
{
	// TODO: Add your control notification handler code here
	static int status = 0;

	if (status == 0)
	{
		m_output += _T("accessing camera ... ");
		ShowTextToOutput(m_output);

		// Initialize the camera
		capture = cvCaptureFromCAM(0);

		// Wait a moment
		#if defined WIN32 || defined _WIN32
			Sleep(2000);	// (in milliseconds): sleep 2 sec
		#endif
		//IplImage* frame = cvQueryFrame(capture); // get the first frame, to make sure the camera is initialized.

		// Test if the camera has been initialized
		if (capture == NULL)  
		{  
			m_output += _T("failed\r\n"); 
			ShowTextToOutput(m_output);
			return;  
		}   
		m_output += _T("successful\r\n");
		ShowTextToOutput(m_output);
	
		GetDlgItem(IDC_BTN_OPENC)->SetWindowTextW(_T("Close Camera"));
		GetDlgItem(IDC_BTN_SIGNUP)->EnableWindow(TRUE); // Enable 'Sign Up' (button)
		SetTimer(1, TIMER_INTERVAL, NULL); // Start timer

		status = 1;
	}
	else
	{
		m_output += _T("closing camera ... ");
		ShowTextToOutput(m_output);

		KillTimer(1); // Stop timer
		cvReleaseCapture(&capture);  
		m_output += _T("successful\r\n"); 
		ShowTextToOutput(m_output);
		DrawPicToHDC(NULL, IDC_PICBOX); // Clear picture control
		
		GetDlgItem(IDC_BTN_OPENC)->SetWindowTextW(_T("Open Camera"));
		GetDlgItem(IDC_BTN_SIGNUP)->EnableWindow(FALSE); // Disable 'Sign Up' (button)

		status = 0;
	}
}


void CFaceRecAppDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	IplImage* m_Frame = cvQueryFrame(capture);

	//// Show fps
	//char fpsstr[32];
	//sprintf(fpsstr, "fps : %d", 1000 / TIMER_INTERVAL);
	//cvText(m_Frame, fpsstr, cvPoint(10, 25), 1.0, 1.0, cvScalar(0, 0, 255), 2); 

	if (faceCascade != NULL)
	{	// Detect face in image using face detetcor
		CvRect faceRect = detectFaceInImage(m_Frame, faceCascade, cvSize(20, 20));
		if (faceRect.width > 0) // if there's a face in the image
		{
			// Show the region where the face is
			CvPoint leftTop = cvPoint(faceRect.x, faceRect.y);
			CvPoint rightBot = cvPoint(faceRect.x + faceRect.width, faceRect.y + faceRect.height);
			cvRectangle(m_Frame, leftTop, rightBot, cvScalar(0, 0, 255), 4);
			
			// Get the face image
			if (faceImage != NULL)
				cvReleaseImage(&faceImage);
			faceImage = cropImage(m_Frame, faceRect);
			
			const char* name = "Unknown Person";

			// Code for face recognizition
			float confidence = 0.0f;
			int personID = facerec(faceImage, &confidence); // Default: based on PCA
			if (personID != -1 && confidence >= m_threshold)
				name = persons[personID].getName();

			// Draw name of the person on the picture 'm_Frame'
			char text[256];
			sprintf(text, "%s, %.2f", name, confidence);
			CvPoint pos = cvPoint(faceRect.x, faceRect.y + faceRect.height + 40);
			cvText(m_Frame, text, pos, 1.2, 1.2, cvScalar(255, 0, 0), 2); // defined in basic.cpp, not from Opencv library
		}
	}
	else
		cvReleaseImage(&faceImage);

	// Display video that gets from camera 
	if (m_Frame != NULL)
	{
		// Show image m_Frame in the picture box control
		DrawPicToHDC(m_Frame, IDC_PICBOX);
		//cvReleaseImage(&m_Frame);
	}

	//static int first_time = 1;
	//if (first_time)
	//{
	//	GetDlgItem(IDC_BTN_SIGNUP)->EnableWindow(TRUE);
	//	first_time = 0;
	//}

	CDialogEx::OnTimer(nIDEvent);
}


BOOL CFaceRecAppDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style &= ~WS_THICKFRAME;
	return CDialogEx::PreCreateWindow(cs);
}


void CFaceRecAppDlg::DrawPicToHDC(IplImage* image, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	if (image == NULL) // Clear device
	{
		pDC->FillSolidRect(&rect, RGB(240, 240, 240));
		pDC->DrawEdge(&rect, EDGE_BUMP, BF_RECT);
	}
	else // Show picture in device
	{
		CvvImage cimg;
		cimg.CopyOf(image);
		cimg.DrawToHDC(hDC, &rect);
	}
	ReleaseDC(pDC);
}

void CFaceRecAppDlg::ShowTextToOutput(CString text)
{
	this->SetDlgItemText(IDC_EDIT_OUTPUT, text);
	m_editControl.LineScroll(m_editControl.GetLineCount());
}


void CFaceRecAppDlg::OnBnClickedBtnSignup()
{
	// TODO: Add your control notification handler code here
	
	KillTimer(1);

	m_output += "sign up ... ";
	ShowTextToOutput(m_output);
	
	CMyDialogSignUp dlg; // Sign up dialog
	if (dlg.DoModal() == IDOK) // OK
	{
		m_output += "user : ";
		m_output += persons[persons.size()-1].getName();
		m_output += "\r\n";
	}
	else // Cancel
		m_output += "canceled\r\n";
	ShowTextToOutput(m_output);

	SetTimer(1, TIMER_INTERVAL, NULL);
}

void CFaceRecAppDlg::OnBnClickedBtnViewr()
{
	// TODO: Add your control notification handler code here
	const char* recordsfile1 = "records.html";
	char tempstr[256];
	sprintf(tempstr, "view records in '%s'\r\n", recordsfile1);
	m_output += tempstr;
	ShowTextToOutput(m_output);

	showRecords(recordsfile1); // Show records in a HTML file named (recordsfile1)

	CString recordsfile2 = _T("");
	recordsfile2 += recordsfile1; // Convert const char* to CString
	ShellExecute(NULL, NULL, recordsfile2, NULL, NULL, SW_SHOWNORMAL); // open HTML file
}


void CFaceRecAppDlg::OnBnClickedBtnAbout()
{
	// TODO: Add your control notification handler code here
	//const char* aboutfile = "about.html";
	//CHtml file(aboutfile, "About");
	//file.ShowText("Face Recognition System, version 1.0", 1);
	//file.ShowText("Copyright(C) 2013, Created by Zhang Aimin, 2013.04.20-2013.04.30");
	//file.EndLine();
	//file.ShowText("Development Environment: Microsoft Visual Studio 2010 Ultimate, OpenCV 2.4.2");
	ShellExecute(NULL, NULL, _T("about.html"), NULL, NULL, SW_SHOWNORMAL);
}

void CFaceRecAppDlg::OnDeltaposSpinThres(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	UpdateData(TRUE);
	if (pNMUpDown->iDelta == -1)
		m_threshold += m_threshold < 1.0 ? 0.01 : 0;
	else
		m_threshold -= m_threshold > 0.0 ? 0.01 : 0;
	UpdateData(FALSE);
}


// SJTU_AGV_DectDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SJTU_AGV_Dect.h"
#include "SJTU_AGV_DectDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;
using namespace std;

AGVResult setAGVResult(float x, float y, float a)
{
	AGVResult result;
	result.pt_x = x;
	result.pt_y = y;
	result.angle = a;
	return result;
}

//////////////////////////////////////////////////////
//Simple helper class to set the HeartbeatTimeout safely			
/////////////////////////////////////////////////////
class CHeartbeatHelper
{
public:
	explicit CHeartbeatHelper(CInstantCamera& camera)
		: m_pHeartbeatTimeout(NULL)
	{
		// m_pHeartbeatTimeout may be NULL
		m_pHeartbeatTimeout = camera.GetTLNodeMap().GetNode("HeartbeatTimeout");
	}

	bool SetValue(int64_t NewValue)
	{
		// Do nothing if no heartbeat feature is available.
		if (!m_pHeartbeatTimeout.IsValid())
			return false;

		// Apply the increment and cut off invalid values if neccessary.
		int64_t correctedValue = NewValue - (NewValue % m_pHeartbeatTimeout->GetInc());

		m_pHeartbeatTimeout->SetValue(correctedValue);
		return true;
	}

	bool SetMax()
	{
		// Do nothing if no heartbeat feature is available.
		if (!m_pHeartbeatTimeout.IsValid())
			return false;

		int64_t maxVal = m_pHeartbeatTimeout->GetMax();
		return SetValue(maxVal);
	}

protected:
	GenApi::CIntegerPtr m_pHeartbeatTimeout; // Pointer to the node, will be NULL if no node exists.
};

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CSJTU_AGV_DectDlg �Ի���



CSJTU_AGV_DectDlg::CSJTU_AGV_DectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSJTU_AGV_DectDlg::IDD, pParent)
	, m_cnt(1)
	, m_open_cam_flag(false)
{
	m_server = HP_Create_TcpServer(this);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSJTU_AGV_DectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RESULT, m_result_list);
	DDX_Control(pDX, IDC_COMBO_POINT, m_combox_select);
}

BEGIN_MESSAGE_MAP(CSJTU_AGV_DectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SETCAP, &CSJTU_AGV_DectDlg::OnBnClickedButtonSetcap)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSJTU_AGV_DectDlg ��Ϣ�������

BOOL CSJTU_AGV_DectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	__init__();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSJTU_AGV_DectDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSJTU_AGV_DectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSJTU_AGV_DectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//�궨
void CSJTU_AGV_DectDlg::OnBnClickedButtonSetcap()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//m_src = imread(_TEST_IMG_FILE_);
	int cursel = m_combox_select.GetCurSel();
	if (m_open_cam_flag)
	{
		CGrabResultPtr ptrPipeGrabResult;
		if (!TriggerCameraAndGrabImage(m_Camera, ptrPipeGrabResult, m_src))
		{
			AfxMessageBox(L"�ɼ�ʧ��");
			m_log.INFO_LOG("�ɼ�ʧ��");
			return;
		}
		int rc = 0;
		if (cursel == 5)
			rc = DectMain(m_src, _DECT_THRE_5, _DECT_MIN_AREA_, _DECT_MAX_AREA_, _DECT_MIN_SUB_, _DECT_MAX_SUB_);
		else if (cursel == 1)
			rc = DectMain(m_src, _DECT_THRE_1, _DECT_MIN_AREA_, _DECT_MAX_AREA_, _DECT_MIN_SUB_, _DECT_MAX_SUB_);
		else
			rc = DectMain(m_src, _DECT_THRE_, _DECT_MIN_AREA_, _DECT_MAX_AREA_, _DECT_MIN_SUB_, _DECT_MAX_SUB_);
		if (rc == 0)
		{
			ShowResult(m_result);
			int cursel = m_combox_select.GetCurSel();
			m_parames[cursel] = m_result;
			if (SaveParam(_DEMARCATE_FILE_) != 0)
			{
				m_log.ERROR_LOG("����궨�ļ�ʧ�ܣ�");
				return;
			}
			m_log.INFO_LOG("�궨���");
			SetDlgItemText(IDC_TEXT_RESULT, L"��λ���");
		}
		else
		{
			SetDlgItemText(IDC_TEXT_RESULT, L"��λ�쳣");
			m_log.INFO_LOG("�궨�쳣");
		}
	}
	else
	{
		AfxMessageBox(L"���ȴ����");
		m_log.INFO_LOG("���ȴ����");
	}
}

void CSJTU_AGV_DectDlg::__init__()
{
	color_img_flag = false;
	ShowWindow(SW_MAXIMIZE);
	SetImgWindow(IDC_SHOW_IMG, _WIN_NAME_);
	m_log.GLOG_start_(_INFO_FILE_, _ERROR_FILE_);
	//��ô��ڳߴ�
	GetClientRect(&m_wndRect);

	PylonInitialize();
	//������ɫ������
	m_greenBrush.CreateSolidBrush(RGB(0, 255, 0));
	m_redBrush.CreateSolidBrush(RGB(255, 0, 0));
	m_grayBrush.CreateSolidBrush(RGB(0x80, 0x80, 0x80));

	m_bigFont.CreatePointFont(200, L"΢���ź�");
	GetDlgItem(IDC_TEXT_RESULT)->SetFont(&m_bigFont);

	m_combox_select.AddString(L"��λһ");
	m_combox_select.AddString(L"��λ��");
	m_combox_select.AddString(L"��λ��");
	m_combox_select.AddString(L"��λ��");
	m_combox_select.AddString(L"��λ��");
	/*m_combox_select.AddString(L"��λ��");
	m_combox_select.AddString(L"��λ��");
	m_combox_select.AddString(L"��λ��");
	m_combox_select.AddString(L"��λ��");
	m_combox_select.AddString(L"��λʮ");
	m_combox_select.AddString(L"��λʮһ");
	m_combox_select.AddString(L"��λʮ��");
	m_combox_select.AddString(L"��λʮ��");*/
	m_combox_select.SetCurSel(0);

	m_src = Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);
	m_log.INFO_LOG("-----����ʼ-----__init__");
	if (!m_server->Start(_TCP_IP_ADR_, _TCP_IP_PORT_))
	{
		AfxMessageBox(L"����������ʧ�ܣ�");
		m_log.ERROR_LOG("����������ʧ�ܣ�");
		return;
	}

	if (LoadParam(_DEMARCATE_FILE_) != 0)
	{
		AfxMessageBox(L"���ر궨�ļ�ʧ�ܣ�");
		m_log.ERROR_LOG("���ر궨�ļ�ʧ�ܣ�");
		return;
	}
	if (openCamera() != 0)
	{
		AfxMessageBox(L"�����ʧ�ܣ�");
		m_log.ERROR_LOG("�����ʧ�ܣ�");
		return;
	}
}

//��ʼ���ã�ȷ�����ڿؼ��ʹ�������
void CSJTU_AGV_DectDlg::SetImgWindow(int Dlgitem, const char* str)
{
	namedWindow(str, WINDOW_AUTOSIZE);
	HWND hWnd = (HWND)cvGetWindowHandle(str);
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(Dlgitem)->m_hWnd);  //picture�ؼ�
	::ShowWindow(hParent, SW_HIDE);
}

//��ʾͼ��
void CSJTU_AGV_DectDlg::ShowMatImg(cv::Mat src, int Dlgitem, const char* str)
{
	if (src.empty())
	{
		return;
	}
	cv::Mat bgr;
	if (src.channels() == 3)
		bgr = src.clone();
	else if (src.channels() == 1)
		cvtColor(src, bgr, CV_GRAY2BGR);
	else if (src.channels() == 4)
		cvtColor(src, bgr, CV_BGRA2BGR);
	else
		return;
	cv::Mat temp;
	CRect m_rect;
	GetDlgItem(Dlgitem)->GetClientRect(&m_rect);
	CvSize window_size = cvSize(m_rect.Width(), m_rect.Height());
	double width = window_size.width;
	double scale = (double)src.rows / (double)src.cols;
	double height = width*scale;
	if (width > window_size.width || height > window_size.height)
	{
		height = window_size.height;
		scale = (double)src.cols / (double)src.rows;
		width = height*scale;
		if (width > window_size.width || height > window_size.height)
		{
			return;
		}
	}
	cv::resize(bgr, temp, cvSize((int)width, (int)height));
	cv::Mat dst = cv::Mat::Mat(window_size, CV_8UC3, cvScalarAll(0xff));
	int x = (dst.cols - temp.cols) / 2;
	int y = (dst.rows - temp.rows) / 2;
	cv::Mat roi = dst(cvRect(x, y, temp.cols, temp.rows));
	temp.copyTo(roi);
	cv::imshow(str, dst);
}

void CSJTU_AGV_DectDlg::OnSize(UINT nType, int cx, int cy)
{
	// TODO:  �ڴ˴������Ϣ����������
	for (int i = 1000; i < 1040; i++)//��Ϊ�Ƕ���ؼ���������������ѭ��
	{
		CWnd *pWnd = GetDlgItem(i);
		if (pWnd && nType != 1 && m_wndRect.Width() && m_wndRect.Height())  //�ж��Ƿ�Ϊ�գ���Ϊ�Ի��򴴽�ʱ����ô˺���������ʱ�ؼ���δ����
		{
			CRect rect;   //��ȡ�ؼ��仯ǰ�Ĵ�С 
			pWnd->GetWindowRect(&rect);
			ScreenToClient(&rect);//���ؼ���Сת��Ϊ�ڶԻ����е���������
			rect.left = rect.left*cx / m_wndRect.Width();//�����ؼ���С
			rect.right = rect.right*cx / m_wndRect.Width();
			rect.top = rect.top*cy / m_wndRect.Height();
			rect.bottom = rect.bottom*cy / m_wndRect.Height();
			pWnd->MoveWindow(rect);//���ÿؼ���С 
		}
	}
	//�ı�ߴ�ʱ����listCtrl���п�
	if (m_result_list.m_hWnd)
	{
		CRect listRect;
		m_result_list.GetClientRect(&listRect);
		m_result_list.SetExtendedStyle(m_result_list.GetExtendedStyle() | LVS_EX_GRIDLINES
			| LVS_EX_FULLROWSELECT);
		m_result_list.InsertColumn(0, L"���", LVCFMT_CENTER, listRect.Width() / 5);//ʹctrlListֻ��һ��
		m_result_list.InsertColumn(1, L"X����ƫ��", LVCFMT_CENTER, listRect.Width() / 4);//ʹctrlListֻ��һ��
		m_result_list.InsertColumn(2, L"Y����ƫ��", LVCFMT_CENTER, listRect.Width() / 4);//ʹctrlListֻ��һ��
		m_result_list.InsertColumn(3, L"ƫת�Ƕ�", LVCFMT_CENTER, listRect.Width() - 2 * listRect.Width() / 4 - listRect.Width() / 5);//ʹctrlListֻ��һ��
	}
	//���»�ô��ڳߴ�
	GetClientRect(&m_wndRect);
}


void CSJTU_AGV_DectDlg::OnOK()
{
	// TODO:  �ڴ����ר�ô����/����û���
	m_server->Stop();
	if (m_open_cam_flag)
		closeCamera();
	PylonTerminate();
	m_log.INFO_LOG("-----��������-----OnOK\n");
	m_log.GLOG_end_();

	CDialogEx::OnOK();
}


void CSJTU_AGV_DectDlg::OnCancel()
{
	// TODO:  �ڴ����ר�ô����/����û���
	m_server->Stop();
	if (m_open_cam_flag)
		closeCamera();
	PylonTerminate();
	m_log.INFO_LOG("-----��������-----OnCancel\n");
	m_log.GLOG_end_();

	CDialogEx::OnCancel();
}


HBRUSH CSJTU_AGV_DectDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	CString str;
	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_TEXT_RESULT:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 255));
		GetDlgItemText(IDC_TEXT_RESULT, str);
		if (str == _T("��λ���"))
		{
			return m_greenBrush;
		}
		else if (str == _T("��λ�쳣"))
		{
			return m_redBrush;
		}
		else
			return m_grayBrush;
		break;
	}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

void CSJTU_AGV_DectDlg::DrawBox(CvBox2D box, cv::Mat &dst, cv::Scalar color)
{
	CvPoint2D32f point[4];
	int i;
	for (i = 0; i < 4; i++)

	{
		point[i].x = 0;
		point[i].y = 0;

	}
	cvBoxPoints(box, point); //�����ά���Ӷ���
	vector<Point> pt;
	vector<vector<Point>> ppt;
	for (i = 0; i < 4; i++)
	{
		pt.push_back(point[i]);
	}
	ppt.push_back(pt);
	polylines(dst, ppt, true, color, 3);
	m_log.INFO_LOG("DrawBox Done");
	return;
}

int CSJTU_AGV_DectDlg::DectMain(cv::Mat src, int thre, double min_area, 
	double max_area, float min_whsub, float max_whsub)
{
	if (src.empty())
	{
		m_log.ERROR_LOG("����Ϊ�գ�");
		return -1;
	}
	Mat gray, dst;
	if (src.channels() == 3)
	{
		cvtColor(src, gray, CV_BGR2GRAY);
		dst = src.clone();
	}
	else if (src.channels() == 1)
	{
		cvtColor(src, dst, CV_GRAY2BGR);
		gray = src.clone();
	}
	else
	{
		m_log.ERROR_LOG("ͼ���ʽ���ԣ�");
		return -2;
	}
	threshold(gray, gray, thre, 0xff, CV_THRESH_BINARY);
	//imwrite("111.png", gray);
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(gray.clone(), contours, CV_RETR_LIST,
		CV_CHAIN_APPROX_NONE);//ÿ��������ȫ������
	RotatedRect rect;
	Scalar red(0, 0, 255);
	int sum = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);
		if (area>10000.0)
			TRACE(L"%.2f", area);
		if (area >= min_area&&area <= max_area)
		{
			sprintf_s(vbuf, "%.2f���������", area);
			m_log.INFO_LOG(vbuf);
			rect = cv::minAreaRect(contours[i]);
			float sub = fabs(rect.size.height - rect.size.width);
			if (sub >= min_whsub&&sub <= max_whsub)
			{
				sprintf_s(vbuf, "����� = %.2f", sub);
				m_log.INFO_LOG(vbuf);
				sprintf_s(vbuf, "��ת���δ�С(%.2f,%.2f)������(%.2f,%.2f)���Ƕ� = %.2f",
					rect.size.width, rect.size.height, rect.center.x, rect.center.y, rect.angle);
				m_log.INFO_LOG(vbuf);
				m_result.pt_x = rect.center.x;
				m_result.pt_y = rect.center.y;
				m_result.angle = rect.angle;
				if (rect.size.width < rect.size.height)
					m_result.angle += 90;
				DrawBox(rect, dst, red);
				circle(dst, rect.center, 4, red, CV_FILLED);
				sum++;
			}
			else
			{
				sprintf_s(vbuf, "���ǳ����%.2f����", sub);
				m_log.INFO_LOG(vbuf);
			}
		}
	}
	ShowMatImg(dst, IDC_SHOW_IMG, _WIN_NAME_);
	if (sum != 1)
	{
		sprintf_s(vbuf, "�������������ϣ�%d", sum);
		m_log.INFO_LOG(vbuf);
		return -9;
	}
	return 0;
}

int CSJTU_AGV_DectDlg::openCamera()
{
	try
	{
		CDeviceInfo di;
		di.SetSerialNumber(_CAMERA_SN_);
		di.SetDeviceClass(BaslerGigEDeviceClass);
		m_Camera.Attach(CTlFactory::GetInstance().CreateDevice(di));
		m_Camera.RegisterConfiguration(new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);

		CHeartbeatHelper heartbeatHelper(m_Camera);
		heartbeatHelper.SetValue(30000);

		m_Camera.Open();

		//���ز�������
		m_Camera.UserSetSelector.SetValue(UserSetSelector_UserSet1);
		m_Camera.UserSetLoad.Execute();
		m_Camera.ExposureTimeAbs = 2000;

		//��������
		m_Camera.StartGrabbing(INFINITE, GrabStrategy_LatestImageOnly, GrabLoop_ProvidedByUser);
		m_log.INFO_LOG("������ɹ�");
		m_open_cam_flag = true;
	}
	catch (const GenericException &e)
	{
		MessageBox(CString(e.GetDescription()));
		m_log.ERROR_LOG(e.GetDescription());
		return -1;
	}
	return 0;
}

bool CSJTU_AGV_DectDlg::TriggerCameraAndGrabImage(CBaslerGigEInstantCamera& cameraObject, CGrabResultPtr& ptrGrabResult, Mat& srcImg)
{
	try
	{
		unsigned int waittime = 5000;
		if (cameraObject.CanWaitForFrameTriggerReady())
		{
			// Execute the software trigger. Wait up to 5000 ms for the camera to be ready for trigger.
			if (cameraObject.WaitForFrameTriggerReady(waittime, TimeoutHandling_ThrowException))
			{
				cameraObject.ExecuteSoftwareTrigger();
				cameraObject.RetrieveResult(waittime, ptrGrabResult, TimeoutHandling_ThrowException);
				if (ptrGrabResult->GrabSucceeded())
				{
					int col = ptrGrabResult->GetWidth();
					int row = ptrGrabResult->GetHeight();
					if (color_img_flag)
					{
						Mat src = Mat::zeros(row, col, CV_8UC3);
						memcpy((uint8_t *)srcImg.datastart, (uint8_t *)ptrGrabResult->GetBuffer(), ptrGrabResult->GetImageSize());
						srcImg = src.clone();
						m_to.Bayer8ToRgb24(src.data, src.cols, src.rows, srcImg.data, src.step*src.rows);
						sprintf_s(vbuf, "��ȡͼ��ɹ�(%d * %d) RGB", col, row);
					}
					else
					{
						srcImg = Mat::zeros(row, col, CV_8UC1);
						memcpy((uint8_t *)srcImg.datastart, (uint8_t *)ptrGrabResult->GetBuffer(), ptrGrabResult->GetImageSize());
						sprintf_s(vbuf, "��ȡͼ��ɹ�(%d * %d) GRAY", col, row);
					}
					m_log.INFO_LOG(vbuf);
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	catch (const GenericException &e)
	{
		AfxMessageBox(CString(e.GetDescription()));
		return false;
	}


	return true;
}

void CSJTU_AGV_DectDlg::closeCamera()
{
	m_Camera.DestroyDevice();
	m_log.INFO_LOG("�ر����");
	m_open_cam_flag = false;
}

void CSJTU_AGV_DectDlg::ShowResult(AGVResult result)
{
	cstr.Format(L"%d", m_cnt);
	int intm = m_result_list.InsertItem(0, cstr);
	cstr.Format(L"%.2f", result.pt_x);
	m_result_list.SetItemText(intm, 1, cstr);
	cstr.Format(L"%.2f", result.pt_y);
	m_result_list.SetItemText(intm, 2, cstr);
	cstr.Format(L"%.2f", result.angle);
	m_result_list.SetItemText(intm, 3, cstr);
	m_cnt++;
}

void CSJTU_AGV_DectDlg::ShowResult(AGVResult result, int cursel)
{
	AGVResult sub;
	sub.pt_x = -(m_parames[cursel].pt_x - result.pt_x)*_AGV_SCALE_;
	sub.pt_y = -(m_parames[cursel].pt_y - result.pt_y)*_AGV_SCALE_;
	sub.angle = -m_parames[cursel].angle + result.angle;
	char msg[30];
	sprintf_s(msg, "%.2f,%.2f,%.2f", sub.pt_x, sub.pt_y, sub.angle);
	m_server->Send(m_dwConnid, (BYTE*)msg, 30);
	cstr.Format(L"%d", m_cnt);
	int intm = m_result_list.InsertItem(0, cstr);
	cstr.Format(L"%.2f", sub.pt_x);
	m_result_list.SetItemText(intm, 1, cstr);
	cstr.Format(L"%.2f", sub.pt_y);
	m_result_list.SetItemText(intm, 2, cstr);
	cstr.Format(L"%.2f", sub.angle);
	m_result_list.SetItemText(intm, 3, cstr);
	m_cnt++;
}

EnHandleResult CSJTU_AGV_DectDlg::OnPrepareListen(ITcpServer * pSender, SOCKET soListen)
{//��ʼ����ʱ����
	m_log.INFO_LOG("��ʼ����");
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnAccept(ITcpServer * pSender, CONNID dwConnID, SOCKET soClient)
{//�ͻ�������ʱ����
	m_dwConnid = dwConnID;
	m_log.INFO_LOG("�ͻ�������");
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnSend(ITcpServer * pSender, CONNID dwConnID, const BYTE * pData, int iLength)
{//������Ϣʱ����
	sprintf_s(vbuf, "������Ϣ  %s", (char*)pData);
	m_log.INFO_LOG(vbuf);
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnReceive(ITcpServer * pSender, CONNID dwConnID, const BYTE * pData, int iLength)
{//���ܵ���Ϣʱ����
	char* pl = new char[iLength+1];
	for (int i = 0; i < iLength; i++)
		pl[i] = pData[i];
	pl[iLength] = '\0';
	sprintf_s(vbuf, "���ܵ���Ϣ  %s", pl);
	m_log.INFO_LOG(vbuf);
	if (pl != NULL)
		free(pl);
	if (pData[0] == '1'&&iLength == 3)
	{
		int cursel = pData[2] - '1';
		if (pData[1] == '1')
			cursel += 10;
		if (cursel >= m_parames.size() || pData[1]<'0' || pData[1]>'9' || pData[2]<'0' || pData[2]>'9')
		{
			SetDlgItemText(IDC_TEXT_RESULT, L"��λ�쳣");
			m_log.INFO_LOG("��λ�쳣");
			m_server->Send(m_dwConnid, (BYTE*)"NG\0", 3);
			return HR_OK;
		}
		CGrabResultPtr ptrPipeGrabResult;
		if (!TriggerCameraAndGrabImage(m_Camera, ptrPipeGrabResult, m_src))
		{
			AfxMessageBox(L"�ɼ�ʧ��");
			m_log.INFO_LOG("�ɼ�ʧ��");
			SetDlgItemText(IDC_TEXT_RESULT, L"��λ�쳣");
			m_log.INFO_LOG("��λ�쳣");
			m_server->Send(m_dwConnid, (BYTE*)"NG\0", 3);
			return HR_OK;
		}
		int rc = 0;
		if (cursel == 5)
			rc = DectMain(m_src, _DECT_THRE_5, _DECT_MIN_AREA_, _DECT_MAX_AREA_, _DECT_MIN_SUB_, _DECT_MAX_SUB_);
		else
			rc = DectMain(m_src, _DECT_THRE_, _DECT_MIN_AREA_, _DECT_MAX_AREA_, _DECT_MIN_SUB_, _DECT_MAX_SUB_);
		if (rc == 0)
		{
			ShowResult(m_result, cursel);
			m_log.INFO_LOG("��λ���");
			SetDlgItemText(IDC_TEXT_RESULT, L"��λ���");
		}
		else
		{
			SetDlgItemText(IDC_TEXT_RESULT, L"��λ�쳣");
			m_log.INFO_LOG("��λ�쳣");
			//m_server->Send(m_dwConnid, (BYTE*)"NG\0", 3);
		}
	}
	else
	{
		SetDlgItemText(IDC_TEXT_RESULT, L"��λ�쳣");
		m_log.INFO_LOG("��λ�쳣");
		m_server->Send(m_dwConnid, (BYTE*)"NG\0", 3);
	}
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnClose(ITcpServer * pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{//�ͻ��˶Ͽ�ʱ����
	m_log.INFO_LOG("�ͻ��˶Ͽ�");
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnShutdown(ITcpServer * pSender)
{//�رռ���ʱ����
	m_log.INFO_LOG("�رռ���");
	return HR_OK;
}

int CSJTU_AGV_DectDlg::SaveParam(const char* filename)
{
	FileStorage fs(filename, FileStorage::WRITE);
	if (!fs.isOpened())
	{
		return -1;
	}
	int sz = m_parames.size();
	string str = "DEMARCATE_SIZE";
	fs << str << sz;
	char filenode[20];
	for (int i = 0; i < sz; i++)
	{
		if (i < 10)
			sprintf_s(filenode, "DEMARCATE_0%d", i);
		else
			sprintf_s(filenode, "DEMARCATE_%d", i);
		fs << filenode << "{" << "PT_X" << m_parames[i].pt_x;
		fs << "PT_Y" << m_parames[i].pt_y;
		fs << "ANGLE" << m_parames[i].angle << "}";
	}
	fs.release();
	return 0;
}

int CSJTU_AGV_DectDlg::LoadParam(const char* filename)
{
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		return -1;
	}
	m_parames.clear();
	string str = "DEMARCATE_SIZE";
	int sz = fs[str];
	char filenode[20];
	for (int i = 0; i < sz; i++)
	{
		if (i < 10)
			sprintf_s(filenode, "DEMARCATE_0%d", i);
		else
			sprintf_s(filenode, "DEMARCATE_%d", i);
		FileNode node = fs[filenode];
		AGVResult temp;
		temp.pt_x = node["PT_X"];
		temp.pt_y = node["PT_Y"];
		temp.angle = node["ANGLE"];
		m_parames.push_back(temp);
	}
	fs.release();
	return 0;
}
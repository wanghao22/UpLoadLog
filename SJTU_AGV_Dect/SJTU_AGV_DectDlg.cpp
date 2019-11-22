
// SJTU_AGV_DectDlg.cpp : 实现文件
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

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CSJTU_AGV_DectDlg 对话框



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


// CSJTU_AGV_DectDlg 消息处理程序

BOOL CSJTU_AGV_DectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	__init__();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSJTU_AGV_DectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSJTU_AGV_DectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//标定
void CSJTU_AGV_DectDlg::OnBnClickedButtonSetcap()
{
	// TODO:  在此添加控件通知处理程序代码
	//m_src = imread(_TEST_IMG_FILE_);
	int cursel = m_combox_select.GetCurSel();
	if (m_open_cam_flag)
	{
		CGrabResultPtr ptrPipeGrabResult;
		if (!TriggerCameraAndGrabImage(m_Camera, ptrPipeGrabResult, m_src))
		{
			AfxMessageBox(L"采集失败");
			m_log.INFO_LOG("采集失败");
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
				m_log.ERROR_LOG("保存标定文件失败！");
				return;
			}
			m_log.INFO_LOG("标定完成");
			SetDlgItemText(IDC_TEXT_RESULT, L"定位完成");
		}
		else
		{
			SetDlgItemText(IDC_TEXT_RESULT, L"定位异常");
			m_log.INFO_LOG("标定异常");
		}
	}
	else
	{
		AfxMessageBox(L"请先打开相机");
		m_log.INFO_LOG("请先打开相机");
	}
}

void CSJTU_AGV_DectDlg::__init__()
{
	color_img_flag = false;
	ShowWindow(SW_MAXIMIZE);
	SetImgWindow(IDC_SHOW_IMG, _WIN_NAME_);

	char info_file[30], error_file[30];
	SYSTEMTIME time;
	GetLocalTime(&time);
	if (time.wMonth < 10)
	{
		sprintf_s(info_file, "../info_%4d_0%d.log", time.wYear, time.wMonth);
		sprintf_s(error_file, "../error_%4d_0%d.log", time.wYear, time.wMonth);
	}
	else
	{
		sprintf_s(info_file, "../info_%4d_%d.log", time.wYear, time.wMonth);
		sprintf_s(error_file, "../error_%4d_%d.log", time.wYear, time.wMonth);
	}

	m_log.GLOG_start_(info_file, error_file);
	//获得窗口尺寸
	GetClientRect(&m_wndRect);

	PylonInitialize();
	//设置颜色和字体
	m_greenBrush.CreateSolidBrush(RGB(0, 255, 0));
	m_redBrush.CreateSolidBrush(RGB(255, 0, 0));
	m_grayBrush.CreateSolidBrush(RGB(0x80, 0x80, 0x80));

	m_bigFont.CreatePointFont(200, L"微软雅黑");
	GetDlgItem(IDC_TEXT_RESULT)->SetFont(&m_bigFont);

	m_combox_select.AddString(L"工位一");
	m_combox_select.AddString(L"工位二");
	m_combox_select.AddString(L"工位三");
	m_combox_select.AddString(L"工位四");
	m_combox_select.AddString(L"工位五");
	/*m_combox_select.AddString(L"工位六");
	m_combox_select.AddString(L"工位七");
	m_combox_select.AddString(L"工位八");
	m_combox_select.AddString(L"工位九");
	m_combox_select.AddString(L"工位十");
	m_combox_select.AddString(L"工位十一");
	m_combox_select.AddString(L"工位十二");
	m_combox_select.AddString(L"工位十三");*/
	m_combox_select.SetCurSel(0);

	m_src = Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);
	m_log.INFO_LOG("-----程序开始-----__init__");
	if (!m_server->Start(_TCP_IP_ADR_, _TCP_IP_PORT_))
	{
		AfxMessageBox(L"开启服务器失败！");
		m_log.ERROR_LOG("开启服务器失败！");
		return;
	}

	if (LoadParam(_DEMARCATE_FILE_) != 0)
	{
		AfxMessageBox(L"加载标定文件失败！");
		m_log.ERROR_LOG("加载标定文件失败！");
		return;
	}
	if (openCamera() != 0)
	{
		AfxMessageBox(L"打开相机失败！");
		m_log.ERROR_LOG("打开相机失败！");
		return;
	}
}

//初始化用，确定窗口控件和窗口名称
void CSJTU_AGV_DectDlg::SetImgWindow(int Dlgitem, const char* str)
{
	namedWindow(str, WINDOW_AUTOSIZE);
	HWND hWnd = (HWND)cvGetWindowHandle(str);
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(Dlgitem)->m_hWnd);  //picture控件
	::ShowWindow(hParent, SW_HIDE);
}

//显示图像
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
	// TODO:  在此处添加消息处理程序代码
	for (int i = 1000; i < 1040; i++)//因为是多个控件，所以这里用了循环
	{
		CWnd *pWnd = GetDlgItem(i);
		if (pWnd && nType != 1 && m_wndRect.Width() && m_wndRect.Height())  //判断是否为空，因为对话框创建时会调用此函数，而当时控件还未创建
		{
			CRect rect;   //获取控件变化前的大小 
			pWnd->GetWindowRect(&rect);
			ScreenToClient(&rect);//将控件大小转换为在对话框中的区域坐标
			rect.left = rect.left*cx / m_wndRect.Width();//调整控件大小
			rect.right = rect.right*cx / m_wndRect.Width();
			rect.top = rect.top*cy / m_wndRect.Height();
			rect.bottom = rect.bottom*cy / m_wndRect.Height();
			pWnd->MoveWindow(rect);//设置控件大小 
		}
	}
	//改变尺寸时设置listCtrl的列宽
	if (m_result_list.m_hWnd)
	{
		CRect listRect;
		m_result_list.GetClientRect(&listRect);
		m_result_list.SetExtendedStyle(m_result_list.GetExtendedStyle() | LVS_EX_GRIDLINES
			| LVS_EX_FULLROWSELECT);
		m_result_list.InsertColumn(0, L"序号", LVCFMT_CENTER, listRect.Width() / 5);//使ctrlList只有一列
		m_result_list.InsertColumn(1, L"X方向偏差", LVCFMT_CENTER, listRect.Width() / 4);//使ctrlList只有一列
		m_result_list.InsertColumn(2, L"Y方向偏差", LVCFMT_CENTER, listRect.Width() / 4);//使ctrlList只有一列
		m_result_list.InsertColumn(3, L"偏转角度", LVCFMT_CENTER, listRect.Width() - 2 * listRect.Width() / 4 - listRect.Width() / 5);//使ctrlList只有一列
	}
	//重新获得窗口尺寸
	GetClientRect(&m_wndRect);
}


void CSJTU_AGV_DectDlg::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类
	m_server->Stop();
	if (m_open_cam_flag)
		closeCamera();
	PylonTerminate();
	m_log.INFO_LOG("-----结束程序-----OnOK\n");
	m_log.GLOG_end_();

	CDialogEx::OnOK();
}


void CSJTU_AGV_DectDlg::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类
	m_server->Stop();
	if (m_open_cam_flag)
		closeCamera();
	PylonTerminate();
	m_log.INFO_LOG("-----结束程序-----OnCancel\n");
	m_log.GLOG_end_();
	if (checkIsNetwork())
		system(_GITHUB_UPLOAD_);

	CDialogEx::OnCancel();
}


HBRUSH CSJTU_AGV_DectDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	CString str;
	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_TEXT_RESULT:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 255));
		GetDlgItemText(IDC_TEXT_RESULT, str);
		if (str == _T("定位完成"))
		{
			return m_greenBrush;
		}
		else if (str == _T("定位异常"))
		{
			return m_redBrush;
		}
		else
			return m_grayBrush;
		break;
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
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
	cvBoxPoints(box, point); //计算二维盒子顶点
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
		m_log.ERROR_LOG("输入为空！");
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
		m_log.ERROR_LOG("图像格式不对！");
		return -2;
	}
	threshold(gray, gray, thre, 0xff, CV_THRESH_BINARY);
	//imwrite("111.png", gray);
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(gray.clone(), contours, CV_RETR_LIST,
		CV_CHAIN_APPROX_NONE);//每个轮廓的全部像素
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
			sprintf_s(vbuf, "%.2f的面积可以", area);
			m_log.INFO_LOG(vbuf);
			rect = cv::minAreaRect(contours[i]);
			float sub = fabs(rect.size.height - rect.size.width);
			if (sub >= min_whsub&&sub <= max_whsub)
			{
				sprintf_s(vbuf, "长宽差 = %.2f", sub);
				m_log.INFO_LOG(vbuf);
				sprintf_s(vbuf, "旋转矩形大小(%.2f,%.2f)，中心(%.2f,%.2f)，角度 = %.2f",
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
				sprintf_s(vbuf, "但是长宽差%.2f不行", sub);
				m_log.INFO_LOG(vbuf);
			}
		}
	}
	ShowMatImg(dst, IDC_SHOW_IMG, _WIN_NAME_);
	if (sum != 1)
	{
		sprintf_s(vbuf, "轮廓个数不符合：%d", sum);
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

		//下载参数设置
		m_Camera.UserSetSelector.SetValue(UserSetSelector_UserSet1);
		m_Camera.UserSetLoad.Execute();
		m_Camera.ExposureTimeAbs = 2000;

		//启动捕获
		m_Camera.StartGrabbing(INFINITE, GrabStrategy_LatestImageOnly, GrabLoop_ProvidedByUser);
		m_log.INFO_LOG("打开相机成功");
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
						sprintf_s(vbuf, "获取图像成功(%d * %d) RGB", col, row);
					}
					else
					{
						srcImg = Mat::zeros(row, col, CV_8UC1);
						memcpy((uint8_t *)srcImg.datastart, (uint8_t *)ptrGrabResult->GetBuffer(), ptrGrabResult->GetImageSize());
						sprintf_s(vbuf, "获取图像成功(%d * %d) GRAY", col, row);
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
	m_log.INFO_LOG("关闭相机");
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
{//开始监听时调用
	m_log.INFO_LOG("开始监听");
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnAccept(ITcpServer * pSender, CONNID dwConnID, SOCKET soClient)
{//客户端连上时调用
	m_dwConnid = dwConnID;
	m_log.INFO_LOG("客户端连上");
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnSend(ITcpServer * pSender, CONNID dwConnID, const BYTE * pData, int iLength)
{//发送消息时调用
	sprintf_s(vbuf, "发送消息  %s", (char*)pData);
	m_log.INFO_LOG(vbuf);
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnReceive(ITcpServer * pSender, CONNID dwConnID, const BYTE * pData, int iLength)
{//接受到消息时调用
	char* pl = new char[iLength+1];
	for (int i = 0; i < iLength; i++)
		pl[i] = pData[i];
	pl[iLength] = '\0';
	sprintf_s(vbuf, "接受到消息  %s", pl);
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
			SetDlgItemText(IDC_TEXT_RESULT, L"定位异常");
			m_log.INFO_LOG("定位异常");
			m_server->Send(m_dwConnid, (BYTE*)"NG\0", 3);
			return HR_OK;
		}
		CGrabResultPtr ptrPipeGrabResult;
		if (!TriggerCameraAndGrabImage(m_Camera, ptrPipeGrabResult, m_src))
		{
			AfxMessageBox(L"采集失败");
			m_log.INFO_LOG("采集失败");
			SetDlgItemText(IDC_TEXT_RESULT, L"定位异常");
			m_log.INFO_LOG("定位异常");
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
			m_log.INFO_LOG("定位完成");
			SetDlgItemText(IDC_TEXT_RESULT, L"定位完成");
		}
		else
		{
			SetDlgItemText(IDC_TEXT_RESULT, L"定位异常");
			m_log.INFO_LOG("定位异常");
			//m_server->Send(m_dwConnid, (BYTE*)"NG\0", 3);
		}
	}
	else
	{
		SetDlgItemText(IDC_TEXT_RESULT, L"定位异常");
		m_log.INFO_LOG("定位异常");
		m_server->Send(m_dwConnid, (BYTE*)"NG\0", 3);
	}
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnClose(ITcpServer * pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{//客户端断开时调用
	m_log.INFO_LOG("客户端断开");
	return HR_OK;
}

EnHandleResult CSJTU_AGV_DectDlg::OnShutdown(ITcpServer * pSender)
{//关闭监听时调用
	m_log.INFO_LOG("关闭监听");
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

bool CSJTU_AGV_DectDlg::checkIsNetwork()
{
	CoInitialize(NULL);
	//  通过NLA接口获取网络状态
	IUnknown *pUnknown = NULL;
	BOOL bOnline = TRUE;//是否在线  
	HRESULT Result = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL, IID_IUnknown, (void **)&pUnknown);
	if (SUCCEEDED(Result))
	{
		INetworkListManager *pNetworkListManager = NULL;
		if (pUnknown)
			Result = pUnknown->QueryInterface(IID_INetworkListManager, (void **)&pNetworkListManager);
		if (SUCCEEDED(Result))
		{
			VARIANT_BOOL IsConnect = VARIANT_FALSE;
			if (pNetworkListManager)
				Result = pNetworkListManager->get_IsConnectedToInternet(&IsConnect);
			if (SUCCEEDED(Result))
			{
				bOnline = (IsConnect == VARIANT_TRUE) ? true : false;
			}


		}
		if (pNetworkListManager)
			pNetworkListManager->Release();
	}
	if (pUnknown)
		pUnknown->Release();
	CoUninitialize();
	return bOnline;
}
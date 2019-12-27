
// SJTU_AGV_DectDlg.h : 头文件
//

#pragma once
#include "allDefine.h"
#include <GLOG.h>
#include "afxcmn.h"
#include "afxwin.h"
#include <vector>
#include <opencv2\opencv.hpp>
#include "pylon/PylonIncludes.h"
#include "pylon/gige/PylonGigEIncludes.h"
#include <pylon/PylonGUI.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <HPSocket.h>
#include <Bayer2BGR.h>
#include <Netlistmgr.h>

using namespace Pylon;
using namespace Basler_GigECamera;

// CSJTU_AGV_DectDlg 对话框
class CSJTU_AGV_DectDlg : public CDialogEx, public CTcpServerListener
{
// 构造
public:
	CSJTU_AGV_DectDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SJTU_AGV_DECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	char vbuf[100];
	CString cstr;
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
public:
	GLOG m_log;
	ITcpServer* m_server;
	CONNID m_dwConnid;
	cv::Mat m_src;
	CListCtrl m_result_list;
	CComboBox m_combox_select;
	afx_msg void OnBnClickedButtonSetcap();
	void __init__();
	void SetImgWindow(int Dlgitem, const char* str);
	void ShowMatImg(cv::Mat src, int Dlgitem, const char* str);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	int DectMain(cv::Mat src, int thre, double min_area, double max_area,
		float min_whsub, float max_whsub);
	void DrawBox(CvBox2D box, cv::Mat &dst, cv::Scalar color);
	CBaslerGigEInstantCamera m_Camera;
	int openCamera();
	bool TriggerCameraAndGrabImage(CBaslerGigEInstantCamera& cameraObject, CGrabResultPtr& ptrGrabResult, cv::Mat& srcImg);
	void closeCamera();
	CRect m_wndRect;
	CBrush m_greenBrush;
	CBrush m_redBrush;
	CBrush m_grayBrush;
	CFont m_bigFont;
	int m_cnt;
	AGVResult m_result;
	void ShowResult(AGVResult result);
	void ShowResult(AGVResult result, int cursel);
	int SaveParam(const char* filename);
	int LoadParam(const char* filename);
	std::vector<AGVResult> m_parames;
	bool m_open_cam_flag;
	BayerToBGR m_to;
	bool color_img_flag;
	bool checkIsNetwork();
};

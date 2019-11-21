#pragma once

#define _INFO_FILE_ "..\\info.log"
#define _ERROR_FILE_ "..\\error.log"
#define _DEMARCATE_FILE_	"demarcate.xml"

#define _CAMERA_SN_ "23064615"

#define _WIN_NAME_	"hide_win"

#define IMG_HEIGHT	1080
#define IMG_WIDTH	1920

//±ê×¼¼þW=30mm
#define _AGV_SCALE_	0.03198

struct AGVResult
{
	float pt_x;
	float pt_y;
	float angle;
};

//TEST
#define _TEST_IMG_FILE_	"D:\\Code\\C++\\vc12 MFC\\SJTU_AGV_Dect\\imgs\\Image__2019-10-12__11-50-30.bmp"

#define _TCP_IP_ADR_	L"192.168.0.34"
#define _TCP_IP_PORT_	3000

#define _GITHUB_UPLOAD_	"github.bat"

#define _DECT_THRE_		128
#define _DECT_THRE_1	68
#define _DECT_THRE_5	68
#define _DECT_MIN_AREA_	200000.0
#define _DECT_MAX_AREA_	300000.0
#define _DECT_MIN_SUB_	500.0f
#define _DECT_MAX_SUB_	700.0f
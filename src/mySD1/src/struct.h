#include "stdint.h"
#include "stdio.h"

// 设置界面
struct setMenuItem
{
  uint8_t menuNum;					        /*当前菜单条目数*/
	char *prtStr;						          /*当前项目要显示的内容*/
	void (*fun)(void);					      /*当前项目对应的功能函数*/
	struct setMenuItem *childrenMenu_T;	/*当前项目的子菜单*/
	struct setMenuItem *parentMenu_T;		/*当前项目的父菜单*/
};
struct setMenuItem screenMenu[1];

struct setMenuItem screenMenu[1] = 
{
  {1, (char *)"屏幕亮度", NULL, NULL, setMainMenu},
};

/* 定义父菜单 */
struct setMenuItem setMainMenu[4] =
{
  {4, (char *)"屏幕亮度", NULL, screenMenu, NULL},
  {4, (char *)"屏幕方向", NULL, NULL, NULL},
  {4, (char *)"更新周期", NULL, NULL, NULL},
  {4, (char *)"wifi信息", NULL, NULL, NULL},
};
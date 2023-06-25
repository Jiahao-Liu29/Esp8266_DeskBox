#include "weathernum.h"

#include <TJpg_Decoder.h>

//显示天气图标
void WeatherNum::printfweather(int numx, int numy, String text)
{
  if(text == "晴")
  {
    TJpgDec.drawJpg(numx,numy,t0, sizeof(t0));
  }
  else if(text == "多云")
  {
    TJpgDec.drawJpg(numx,numy,t1, sizeof(t1));
  }
  else if(text == "阴")
  {
    TJpgDec.drawJpg(numx,numy,t2, sizeof(t2));
  }
  else if(text == "阵雨")
  {
    TJpgDec.drawJpg(numx,numy,t3, sizeof(t3));
  }
  else if(text == "雷阵雨")
  {
    TJpgDec.drawJpg(numx,numy,t4, sizeof(t4));
  }
  else if(text == "雷阵雨伴有冰雹")
  {
    TJpgDec.drawJpg(numx,numy,t5, sizeof(t5));
  }
  else if(text == "雨夹雪")
  {
    TJpgDec.drawJpg(numx,numy,t6, sizeof(t6));
  }
  else if(text=="小雨"||text=="中雨"||text=="小到中雨"||text=="中到大雨")
  {
    TJpgDec.drawJpg(numx,numy,t7, sizeof(t7));
  }
  else if(text=="大雨"||text=="暴雨"||text=="大到暴雨"||text=="暴雨到大暴雨")
  {
    TJpgDec.drawJpg(numx,numy,t9, sizeof(t9));
  }
  else if(text=="大暴雨"||text=="特大暴雨"||text=="大暴雨到特大暴雨"||text=="雨")
  {
    TJpgDec.drawJpg(numx,numy,t11, sizeof(t11));
  }
  else if(text=="阵雪")
  {
    TJpgDec.drawJpg(numx,numy,t13, sizeof(t13));
  }
  else if(text=="小雪"||text=="小到中雪")
  {
    TJpgDec.drawJpg(numx,numy,t14, sizeof(t14));
  }
  else if(text=="中雪"||text=="中到大雪")
  {
    TJpgDec.drawJpg(numx,numy,t15, sizeof(t15));
  }
  else if(text=="大雪"||text=="暴雪"||text=="大到暴雪"||text=="雪")
  {
    TJpgDec.drawJpg(numx,numy,t16, sizeof(t16));
  }
  else if(text=="雾")
  {
    TJpgDec.drawJpg(numx,numy,t18, sizeof(t18));
  }
  else if(text=="冻雨")
  {
    TJpgDec.drawJpg(numx,numy,t19, sizeof(t19));
  }
  else if(text=="沙尘暴")
  {
    TJpgDec.drawJpg(numx,numy,t20, sizeof(t20));
  }
  else if(text=="浮尘")
  {
    TJpgDec.drawJpg(numx,numy,t29, sizeof(t29));
  }
  else if(text=="扬沙")
  {
    TJpgDec.drawJpg(numx,numy,t30, sizeof(t30));
  }
  else if(text=="强沙尘暴")
  {
    TJpgDec.drawJpg(numx,numy,t31, sizeof(t31));
  }
  else if(text=="霾"||text=="浓雾"||text=="强浓雾"||text=="中度霾"||text=="重度霾"||text=="严重霾"||text=="大雾"||text=="特强浓雾")
  {
    TJpgDec.drawJpg(numx,numy,t53, sizeof(t53));
  }
  else
  {
    TJpgDec.drawJpg(numx,numy,t99, sizeof(t99));
  }

  
}

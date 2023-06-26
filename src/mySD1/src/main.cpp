#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <TFT_eSPI.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <Button2.h>
#include <SPI.h>
#include <TJpg_Decoder.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#include "weathernum.h"

// font
#include "font/font_t1_64x80.h"
#include "font/font_zhong.h"
#include "font/font_zhongCity28.h"
#include "font/font_t2_60x64.h"
#include "font/font_t2_64.h"

// icon
#include "img/icon/icon_temp.h"
#include "img/icon/icon_weath.h"
#include "img/icon/icon_wifibmp.h"
#include "img/icon/icon_set.h"
#include "img/icon/icon_cpu.h"
#include "img/icon/icon_flash.h"

#define Version  "mySD V1.0.0"

/* 定义按钮引脚 */
Button2 Button_sw1 = Button2(4); 
Button2 Button_UP  = Button2(D0, INPUT_PULLDOWN_16, false);

/* tft 相关参数 */
#define LCD_BL_PIN 5
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite eSprite = TFT_eSprite(&tft);

uint8_t LCD_Rotation = 0;   //LCD屏幕方向
uint8_t LCD_BL_PWM = 50;//屏幕亮度0-100，默认50
#define tft_CalBL(x)    ((int)(1023 - ((100 - (x))*10)))

/* 系统各项参数 */
uint8_t updata_flag = 1;
int weaterTime = 0; //天气更新时间记录
uint8_t Wifi_en = 1; //wifi状态标志位  1：打开    0：关闭
int updateweater_time = 1;
bool infoShow = false;

/* 网络相关配置 */
WiFiManager wm;
struct config_type
{
  char stassid[32];//定义配网得到的WIFI名长度(最大32字节)
  char stapsw[64];//定义配网得到的WIFI密码长度(最大64字节)
};

//---------------修改此处""内的信息--------------------
//如开启WEB配网则可不用设置这里的参数，前一个为wifi ssid，后一个为密码
config_type wificonf ={{"huawei"},{"20020903ljh"}};

/* EEPROM 参数 */
int BL_addr = 1;      //被写入数据的EEPROM地址编号  1 亮度
int Ro_addr = 2;      //被写入数据的EEPROM地址编号  2 旋转方向
int weather_addr = 3; //被写入数据的EEPROM地址编号  3 天气更新间隔
int City_addr = 10;   //被写入数据的EEPROM地址编号  10 城市
int wifi_addr = 40;   //被写入数据的EEPROM地址编号  40 wifi-ssid-psw

/* NTP 获取时间参数 */
WiFiUDP udp;
uint16_t localPort = 8000;
time_t prevDisplay = 0;       //显示时间显示记录
//NTP服务器参数
static const char ntpServerName[] = "ntp6.aliyun.com";
const int timeZone = 8;     //东八区

/* api 接口信息 */
WiFiClient wifiClient;
String AK = "XnGaK9n8X5oQc3GWm1EacC9TxQ9lwXma";
String host = "api.map.baidu.com";

char cityArray[32] = {"洛龙区"};
String cityID;

/* 天气对象 */
WeatherNum  wrat;
struct nowWeather_Info
{
  String text;          // 天气
  int temp;             // 温度
  int feels_like;       // 体感温度
  int rh;               // 相对湿度
  String wind_class;    // 风力等级
  String wind_dir;      // 风向描述
};
struct forecasts_Info
{
  String text_day;      // 白天天气
  String text_night;    // 夜晚天气
  int high;             // 最高温度
  int low;              // 最低温度
  String wc_day;        // 白天风力
  String wd_day;        // 白天风向
  String wc_night;      // 夜晚风力
  String wd_night;      // 夜晚风向
  String week;          // 星期
};

nowWeather_Info weatherInfo = {};
forecasts_Info  forecastsInfo[5] = {};

/* 状态机 */
uint8_t menu_num = 3;
uint16_t menu_flag = 0;

#define LOW_POWER     0x0001  // 低功耗界面
#define MAIN_MENU     0x0002  // 多功能界面
#define SET_MENU      0x0004  // 设置界面
#define SELECT_MENU   0x0008  // 设置子界面
#define ISSAVE_MENU   0x0010  // 是否保存界面
#define INFO_MENU     0x0020  // 系统信息界面

/******************************* 产品/设备配置 ****************************************/
#define PRODUCT_KEY "iym4xOYxNVB"
#define DEVICE_NAME "ESP82661"
#define MQTT_SERVER "iot-06z00gwmda9q68k.mqtt.iothub.aliyuncs.com"
#define MQTT_PORT   1883
#define MQTT_CLIENT_ID  PRODUCT_KEY "." DEVICE_NAME "|securemode=2,signmethod=hmacsha256,timestamp=1687767843838|"
#define MQTT_USERNAME   DEVICE_NAME "&" PRODUCT_KEY
#define MQTT_PASSWORD   "5f9f447a304ce35789b9a1e79ed1e5a9f0eb53962f359bc1e2d587519d795490"
// 相关主题
#define TOPIC1 "/" PRODUCT_KEY "/" DEVICE_NAME "/user/updataStatus"
/*************************************************************************************/

/* 获取天气信息 */
void weatherInfoGet(WiFiClient client)
{
  const size_t capacity = 2048;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, client);

  JsonObject result = doc["result"];
  JsonObject now = result["now"];

  weatherInfo.text = now["text"].as<String>();
  weatherInfo.temp = now["temp"].as<int>();
  weatherInfo.feels_like = now["feels_like"].as<int>();
  weatherInfo.rh = now["rh"].as<int>();
  weatherInfo.wind_class = now["wind_class"].as<String>();
  weatherInfo.wind_dir = now["wind_dir"].as<String>();

  for (uint8_t i = 0; i < 5; i ++) {
    forecastsInfo[i].text_day = result["forecasts"][i]["text_day"].as<String>();
    forecastsInfo[i].text_night = result["forecasts"][i]["text_night"].as<String>();
    forecastsInfo[i].high = result["forecasts"][i]["high"].as<int>();
    forecastsInfo[i].low = result["forecasts"][i]["low"].as<int>();
    forecastsInfo[i].wc_day = result["forecasts"][i]["wc_day"].as<String>();
    forecastsInfo[i].wd_day = result["forecasts"][i]["wd_day"].as<String>();
    forecastsInfo[i].wc_night = result["forecasts"][i]["wc_night"].as<String>();
    forecastsInfo[i].wd_night = result["forecasts"][i]["wd_night"].as<String>();
    forecastsInfo[i].week = result["forecasts"][i]["week"].as<String>();
  }

  Serial.printf("info: 天气：%c\t温度：%d\t体感温度：%d\t相对湿度：%d\n", weatherInfo.text, weatherInfo.temp, weatherInfo.feels_like, weatherInfo.rh);
}

// https://api.map.baidu.com/weather/v1/?district_id=222405&data_type=all&ak=XnGaK9n8X5oQc3GWm1EacC9TxQ9lwXma
void weatherRequest(String district_id)
{
  String GetUrl = "/weather/v1/?ak=";
  GetUrl += AK;
  GetUrl += "&data_type=all&district_id=";
  GetUrl += district_id;

  String requestUrl = String("GET ") + GetUrl + " HTTP/1.1\r\n" +
                            "Host: " + host + "\r\n" +
                            "Connection: close\r\n\r\n";

  // 向服务器发送 http 请求
  wifiClient.print(requestUrl);
  Serial.print("Creat a request: ");
  Serial.println("https://" + host + GetUrl);

  // 获取并显示服务器响应状态行 
  String status_response = wifiClient.readStringUntil('\n');
  Serial.print("status_response: ");
  Serial.println(status_response);

  // 使用find跳过HTTP响应头
  if (wifiClient.find("\r\n\r\n")) {
      Serial.println("Found Header End. Start Parsing.");
  }

  weatherInfoGet(wifiClient);
}

/* 获取城市id */
void cityInfoGet(WiFiClient client) 
{
  const size_t capacity = JSON_ARRAY_SIZE(0) + JSON_ARRAY_SIZE(1) + 2*JSON_OBJECT_SIZE(4) + 94;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, client);

  JsonObject result = doc["districts"][0];

  cityID = result["code"].as<String>();
  Serial.print("Get cityId: ");
  Serial.println(result["code"].as<String>());
}

// https://api.map.baidu.com/api_region_search/v1/?keyword=洛龙区&sub_admin=0&extensions_code=1&ak=XnGaK9n8X5oQc3GWm1EacC9TxQ9lwXma
void cityIDRequest(String city)
{
  String GetUrl = "/api_region_search/v1/?ak=";
  GetUrl += AK;
  GetUrl += "&sub_admin=0&extensions_code=1&keyword=";
  GetUrl += city;

  String requestUrl = String("GET ") + GetUrl + " HTTP/1.1\r\n" +
                            "Host: " + host + "\r\n" +
                            "Connection: keep-alive\r\n\r\n";

  // 向服务器发送 http 请求
  wifiClient.print(requestUrl);
  Serial.print("Creat a request: ");
  Serial.println("https://" + host + GetUrl);

  // 获取并显示服务器响应状态行 
  String status_response = wifiClient.readStringUntil('\n');
  Serial.print("status_response: ");
  Serial.println(status_response);

  // 使用find跳过HTTP响应头
  if (wifiClient.find("\r\n\r\n")) {
      Serial.println("Found Header End. Start Parsing.");
  }

  cityInfoGet(wifiClient);
}

//TFT屏幕输出函数
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}

//进度条函数
byte loadNum = 6;
void loading(byte addtime)//绘制进度条
{
  eSprite.setColorDepth(8);
  
  eSprite.createSprite(200, 100);//创建窗口
  eSprite.fillSprite(0x0000);   //填充率

  eSprite.drawRoundRect(0,0,200,16,8,0xFFFF);       //空心圆角矩形
  eSprite.fillRoundRect(3,3,loadNum,10,5,0xFFFF);   //实心圆角矩形
  eSprite.setTextDatum(CC_DATUM);   //设置文本数据
  eSprite.setTextColor(TFT_GREEN, 0x0000); 
  eSprite.drawString("Connecting to WiFi......",100,40,2);
  eSprite.setTextColor(TFT_WHITE, 0x0000); 
  eSprite.drawRightString(Version,180,60,2);
  eSprite.pushSprite(20,120);  //窗口位置
  
  eSprite.deleteSprite();
  loadNum += 1;
  delay(addtime);
}

String getParam(String name){
  //read parameter from server, for customhmtl input
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

//WEB配网LCD显示函数
void Web_win()
{
  eSprite.setColorDepth(8);
  
  eSprite.createSprite(200, 60);//创建窗口
  eSprite.fillSprite(0x0000);   //填充率

  eSprite.setTextDatum(CC_DATUM);   //设置文本数据
  eSprite.setTextColor(TFT_GREEN, 0x0000); 
  eSprite.drawString("WiFi Connect Fail!",100,10,2);
  eSprite.drawString("SSID:",45,40,2);
  eSprite.setTextColor(TFT_WHITE, 0x0000); 
  eSprite.drawString("AutoConnectAP",125,40,2);
  eSprite.pushSprite(20,50);  //窗口位置
    
  eSprite.deleteSprite();
}

void saveParamCallback()
{
  String city;

  // updateweater_time = getParam("WeaterUpdateTime").toInt();
  city =  getParam("CityCode");
  LCD_Rotation = getParam("set_rotation").toInt();
  LCD_BL_PWM = getParam("LCDBL").toInt();
  updateweater_time = getParam("WeaterUpdateTime").toInt();

  // 城市
  strcpy(cityArray, city.c_str());
  Serial.println(cityArray);
  // 读出 eeprom 中的城市信息
  char temp_city[32];
  uint8_t *data = (uint8_t*)(&temp_city);
  for (uint8_t i = 0; i < sizeof(temp_city); i++)
  {
    *(data + i) = EEPROM.read(i + City_addr);
  }
  Serial.print("temp city: ");
  Serial.println(temp_city);
  if(temp_city != cityArray) {
    // 写入城市信息
    uint8_t *p = (uint8_t*)(&cityArray);
    for (uint8_t i = 0; i < sizeof(cityArray); i++)
    {
      EEPROM.write(i + City_addr, *(p + i)); //在闪存内模拟写入
    }
    delay(10);
    EEPROM.commit();//执行写入ROM
    delay(10);
  }
  
  // 屏幕方向
  if(EEPROM.read(Ro_addr) != LCD_Rotation) {
    EEPROM.write(Ro_addr, LCD_Rotation);
    EEPROM.commit();
    delay(5);
  }
  tft.setRotation(LCD_Rotation);
  tft.fillScreen(TFT_BLACK);
  Web_win();
  loadNum--;
  loading(1);

  // 屏幕亮度
  if(EEPROM.read(BL_addr) != LCD_BL_PWM)
  {
    EEPROM.write(BL_addr, LCD_BL_PWM);
    EEPROM.commit();
    delay(5);
  }
  analogWrite(LCD_BL_PIN, tft_CalBL(LCD_BL_PWM));

  // 天气更新间隔
  if(EEPROM.read(weather_addr) != updateweater_time) 
  {
    EEPROM.write(weather_addr, updateweater_time);
    EEPROM.commit();
    delay(5);
  }
}

//WEB配网函数
void WebConfig()
{
  WiFi.mode(WIFI_STA);

  delay(3000);

  wm.resetSettings();

  const char* set_rotation = "<br/><label for='set_rotation'>显示方向设置</label><br>\
                            <div style='text-align:center;'>\
                            <input type='radio' name='set_rotation' value='0' checked> USB接口朝下<br>\
                            <input type='radio' name='set_rotation' value='1'> USB接口朝右<br>\
                            <input type='radio' name='set_rotation' value='2'> USB接口朝上<br>\
                            <input type='radio' name='set_rotation' value='3'> USB接口朝左</div>";

  WiFiManagerParameter  custom_rot(set_rotation);
  WiFiManagerParameter  custom_bl("LCDBL","屏幕亮度(1-100)","50",3);

  WiFiManagerParameter  custom_weatertime("WeaterUpdateTime","天气刷新时间（分钟）","10",3);
  WiFiManagerParameter  custom_cc("CityCode","城市代码","洛龙区",9);
  WiFiManagerParameter  p_lineBreak_notext("<p></p>");

  wm.addParameter(&p_lineBreak_notext);
  wm.addParameter(&custom_cc);
  wm.addParameter(&p_lineBreak_notext);
  wm.addParameter(&custom_bl);
  wm.addParameter(&p_lineBreak_notext);
  wm.addParameter(&custom_weatertime);
  wm.addParameter(&p_lineBreak_notext);
  wm.addParameter(&custom_rot);

  wm.setSaveParamsCallback(saveParamCallback);
  std::vector<const char *> menu = {"wifi","restart"};
  wm.setMenu(menu);
  wm.setClass("invert");

  wm.setMinimumSignalQuality(20);

  bool res;
  res = wm.autoConnect("AutoConnectAP"); // anonymous ap

  while(!res);
}

/* 从 EEPROM 读取 tft 信息 */
void eeprom_readtftInfo(void)
{
  // 从 eeprom 读取背光亮度设置
  if(EEPROM.read(BL_addr) > 0 && EEPROM.read(BL_addr) < 100)
    LCD_BL_PWM = EEPROM.read(BL_addr);
  // 从 eeprom 读取屏幕方向设置
  if(EEPROM.read(Ro_addr) >= 0 && EEPROM.read(Ro_addr) <= 3)
    LCD_Rotation = EEPROM.read(Ro_addr);
}

/* 从 EEPROM 读取 wifi 信息 */
void eeprom_readwifiInfo(void)
{
  uint8_t *p = (uint8_t*)(&wificonf);
  for (uint8_t i = 0; i < sizeof(wificonf); i++)
  {
    *(p + i) = EEPROM.read(i + wifi_addr);
  }
  Serial.printf("Read WiFi Config.....\r\n");
  Serial.printf("SSID:%s\r\n",wificonf.stassid);
  Serial.printf("PSW:%s\r\n",wificonf.stapsw);
  Serial.printf("Connecting.....\r\n");
}

/* 向 EEPROM 写入 wifi 信息 */
void eeprom_writewifiInfo(void)
{
  //开始写入
  uint8_t *p = (uint8_t*)(&wificonf);
  for (uint8_t i = 0; i < sizeof(wificonf); i++)
  {
    EEPROM.write(i + wifi_addr, *(p + i)); //在闪存内模拟写入
  }
  delay(10);
  EEPROM.commit();//执行写入ROM
  delay(10);
}

/* 清空 EEPROM 中的 Wifi 信息 */
void eeprom_deletewifiInfo(void)
{
  config_type deletewifi ={{""},{""}};
  uint8_t *p = (uint8_t*)(&deletewifi);
  for (uint8_t i = 0; i < sizeof(deletewifi); i++)
  {
    EEPROM.write(i + wifi_addr, *(p + i)); //在闪存内模拟写入
  }
  delay(10);
  EEPROM.commit();//执行写入ROM
  delay(10);
}

const int NTP_PACKET_SIZE = 48; // NTP时间在消息的前48字节中
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// 向NTP服务器发送请求
void sendNTPpacket(IPAddress &address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

// 获取 NTP 时间
time_t getNtpTime(void)
{
  IPAddress ntpServerIp;

  while (udp.parsePacket() > 0);

  WiFi.hostByName(ntpServerName, ntpServerIp);

  sendNTPpacket(ntpServerIp);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      //Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // 无法获取时间时返回0
}

void scan_num(uint16_t x, uint16_t y, int num, uint8_t weiflag, bool flag)
{
  uint8_t index_high, index_low;
  uint8_t num_high = num / 10;
  uint8_t num_low = num % 10;
  num_high > 0 ? index_high = num_high - 1: index_high = 5;
  num_low > 0 ? index_low = num_low - 1: index_low = 9;
  for(uint8_t i = 0; i < 9; i ++) {
    if(flag && weiflag==1) {
      if(index_high == 5)
      {
        tft.drawBitmap(x, y, &font_t1_64x80[5*font_t1_bytes + i*40], 32, font_t1_height-((i/2)*20), TFT_WHITE, TFT_BLACK);
        tft.drawBitmap(x, y+font_t1_height-((i/2)*20), &font_t1_64x80[0], 32, ((i/2)*20), TFT_WHITE, TFT_BLACK);
      } else
        tft.drawBitmap(x, y, &font_t1_64x80[index_high*font_t1_bytes + i*40], 32, font_t1_height, TFT_WHITE, TFT_BLACK);
    }
    tft.drawBitmap(x+32, y, &font_t1_64x80[index_low*font_t1_bytes + i*40], 32, font_t1_height, TFT_WHITE, TFT_BLACK);
  }
}

/* 时钟刷新 */
unsigned char Hour_sign   = 60;
unsigned char Minute_sign = 60;
unsigned char Second_sign = 60;
uint8_t Hour_flag = 6;
uint8_t Minute_flag = 6;
uint8_t Second_flag = 6;

//星期
String week()
{
  String wk[7] = {"日","一","二","三","四","五","六"};
  String s = "周" + wk[weekday()-1];
  return s;
}

//月日
String monthDay()
{
  String s = String(month());
  s = s + "月" + day() + "日";
  return s;
}

/* 电池电量 */
/*
* 电池电量图标有效范围 0-24
*/
void liVal(uint8_t value)
{
  eSprite.createSprite(33, 28);
  eSprite.fillSprite(TFT_BLACK);

  eSprite.drawRoundRect(2, 8, 3, 10, 1, TFT_WHITE);
  eSprite.drawRoundRect(5, 6, 28, 16, 2, TFT_WHITE);
  eSprite.fillRect(7, 8, 24, 12, TFT_GREEN);
  eSprite.fillRect(7, 8, value, 12, TFT_BLACK);
  eSprite.pushSprite(205, 2);

  eSprite.deleteSprite();
}

// 息屏时钟
void clockDisplay(uint8_t relashFlag)
{
  // 秒钟刷新
  if(second() != Second_sign || relashFlag == 1) {
    if(Second_flag != second()/10 || relashFlag == 1)
      scan_num(175, 70, second(), 1, true);
    else
      scan_num(175, 70, second(), 1, false);
    if(second() % 2 == 0) {
      tft.drawBitmap(79, 100, font_t1_symbol, 16, 32, TFT_WHITE, TFT_BLACK);
      tft.drawBitmap(159, 100, font_t1_symbol, 16, 32, TFT_WHITE, TFT_BLACK);
    } else {
      tft.drawBitmap(79, 100, font_t1_symbol, 16, 32, TFT_BLACK, TFT_BLACK);
      tft.drawBitmap(159, 100, font_t1_symbol, 16, 32, TFT_BLACK, TFT_BLACK);
    }
    Second_sign = second();
    Second_flag = second()/10;
  }
  // 分钟刷新
  if(minute() != Minute_sign || relashFlag == 1) {
    if(Minute_flag != minute()/10 || relashFlag == 1)
      scan_num(95, 70, minute(), 1, true);
    else
      scan_num(95, 70, minute(), 1, false);
    Minute_sign = minute();
    Minute_flag = minute()/10;
  }
  // 小时刷新
  if(hour() != Hour_sign || relashFlag == 1) {
    if(Hour_flag != hour()/10 || relashFlag == 1)
      scan_num(15, 70, hour(), 1, true);
    else
      scan_num(15, 70, hour(), 1, false);
    Hour_sign = hour();
    Hour_flag = hour()/10;
  }

  if(relashFlag == 1) relashFlag = 0;
  /***日期****/
  eSprite.loadFont(font_zhong);

  //星期
  eSprite.createSprite(58, 30);
  eSprite.fillSprite(TFT_BLACK);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString(week(),29,16);
  eSprite.pushSprite(102,150);
  eSprite.deleteSprite();

  //月日
  eSprite.createSprite(95, 30);
  eSprite.fillSprite(TFT_BLACK);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);  
  eSprite.drawString(monthDay(),49,16);
  eSprite.pushSprite(5,150);
  eSprite.deleteSprite();

  eSprite.unloadFont();
}

void clockShow()
{
  // 翻页时钟效果
  String secondH = String(second() / 10);
  String secondL = String(second() % 10);
  eSprite.loadFont(DINNextLTPro_Bold64);
  eSprite.createSprite(115, 60);
  eSprite.fillSprite(TFT_BLACK);
  eSprite.fillRoundRect(28, 0, 80, 60, 4, 0x8410);  // 特定颜色
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_LIGHTGREY, 0x8410); 
  eSprite.drawString(secondH, 47, 35);
  eSprite.drawString(secondL, 83, 35);
  eSprite.fillRect(28, 28, 80, 4, TFT_BLACK);
  eSprite.pushSprite(110,75);
  eSprite.deleteSprite();
  eSprite.unloadFont();
}

/* 多功能界面 */
int flashcout = 0;
void mainDisplay(uint8_t enFlag)
{
  /******************************* 静态部分 ************************************/
  // 城市名称
  eSprite.loadFont(font_zhong25);

  eSprite.createSprite(94, 30); 
  eSprite.fillSprite(TFT_BLACK);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK); 
  eSprite.drawString(cityArray, 44, 16);
  eSprite.pushSprite(15,10);
  eSprite.deleteSprite();

  // 天气详情
  String text = weatherInfo.text;
  uint16_t textBgColor;
  if (text == "晴")
    textBgColor = TFT_GREEN;
  else if(text == "多云")
    textBgColor = 0xA7F9;
  else if(text == "阴")
    textBgColor = TFT_LIGHTGREY;
  else
    textBgColor = 0xDD24;

  eSprite.createSprite(100, 30); 
  eSprite.fillSprite(TFT_BLACK);
  eSprite.fillRoundRect(0, 0, 100, 30, 5, textBgColor);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_BLACK, TFT_WHITE); 
  eSprite.drawString(text, 48, 16);
  eSprite.pushSprite(5, 130);
  eSprite.deleteSprite();

  /***日期****/
  String wk[7] = {"日","一","二","三","四","五","六"};
  String weekstr = "星期" + wk[weekday()-1];

  //星期
  eSprite.createSprite(80, 30);
  eSprite.fillSprite(TFT_BLACK);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString(weekstr,40,16);
  eSprite.pushSprite(138,40);
  eSprite.deleteSprite();

  eSprite.unloadFont();

  // 天气图标
  wrat.printfweather(23, 50, weatherInfo.text);

  // 温度图标
  TJpgDec.drawJpg(5, 173, icon_temp, sizeof(icon_temp));
  // 湿度图标
  TJpgDec.drawJpg(5, 210, icon_weath, sizeof(icon_weath));

  // wifi图标
  tft.drawBitmap(170, 2, icon_wifi, 28, 28, TFT_LIGHTGREY);

  /******************************* 数据部分 ************************************/
  eSprite.loadFont(font_zhong25);
  // 温度数据
  eSprite.createSprite(68, 30); 
  eSprite.fillSprite(TFT_BLACK);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK); 
  eSprite.drawString(String(weatherInfo.temp) + "℃", 32, 16);
  eSprite.pushSprite(37,173);
  eSprite.deleteSprite();

  // 湿度数据
  eSprite.createSprite(68, 30); 
  eSprite.fillSprite(TFT_BLACK);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK); 
  eSprite.drawString(String(weatherInfo.rh) + "%", 32, 16);
  eSprite.pushSprite(37,210);
  eSprite.deleteSprite();

  eSprite.unloadFont();

  // 分钟刷新
  tft.drawBitmap(177, 175, font_t2_60x64[minute()/10], 30, 64, TFT_BROWN, TFT_BLACK);
  tft.drawBitmap(210, 175, font_t2_60x64[minute()%10], 30, 64, TFT_BROWN, TFT_BLACK);
  // 小时刷新
  tft.drawBitmap(102, 175, font_t2_60x64[hour()/10], 30, 64, TFT_GREENYELLOW, TFT_BLACK);
  tft.drawBitmap(134, 175, font_t2_60x64[hour()%10], 30, 64, TFT_GREENYELLOW, TFT_BLACK);

  /******************************* 动态部分 ************************************/
  // 秒刷新
  // 动态点
  if (second() % 2 == 0) {
    tft.drawBitmap(166, 191, font_t2_symbol, 8, 32, TFT_WHITE, TFT_BLACK);
  } else {
    tft.drawBitmap(166, 191, font_t2_symbol, 8, 32, TFT_BLACK, TFT_BLACK);
  }

  // 天气详情以及日期刷新
  // 每 7 秒刷新一次天气详情
  if (second() % 8 == 0)
  {
    flashcout = second();
    infoShow = true;
  }
  else if(second() == flashcout + 2)
  {
    flashcout = 0;
    infoShow = false;
  }
  if (infoShow) {
    eSprite.loadFont(font_zhong25);

    eSprite.createSprite(130, 100);  // 创建区域
    eSprite.fillScreen(TFT_BLACK);  // 覆盖区域

    // 最高 / 最低气温
    eSprite.setTextDatum(CC_DATUM);
    eSprite.setTextColor(0xFB08, TFT_BLACK);
    eSprite.drawString(String(forecastsInfo[0].high), 35, 20);
    eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    eSprite.drawString(" / ", 60, 20);
    eSprite.setTextColor(0x07FF, TFT_BLACK);
    eSprite.drawString(String(forecastsInfo[0].low), 85, 20);
    eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    eSprite.drawString("℃", 110, 20);
    
    // 风力风向
    eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    eSprite.drawString(forecastsInfo[0].wc_day, 60, 53);
    eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    eSprite.drawString(forecastsInfo[0].wd_day, 60, 86);

    eSprite.pushSprite(110, 75);
    eSprite.deleteSprite(); // 释放对象

    eSprite.unloadFont();
  } else {
    eSprite.loadFont(font_zhong25);

    //年月
    eSprite.createSprite(180, 30);
    eSprite.fillSprite(TFT_BLACK);
    eSprite.setTextDatum(CC_DATUM);
    eSprite.setTextColor(TFT_WHITE, TFT_BLACK);  
    eSprite.drawString(String(year()) + "年" + String(month()) + "月",65,16);
    eSprite.pushSprite(110,145);
    eSprite.deleteSprite();

    eSprite.unloadFont();

    // 日期 (日历效果)
    eSprite.loadFont(DINNextLTPro_Bold64);
    eSprite.createSprite(130, 70);
    eSprite.fillSprite(TFT_BLACK);
    eSprite.fillRoundRect(28, 0, 80, 60, 4, 0x8410);  // 特定颜色
    eSprite.setTextDatum(CC_DATUM);
    eSprite.setTextColor(TFT_LIGHTGREY, 0x8410); 
    eSprite.drawString(String(day()), 65, 35);
    eSprite.fillRect(28, 28, 80, 4, TFT_BLACK);
    eSprite.pushSprite(110, 75);
    eSprite.deleteSprite();
    eSprite.unloadFont();
  }
} 

/* 系统信息界面 */
void systemInfo(void)
{
  // CPU 图标
  TJpgDec.drawJpg(0, 40, icon_cpu, sizeof(icon_cpu));

  // flash 图标
  TJpgDec.drawJpg(0, 109, icon_flash, sizeof(icon_flash));

  /* 数据显示 */
  eSprite.loadFont(font_zhong25);

  eSprite.createSprite(160, 64);
  eSprite.setTextDatum(CL_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("ESP8266", 5, 20);
  eSprite.drawString(String(ESP.getCpuFreqMHz()) + " Mhz", 5, 50);
  eSprite.pushSprite(64, 40);
  eSprite.deleteSprite();

  uint32_t sizeBin = 0;
  uint32_t sizeSpaceBin = 0;
  sizeBin = uint32_t(ESP.getSketchSize() / 1024); // 无符号32位整数返回当前固件大小
  sizeSpaceBin = uint32_t(ESP.getFreeSketchSpace() / 1024); // 无符号32位整数的形式返回可用空闲固件空间

  eSprite.createSprite(200, 64);
  eSprite.setTextDatum(CL_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString(String(uint32_t(ESP.getFlashChipRealSize() / 1024)) + " KB", 5, 20);
  eSprite.setTextColor(TFT_RED, TFT_BLACK);
  eSprite.drawString(String(sizeBin), 5, 50);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("/", 17*String(sizeBin).length(), 50);
  eSprite.setTextColor(TFT_BLUE, TFT_BLACK);
  eSprite.drawString(String(sizeSpaceBin), 17*String(sizeBin).length() + 16, 50);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("KB", 17*String(sizeBin).length()+15+17*String(sizeSpaceBin).length(), 50);
  eSprite.pushSprite(64, 109);
  eSprite.deleteSprite();

  eSprite.createSprite(100, 30);
  eSprite.setTextDatum(CL_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("亮度: " + String(LCD_BL_PWM), 5, 15);
  eSprite.pushSprite(5, 178);
  eSprite.deleteSprite();

  eSprite.createSprite(180, 30);
  eSprite.setTextDatum(CL_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("更新周期: " + String(updateweater_time), 5, 15);
  eSprite.pushSprite(5, 213);
  eSprite.deleteSprite();

  eSprite.unloadFont();
}

// 设置界面
struct setMenuItem
{
  uint8_t menuNum;					        /*当前菜单条目数*/
	char *prtStr;						          /*当前项目要显示的内容*/
	void (*fun)(void);					      /*当前项目对应的功能函数*/
	struct setMenuItem *childrenMenu_T;	/*当前项目的子菜单*/
	struct setMenuItem *parentMenu_T;		/*当前项目的父菜单*/
};

// struct setMenuItem screenMenu[1] =
// {
//   {1, (char *)"屏幕亮度", NULL, NULL, setMainMenu},
// };

/* 定义父菜单 */
setMenuItem setMainMenu[4] =
{
  {4, (char *)"屏幕亮度", NULL, NULL, NULL},
  {4, (char *)"屏幕方向", NULL, NULL, NULL},
  {4, (char *)"更新周期", NULL, NULL, NULL},
  {4, (char *)"wifi信息", NULL, NULL, NULL},
};

setMenuItem *ptrSelectMenu = setMainMenu;
int indexnum = 0;  // 标号
uint8_t addtime = 0;
uint8_t movetime = 0;
/* 菜单刷新 */
void setMenuDis(void)
{
  eSprite.loadFont(font_zhong25);

  for (uint8_t i = 0; i < ptrSelectMenu->menuNum; i++)
  {
    eSprite.createSprite(140, 50);
    eSprite.setTextDatum(CC_DATUM);
    // 选中效果
    if (i == indexnum) {
      // 背景框动态增大
      eSprite.fillRoundRect(18-(addtime/5*2), 4-(addtime/5), 104+(addtime/5*4), 30+(addtime/5), 2, TFT_WHITE);
      eSprite.setTextColor(TFT_BLACK, TFT_WHITE); // 设置反色
      // 分隔线动态增大
      eSprite.fillRect(25-addtime, 41-(addtime / 10), 90+(addtime*2), 2+(addtime/10), TFT_WHITE); // 分隔线
    } 
    // 上一个对象效果
    else if (i == ((indexnum - 1) < 0 ? 3 : indexnum - 1) ) {
      // 分隔线动态减小
      eSprite.fillRect(5+addtime, 41+(addtime/10), 130-(addtime*2), 4-(addtime/10), TFT_WHITE); // 分隔线
    }
    else {
      eSprite.setTextColor(TFT_WHITE, TFT_BLACK); // 正常颜色
      eSprite.fillRect(25, 41, 90, 2, TFT_WHITE); // 分隔线
    }
    eSprite.drawString((ptrSelectMenu+i)->prtStr, 70, 19);
    // (55, i*50+40)
    if (i == 0 || i == 1)
      eSprite.pushSprite(movetime*5+5, i*50+40);
    else {
      eSprite.pushSprite(235 - movetime*18, i*50+40);

      /* 前一帧清屏 */
      eSprite.createSprite(20, 50);
      eSprite.fillSprite(TFT_BLACK);
      eSprite.pushSprite(375 - movetime*18, i*50+40);
      eSprite.deleteSprite();
      /* 前一帧清屏 */
    }

    eSprite.deleteSprite();
  }

  eSprite.unloadFont();
  // 动态递增
  addtime++;
  if (addtime >= 20)
    addtime = 20;
  movetime++;
  if (movetime >= 10)
    movetime = 10;
}

/* 屏幕亮度设置 */
void SCREEN_SET(void)
{
  // 汉字提示
  eSprite.loadFont(font_zhong25);

  eSprite.createSprite(110, 30);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString((ptrSelectMenu+indexnum)->prtStr, 55, 15);
  eSprite.pushSprite(65, 40);
  eSprite.deleteSprite();

  eSprite.unloadFont();

  // 箭头 上
  eSprite.createSprite(20, 10);
  eSprite.fillTriangle(3, 10, 10, 0, 17, 10, TFT_WHITE);
  eSprite.pushSprite(110, 81);
  eSprite.deleteSprite();

  // 设置进度条
  // 最大 86
  eSprite.createSprite(20, 104);
  eSprite.drawRect(5, 0, 10, 104, TFT_WHITE);
  eSprite.fillRect(7, 102-LCD_BL_PWM, 6, LCD_BL_PWM, TFT_GREEN);
  eSprite.pushSprite(110, 101);
  eSprite.deleteSprite();

  // 数字提示
  eSprite.loadFont(font_zhong25);
  eSprite.createSprite(75, 35);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString(String(LCD_BL_PWM), 37, 17);
  eSprite.pushSprite(135, 138);
  eSprite.deleteSprite();
  eSprite.unloadFont();

  // 箭头 下
  eSprite.createSprite(20, 11);
  eSprite.fillTriangle(3, 0, 10, 10,  17, 0, TFT_WHITE);
  eSprite.pushSprite(110, 214);
  eSprite.deleteSprite();
}

/* 屏幕显示方向设置 */
void ROTATION_SET(void)
{
  tft.fillScreen(TFT_BLACK);
  // 汉字提示
  eSprite.loadFont(font_zhong25);

  eSprite.createSprite(160, 60);
  eSprite.drawRoundRect(0, 0, 160, 60, 4, TFT_WHITE);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("暂不支持设置", 80, 30);
  eSprite.pushSprite(40, 90);
  eSprite.deleteSprite();

  eSprite.unloadFont();
}

/* 更新周期设置 */
uint8_t scroll;
byte addFlag = 0;
byte minFlag = 0;
void REFLASH_SET(void)
{
  // 汉字提示
  eSprite.loadFont(font_zhong25);

  eSprite.createSprite(110, 30);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString((ptrSelectMenu+indexnum)->prtStr, 55, 15);
  eSprite.pushSprite(65, 40);
  eSprite.deleteSprite();

  eSprite.createSprite(30, 30);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("每", 15, 15);
  eSprite.pushSprite(50, 120);
  eSprite.deleteSprite();

  String timeList[25] = {" ", "01", "02", "03", "04", "05", "06", "07", "08", "09",\
                    "10", "11", "12", "13", "14", "15", "16", "17", "18",\
                    "19", "20", "21", "22", "23", "24"};

  eSprite.createSprite(80, 120);
  eSprite.fillRoundRect(0, 0, 80, 120, 4, TFT_DARKGREY);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.drawRect(5, 35, 70, 2, 0xAD55);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);

  // if ((updateweater_time-1) != 0)
  //   eSprite.drawString(timeList[updateweater_time-1], 40, 20);
  // eSprite.drawString(timeList[updateweater_time], 40, 60);
  // if ((updateweater_time+1) != 25)
  // eSprite.drawString(timeList[updateweater_time+1], 40, 100);

  // 当前更新间隔数值
  if (addFlag == 0) {
    if (updateweater_time > 2)
      eSprite.drawString(timeList[updateweater_time-2], 40, 20 - (scroll));
    for (uint8_t i = updateweater_time; i < 25; i++) {
      eSprite.drawString(timeList[i-1], 40, ((i-updateweater_time)*40)+20 - (scroll - 40));
    }
  } 
  else if (minFlag == 1) {
    if (updateweater_time > 1)
      eSprite.drawString(timeList[updateweater_time-1], 40, (scroll) - 20);
    for (uint8_t i = updateweater_time; i < 25; i++) {
      eSprite.drawString(timeList[i], 40, ((i-updateweater_time)*40)+20 + (scroll));
    }
  }
  else if (addFlag == 1) {
    for (uint8_t i = updateweater_time; i < 25; i++) {
      eSprite.drawString(timeList[i-1], 40, ((i-updateweater_time)*40)+20);
    }
  }
  
  eSprite.drawRect(5, 80, 70, 2, 0xAD55);
  eSprite.pushSprite(80, 80);
  eSprite.deleteSprite();

  eSprite.createSprite(55, 30);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("小时", 24, 15);
  eSprite.pushSprite(160, 120);
  eSprite.deleteSprite();

  eSprite.unloadFont();

  if (addFlag == 0 || minFlag == 1) {
    scroll++;
    if(scroll >= 40) {
      scroll = 40;
      addFlag = 1;
      minFlag = 0;
    }
  }
}

/* wifi信息设置 */
void WIFIINFO_SET(void)
{
  // 13 次
  uint8_t timeDelay = 0;
  while(true) {
  // 汉字提示
  eSprite.loadFont(font_zhong25);

  /* 前一帧清屏 */
  eSprite.createSprite(20, 30);
  eSprite.fillSprite(TFT_BLACK);
  eSprite.pushSprite((timeDelay)*13-130, 40);
  eSprite.deleteSprite();
  /* 前一帧清屏 */

  eSprite.createSprite(110, 30);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString((ptrSelectMenu+indexnum)->prtStr, 55, 15);
  // 位置 （65, 40）
  eSprite.pushSprite(timeDelay*13-110, 40);
  eSprite.deleteSprite();

  eSprite.createSprite(200, 60);
  eSprite.fillRoundRect(0, 0, 200, 64, 4, TFT_DARKGREY);
  eSprite.setTextDatum(CL_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("WIFI: ", 5, 15);
  eSprite.setTextColor(TFT_GREEN, TFT_BLACK);
  eSprite.drawString(wificonf.stassid, 20, 45);
  // (20, 90)
  eSprite.pushSprite(200-((timeDelay / 2) * 30), 90);
  eSprite.deleteSprite();

  eSprite.createSprite(200, 60);
  eSprite.fillSprite(TFT_DARKGREY);
  eSprite.setTextDatum(CL_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("SSID: ", 5, 15);
  eSprite.setTextColor(TFT_GREEN, TFT_BLACK);
  eSprite.drawString(wificonf.stapsw, 20, 45);
  // (20, 90)
  eSprite.pushSprite(200-((timeDelay / 2) *30), 150);
  eSprite.deleteSprite();

  /* 前一帧清屏 */
  eSprite.createSprite(20, 120);
  eSprite.fillSprite(TFT_BLACK);
  eSprite.pushSprite(400-((timeDelay / 2) * 30), 90);
  eSprite.deleteSprite();
  /* 前一帧清屏 */

  eSprite.unloadFont();

  timeDelay++;
  // delay(200);
  if (timeDelay >= 14)
    break;
  }
}

// 是否保存设置
void isSaveMenu(void)
{
  tft.fillScreen(TFT_BLACK);  // 清屏

  eSprite.loadFont(font_zhong25);

  eSprite.createSprite(120, 60);
  eSprite.drawRoundRect(0, 0, 120, 60, 4, TFT_WHITE);
  eSprite.setTextDatum(CC_DATUM);
  eSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  eSprite.drawString("是否保存", 60, 30);
  eSprite.pushSprite(30, 90);
  eSprite.deleteSprite();

  eSprite.createSprite(60, 140);
  eSprite.setTextDatum(CC_DATUM); 
  eSprite.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  eSprite.drawString("√", 5, 30);
  eSprite.drawString("→", 30, 30);
  eSprite.fillRoundRect(50, 5, 20, 60, 4, TFT_WHITE);
  eSprite.setTextColor(TFT_BROWN, TFT_BLACK);
  eSprite.drawString("×", 5, 110);
  eSprite.drawString("→", 30, 110);
  eSprite.fillRoundRect(50, 85, 20, 60, 4, TFT_WHITE);
  eSprite.pushSprite(180, 60);
  eSprite.deleteSprite();

  eSprite.unloadFont();
}

// 屏幕亮度调节菜单回调函数
byte Button1_flag = 0;
byte Button2_flag = 0;
void valChange(void)
{
  if (menu_flag & SELECT_MENU) {
    switch (indexnum)
    {
    case 0: {
      if (Button1_flag) {
        LCD_BL_PWM++;
        Button1_flag = 0;
      }
      else if (Button2_flag) {
        LCD_BL_PWM--;
        Button2_flag =0;
      }
      analogWrite(LCD_BL_PIN, tft_CalBL(LCD_BL_PWM)); 
      SCREEN_SET();
    } break;
    case 2: {
      scroll = 0;
      if (Button1_flag) {
        addFlag = 0;
        updateweater_time < 25 ? updateweater_time++ : updateweater_time=24;
        Button1_flag = 0;
      }
      else if (Button2_flag) {
        minFlag = 1;
        updateweater_time > 1 ? updateweater_time-- : updateweater_time=1;
        Button2_flag = 0;
      }
    } break;
    default:
      break;
    }
  }
}

/* 按键选中回调函数 */
void keyCenter_CallBack(void)
{
  tft.fillScreen(TFT_BLACK);  // 清屏
  menu_flag &= ~SET_MENU; // 首先清除设置界面
  menu_flag |= SELECT_MENU;
  if (indexnum == 0) {
    SCREEN_SET();
  }
  else if (indexnum == 1) {
    ROTATION_SET();
    delay(1000);
    menu_flag |= SET_MENU;
    menu_flag &= ~SELECT_MENU;
    tft.fillScreen(TFT_BLACK);
    movetime = 0;
  } 
  else if (indexnum == 2) {
    REFLASH_SET();
  }
  else if (indexnum == 3) {
    WIFIINFO_SET();
  }
}

void setDisplay(void)
{
  // y - 35 开始
  setMenuDis();
}

void apiRequest(uint8_t flag)
{
  // 判断 tcp 是否处于连接状态，不是就建立连接
  while (!wifiClient.connected()) {
    if(!wifiClient.connect(host, 80)) {
      wifiClient.println("connection....");
    }
  }
  Serial.println("Successed!");

  // flag == 1 只更新天气
  if (flag == 1)
    weatherRequest(cityID);
  else {
    // 否则都更新
    cityIDRequest(cityArray);
    delay(100);
    weatherRequest(cityID);
  }
  wifiClient.stop();
}


/* tft 刷新 */
void tft_reflash(void)
{
  liVal(10);
  // 低功耗界面
  if(menu_flag & LOW_POWER) {
    if(now() != prevDisplay) {
      prevDisplay = now();
      clockDisplay(0);
    }
  } 
  // 多功能界面
  else if (menu_flag & MAIN_MENU) {
    // menu_flag &= ~MAIN_MENU;
    mainDisplay(0);
  }
  // 设置界面
  else if (menu_flag & SET_MENU) {
    setDisplay();
  }
  else if (menu_flag & SELECT_MENU) {
    if (indexnum == 2) {
      REFLASH_SET();  // 有动画，要动态刷新
    }
  }

  /* 天气更新 */
  // 一个小时更新一次
  if(hour() - weaterTime >= updateweater_time)
  {
    if(Wifi_en == 0)
    {
      WiFi.forceSleepWake();  //wifi on
      Serial.println("WIFI恢复......");
      Wifi_en = 1;
    }

    if(WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WIFI已连接");

      apiRequest(updata_flag); // 只更新天气
      
      weaterTime = hour();
      while(!getNtpTime());
      WiFi.forceSleepBegin(); // Wifi Off
      Serial.println("WIFI休眠......");
      Wifi_en = 0;
    }
  }
}

/* esp 复位 */
void esp_reset(Button2& btn){
  ESP.restart();
}

/* wifi 复位 */
void wifi_reset(Button2& btn){
  wm.resetSettings();
  eeprom_deletewifiInfo();
  delay(10);
  Serial.println("重置WiFi成功");
  ESP.restart();
}

/* 界面切换 */
uint8_t button_sw1Flag = 0;
void menu_change(Button2& btn)
{
  if (menu_flag & SELECT_MENU) {
    Button1_flag = 1;
    valChange();
  }
  else if (menu_flag & SET_MENU) {
    keyCenter_CallBack();
  }

  if (!(menu_flag & SELECT_MENU)) {
    tft.fillScreen(TFT_BLACK);  // 清屏
    Serial.println("model changed!");
  }

  if (button_sw1Flag == 0) {
    // 低功耗界面
    menu_flag |= LOW_POWER;
    menu_flag &= ~SET_MENU;
    clockDisplay(1);
  } 
  else if (button_sw1Flag == 1)
  {
    // 多功能界面
    menu_flag |= MAIN_MENU;
    menu_flag &= ~LOW_POWER;
  } 
  else if (button_sw1Flag == 2) {
    // 系统信息界面
    menu_flag |= INFO_MENU;
    menu_flag &= ~MAIN_MENU;
    systemInfo();
  }
  else if (button_sw1Flag == 3 && !(menu_flag & SET_MENU) && !(menu_flag & SELECT_MENU)) {
    // 设置界面
    menu_flag |= SET_MENU;
    menu_flag &= ~INFO_MENU;
    TJpgDec.drawJpg(72, 72, icon_set, sizeof(icon_set));
    delay(1000);
    tft.fillScreen(TFT_BLACK);
  }

  
  if (menu_flag & ISSAVE_MENU) {
    // 保存
    menu_flag |= SET_MENU;
    menu_flag &= ~ISSAVE_MENU;
    // 保存更改
    if (indexnum == 0) {
      // 更新背光
      if(EEPROM.read(BL_addr) != LCD_BL_PWM)
      {
        EEPROM.write(BL_addr, LCD_BL_PWM);
        EEPROM.commit();
        delay(5);
      }
    } else if (indexnum == 2) {
      // 更新天气更新间隔
      if(EEPROM.read(weather_addr) != updateweater_time) 
      {
        EEPROM.write(weather_addr, updateweater_time);
        EEPROM.commit();
        delay(5);
      }
    }
    movetime = 0;
  }
  else if (!(menu_flag & SET_MENU) || !(menu_flag & SELECT_MENU)) {
    button_sw1Flag++;
    // 达到最大菜单数
    // if (button_sw1Flag == menu_num) {
    //   button_sw1Flag = 0;
    // }
  }
}

// +
void key_up(Button2& btn)
{
  if (menu_flag & SET_MENU) {
    addtime = 0;
    if(++indexnum >= setMainMenu->menuNum)
    {
      indexnum = 0;
    }
    setMenuDis();
  } 
  else if (menu_flag & ISSAVE_MENU) {
    // 不保存
    menu_flag |= SET_MENU;
    menu_flag &= ~ISSAVE_MENU;
    if (indexnum == 0) {
      // 不更背光
      if (EEPROM.read(BL_addr) != LCD_BL_PWM) {
        LCD_BL_PWM = EEPROM.read(BL_addr);
        analogWrite(LCD_BL_PIN, tft_CalBL(LCD_BL_PWM)); 
      }
    } else if (indexnum == 2) {
      // 不更新天气更新间隔
      if (EEPROM.read(weather_addr) != updateweater_time) {
        updateweater_time = EEPROM.read(weather_addr);
      }
    }
    tft.fillScreen(TFT_BLACK);
    movetime = 0;
  }
  else {
    Button2_flag = 1;
    valChange();
  }
}

/* 是否保存界面 */
void exit_set(Button2& btn)
{
  if (menu_flag & SELECT_MENU) {
    menu_flag |= ISSAVE_MENU;
    menu_flag &= ~SELECT_MENU;
    if (indexnum != 3)
      isSaveMenu();
    else {
      menu_flag |= SET_MENU;
      menu_flag &= ~ISSAVE_MENU;
      menu_flag &= ~SELECT_MENU;
      tft.fillScreen(TFT_BLACK);
      movetime = 0;
    }
  } else if (menu_flag & SET_MENU) {
    tft.fillScreen(TFT_BLACK);
    menu_flag |= LOW_POWER;
    menu_flag &= ~SET_MENU;
    clockDisplay(1);
    button_sw1Flag = 1;
    indexnum = 0;
    movetime = 0;
  }
}

// 连接心跳函数
void doWifiTick(void)
{
  static bool taskStarted = false;
  static bool startSTAFlag = false;
  static uint32_t lastWifiCheckTick = 0;

  if (!startSTAFlag) {
    startSTAFlag = true;
    Serial.print(wificonf.stassid);
    WiFi.begin(wificonf.stassid, wificonf.stapsw);
    Serial.printf("Heap size: %d\r\n", ESP.getFreeHeap());
  }

  if (WiFi.status() != WL_CONNECTED) {
    if(millis() - lastWifiCheckTick > 1000) {
      lastWifiCheckTick = millis();
      Serial.print(".");
    }
  } else {
    if(taskStarted == false) {
      taskStarted = true;
      Serial.print("\r\nGet IP Address:");
      Serial.println(WiFi.localIP());
    }
  }
}

/* MQTT 数据报解析 */
void praseMqttResponse(char* payload)
{
  Serial.println("start praseMqttResponse");
  StaticJsonDocument<200> doc;

  deserializeJson(doc, payload);

  String deviceName = doc["deviceId"];
  int status = doc["s"];
}

/* MQTT 回调函数 */
void mqtt_callback(char * topic, byte * payload, unsigned int length) {
  byte *end = payload + length;
  for (byte *p = payload; p < end; ++p) {
    Serial.print(*((const char *)p));
  }
  Serial.println("");
  praseMqttResponse((char *)payload);
}

PubSubClient mqttclient(MQTT_SERVER, MQTT_PORT, &mqtt_callback, wifiClient);

/* 错误返回函数 */
const __FlashStringHelper* connectErrorToString(int8_t code)
{
  switch (code)
  {
  case 1: return F("The Server does not support...");
  case 2: return F("The Client identifier...");
  case 3: return F("The MQTT service ...");
  case 4: return F("The data in the user...");
  case 5: return F("Not authorized to connect");
  case 6: return F("Exceeded ...");
  case 7: return F("You huve...");
  case -1: return F("Connection failed");
  case -2: return F("Failed to subscribe");
  case -3: return F("Connection");
  case -4: return F("Connection Timeout");
  
  default:  return F("Unkown error");
  }
}

/* MQTT 连接函数 */
void connectToMqtt(void) {
  if(mqttclient.connected()) {
    return;
  }

  Serial.print(F("connecting to MQTT..."));

  uint8_t retries = 3;

  while (!mqttclient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println(connectErrorToString(mqttclient.state()));
    Serial.println(F("retying MQTT "));
    mqttclient.disconnect();
    delay(5000);
    retries--;
    if(retries == 0) {
      while (1);
    }
    yield();
  }
  Serial.println("successed");
  mqttclient.subscribe(TOPIC1);
}

void setup() {
  // Button_sw1.setClickHandler(esp_reset);
  // Button_sw1.setLongClickHandler(wifi_reset);
  Button_sw1.setClickHandler(menu_change);
  Button_UP.setClickHandler(key_up);
  Button_UP.setLongClickDetectedHandler(exit_set);
  Serial.begin(115200); 
  EEPROM.begin(1024); // EEPROM 初始化

  eeprom_readtftInfo();

  pinMode(LCD_BL_PIN, OUTPUT); 
  analogWrite(LCD_BL_PIN, tft_CalBL(LCD_BL_PWM));  

  /* tft 初始化 */
  tft.init();
  tft.fillScreen(TFT_BLACK);            //屏幕颜色

  /* wifi初始化 */
  eeprom_readwifiInfo();
  WiFi.begin(wificonf.stassid, wificonf.stapsw);

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  liVal(20);

  while (WiFi.status() != WL_CONNECTED) {
      loading(30);

      if(loadNum >= 194)
      {
          Web_win();
          WebConfig();
      }
  }
  delay(10);
  while(loadNum < 194) //让动画走完
  { 
      loading(1);
  }

  if(WiFi.status() == WL_CONNECTED)
  {
      Serial.print("SSID:");
      Serial.println(WiFi.SSID().c_str());
      Serial.print("PSW:");
      Serial.println(WiFi.psk().c_str());
      strcpy(wificonf.stassid,WiFi.SSID().c_str());//名称复制
      strcpy(wificonf.stapsw,WiFi.psk().c_str());//密码复制
      eeprom_writewifiInfo();
      eeprom_readwifiInfo();
  }

  // 更新城市并获取天气
  apiRequest(0);

  Serial.print("本地IP： ");
  Serial.println(WiFi.localIP());

  // NTP 启动
  Serial.println("启动UDP");
  udp.begin(localPort);
  Serial.println("等待同步...");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  weaterTime = hour(); // 天气更新标志赋值

  menu_flag |= INFO_MENU; // 状态机赋值
  button_sw1Flag = 2;     // 按键标志赋值

  tft.fillScreen(TFT_BLACK);//清屏

  WiFi.forceSleepBegin(); // wifi休眠
  Serial.println("WIFI休眠......");
  Wifi_en = 0;
}
 
void loop() {
  ESP.wdtFeed();
  // doWifiTick();
  // if (WiFi.status() == WL_CONNECTED) {
  //   connectToMqtt();
  //   mqttclient.loop();
  // }
  tft_reflash();
  Button_sw1.loop(); //按钮轮询
  Button_UP.loop();
  // display_num();
}
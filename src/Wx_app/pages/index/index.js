// index.js
// 获取应用实例
const app = getApp()
const util = require('../../utils/util')
const iot = require('../../utils/alibabacloud-iot-device-sdk.min')

Page({
  data: {
    minute: 0,
    second: 0,
    nowTime: '00:00',
    inter: '',
    showTime: false,
    connectText: "未连接",
    connectStatus: false,
    device: null,
    inputInfo: null,
    deviceShowFlag: false,    // 是否设备显示列表
    // 设备信息
    deviceState: null,
    // 提交到数据库的时间
    upServerTime: null
  },
  onLoad() {

  },

  // 开启定时器
  startInter() {
    // 定时器事件
    this.data.inter = setInterval(() => {
      this.data.second += 1
      if (this.data.second >= 60) {
        this.setData({
          second: 0,
          minute: this.data.minute + 1
        })
      }
      if(this.data.minute >= 60) {
        this.setData({
          minute: 0
        })
      }
      this.setData({
        nowTime: this.toZero(this.data.minute)+':'+this.toZero(this.data.second)
      })
    }, 1000);
  },

  /* 补0函数 */
  toZero(timeNumber){
    return timeNumber < 10 ? '0'+timeNumber : timeNumber
  },

  /* 停止定时器 */
  endInter() {
    clearInterval(this.data.inter);
    this.setData({
      minute: 0,
      second: 0,
      nowTime: '00:00'
    })
  },

  /* mqtt 连接 */
  async doConnect() {
    // 连接信息
    let device = await iot.device ({
      productKey: "iym4xOYxNVB",
      deviceName: "Wx_app",
      deviceSecret: "37ec49bbaea99c96f9d6115b5a6908bd",
      regionId: "cn-shanghai",
      brokerUrl: "wxs://iot-06z00gwmda9q68k.mqtt.iothub.aliyuncs.com",
      clean: false,
    });
    // 监听 connect 事件
    device.on('connect', () => {
      wx.showToast({
        title: '连接成功',
      })
      console.log('连接成功')
      // 订阅主题
      device.subscribe("/iym4xOYxNVB/Wx_app/user/updataStatus")
      // device.publish("/iym4xOYxNVB/Wx_app/user/updataStatus", 'hello world')
    });

    // 监听 message 事件
    device.on('message', (topic, payload) => {
      console.log(topic, payload.toString());
    })

    this.setData({
      device
    })

    console.log(device);
  },

  /* 查看同一产品下指定设备的运行状态 */
  batchGetDeviecesState() {
    // 调用云函数
    app.mpServerless.function.invoke('test', {}).then((res) =>{
      if (res.success && res.result) {
        const deviceState = JSON.parse(res.result).body.deviceStatusList.deviceStatus

        // 赋值
        this.setData({
          deviceState
        }, () => {
          console.log(this.data.deviceState);
          // 添加到数据库
          this.addDateInfo()
        })
      }
    }).catch(console.error);
  },

  /* 在云数据库中添加数据 */
  addDateInfo() {
    var update = []

    this.data.deviceState.forEach(dateTemp => {
      var dictTemp = {
        deviceName: dateTemp.deviceName,
        status: dateTemp.status,
        lastOnLineTime: dateTemp.lastOnlineTime
      }
      update.push(dictTemp)
    });

    var upServerTime = util.formatTime(new Date())

    // 上传数据库
    app.mpServerless.db.collection('deviceInfo').insertOne({
      upDateTime: upServerTime,
      date: update
    })
    .then(res => {})
    .catch(console.error)

    this.setData({
      upServerTime
    }, () => {
      console.log(this.data.upServerTime)
      this.readDateInfo()
    })
  },

  /* 从云数据库中读数据 */
  readDateInfo() {
    app.mpServerless.db.collection('deviceInfo').findOne({
      // upDateTime: {$gt: this.data.upServerTime}
    }, {
      projection: {upDateTime:0}
    })
    .then(res => {
      console.log(res);
    })
    .catch(console.error)
  },

  // 发布消息
  sendPublish() {
    this.data.device.publish("/iym4xOYxNVB/Wx_app/user/updataStatus", inputInfo)
  },

  // 按钮点击事件
  switchFlag(e) {
    var connectText = ''
    var connectStatus = null
    var showTime = null
    var deviceShowFlag = null

    if (e.detail.value) {
      connectText = '已连接'
      connectStatus = true
      showTime = true
      deviceShowFlag = true
      // 连接Mqtt
      this.doConnect()
      this.startInter()
      
      // 调用api
      this.batchGetDeviecesState()
    } else {
      connectText = '未连接'
      connectStatus = false
      showTime = false 
      wx.showToast({
        title: '断开连接',
        icon: 'none'
      })
      console.log('断开连接')
      this.endInter()

      // 断开连接
      this.data.device.end(); 
    }

    this.setData({
      connectText,
      connectStatus,
      showTime,
      deviceShowFlag
    })
  },

  // 跳转页面
  device_page() {
    wx.navigateTo({
      url: '/pages/device/device',
    })
  }
})

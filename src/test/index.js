const Iot20180120 = require('@alicloud/iot20180120');
const OpenApi = require('@alicloud/openapi-client');
const Util = require('@alicloud/tea-util');

module.exports = async (event, context) => {
  const accessKeyId = 'LTAI5tEwzrp956rhcAuDZYwy';
  const accessKeySecret = '0IHbDcb5UZWJhTIKIfrkSbGmswNLA0';

  const client = createClient(accessKeyId, accessKeySecret);
  const batchGetDeviceStateRequest = new Iot20180120.BatchGetDeviceStateRequest({
    iotInstanceId: "iot-06z00gwmda9q68k",
    productKey: "iym4xOYxNVB",
    deviceName: [
      "MQTT_Device",
      "ESP82661",
      "Wx_app"
    ],
  });
  const runtime = new Util.RuntimeOptions({});

  try {
    const response = await client.batchGetDeviceStateWithOptions(batchGetDeviceStateRequest, runtime);
    // 返回 API 的响应
    return JSON.stringify(response);
  } catch (error) {
    // 返回错误信息
    return error.message;
  }
};

function createClient(accessKeyId, accessKeySecret) {
  const config = new OpenApi.Config({
    accessKeyId: accessKeyId,
    accessKeySecret: accessKeySecret,
  });
  config.endpoint = 'iot.cn-shanghai.aliyuncs.com';
  return new Iot20180120.default(config);
}

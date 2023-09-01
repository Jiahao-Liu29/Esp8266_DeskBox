const Iot20180120 = require('@alicloud/iot20180120');
const OpenApi = require('@alicloud/openapi-client');
const Util = require('@alicloud/tea-util');

async function test() {
  // 请确保代码运行环境设置了环境变量 ALIBABA_CLOUD_ACCESS_KEY_ID 和 ALIBABA_CLOUD_ACCESS_KEY_SECRET。
  // 工程代码泄露可能会导致 AccessKey 泄露，并威胁账号下所有资源的安全性。以下代码示例使用环境变量获取 AccessKey 的方式进行调用，仅供参考，建议使用更安全的 STS 方式，更多鉴权访问方式请参见：https://help.aliyun.com/document_detail/378664.html
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
    // 打印 API 的返回值
    console.log('API response:', JSON.stringify(response));
  } catch (error) {
    // 打印错误信息
    console.error(error.message);
  }
};

function createClient(accessKeyId, accessKeySecret) {
  const config = new OpenApi.Config({
    accessKeyId: accessKeyId,
    accessKeySecret: accessKeySecret,
  });
  config.endpoint = 'iot.cn-shanghai.aliyuncs.com';
  return new Iot20180120.default(config);
};

test();

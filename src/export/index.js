const OpenApi = require('@alicloud/openapi-client');
const OpenApiUtil = require('@alicloud/openapi-util');
const Util = require('@alicloud/tea-util');
const tea = require('@alicloud/tea-typescript');

module.exports.test = async (event, context) => {
  const accessKeyId = 'LTAI5tEwzrp956rhcAuDZYwy';
  const accessKeySecret = '0IHbDcb5UZWJhTIKIfrkSbGmswNLA0';

  const client = createClient(accessKeyId, accessKeySecret);
  const params = createApiInfo();

  // 设置请求参数
  const queries = {
    IotInstanceId: 'iot-06z00gwmda9q68k',
    ProductKey: 'iym4xOYxNVB',
    DeviceName: ['MQTT_Device', 'ESP82661', 'Wx_app'],
  };

  const body = {
    ApiProduct: null,
    ApiRevision: null,
  };

  const runtime = new Util.RuntimeOptions({});

  const request = new OpenApi.OpenApiRequest({
    query: OpenApiUtil.query(queries),
    body: body,
  });

  const response = await client.callApi(params, request, runtime);

  return response;
};

function createClient(accessKeyId, accessKeySecret) {
  const config = new OpenApi.Config({
    accessKeyId: accessKeyId,
    accessKeySecret: accessKeySecret,
  });

  config.endpoint = 'iot.cn-shanghai.aliyuncs.com';

  return new OpenApi(config);
}

function createApiInfo() {
  const params = new OpenApi.Params({
    action: 'BatchGetDeviceState',
    version: '2018-01-20',
    protocol: 'HTTPS',
    method: 'POST',
    authType: 'AK',
    style: 'RPC',
    pathname: '/',
    reqBodyType: 'formData',
    bodyType: 'json',
  });

  return params;
}

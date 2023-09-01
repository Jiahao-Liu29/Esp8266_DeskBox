// This file is auto-generated, don't edit it
// 依赖的模块可通过下载工程中的模块依赖文件或右上角的获取 SDK 依赖信息查看
import Iot20180120, * as $Iot20180120 from '@alicloud/iot20180120';
import OpenApi, * as $OpenApi from '@alicloud/openapi-client';
import Console from '@alicloud/tea-console';
import Util, * as $Util from '@alicloud/tea-util';
import * as $tea from '@alicloud/tea-typescript';


export default class Client {

  /**
   * 使用AK&SK初始化账号Client
   * @param accessKeyId
   * @param accessKeySecret
   * @return Client
   * @throws Exception
   */
  static createClient(accessKeyId: string, accessKeySecret: string): Iot20180120 {
    let config = new $OpenApi.Config({
      // 必填，您的 AccessKey ID
      accessKeyId: accessKeyId,
      // 必填，您的 AccessKey Secret
      accessKeySecret: accessKeySecret,
    });
    // 访问的域名
    config.endpoint = `iot.cn-shanghai.aliyuncs.com`;
    return new Iot20180120(config);
  }

  /**
  * 使用STS鉴权方式初始化账号Client，推荐此方式。
  * @param accessKeyId
  * @param accessKeySecret
  * @param securityToken
  * @return Client
  * @throws Exception
  */
  static createClientWithSTS(accessKeyId: string, accessKeySecret: string, securityToken: string): Iot20180120 {
    let config = new $OpenApi.Config({
      // 必填，您的 AccessKey ID
      accessKeyId: accessKeyId,
      // 必填，您的 AccessKey Secret
      accessKeySecret: accessKeySecret,
      // 必填，您的 Security Token
      securityToken: securityToken,
      // 必填，表明使用 STS 方式
      type: "sts",
    });
    // 访问的域名
    config.endpoint = `iot.cn-shanghai.aliyuncs.com`;
    return new Iot20180120(config);
  }

  static async main(args: string[]): Promise<String> {
    // 请确保代码运行环境设置了环境变量 ALIBABA_CLOUD_ACCESS_KEY_ID 和 ALIBABA_CLOUD_ACCESS_KEY_SECRET。
    // 工程代码泄露可能会导致 AccessKey 泄露，并威胁账号下所有资源的安全性。以下代码示例仅供参考，建议使用更安全的 STS 方式，更多鉴权访问方式请参见：https://help.aliyun.com/document_detail/378664.html
    let client = Client.createClient('LTAI5tEwzrp956rhcAuDZYwy', '0IHbDcb5UZWJhTIKIfrkSbGmswNLA0');
    let batchGetDeviceStateRequest = new $Iot20180120.BatchGetDeviceStateRequest({
      iotInstanceId: "iot-06z00gwmda9q68k",
      productKey: "iym4xOYxNVB",
      // Array, 可选, 要查看运行状态的设备的名称列表。 ><notice> 如果传入该参数，需同时传入**ProductKey**。 ></notice> 
      deviceName: [
        "Wx_app",
        "MQTT_Device",
        "ESP82661"
      ],
    });
    let runtime = new $Util.RuntimeOptions({ });
    let resp = await client.batchGetDeviceStateWithOptions(batchGetDeviceStateRequest, runtime);
    let json = Util.toJSONString(resp)
    return json;
  }

}

Client.main(process.argv.slice(2));
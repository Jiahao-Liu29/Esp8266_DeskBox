"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (g && (g = 0, op[0] && (_ = 0)), _) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
Object.defineProperty(exports, "__esModule", { value: true });
// This file is auto-generated, don't edit it
// 依赖的模块可通过下载工程中的模块依赖文件或右上角的获取 SDK 依赖信息查看
var iot20180120_1 = require("@alicloud/iot20180120"), $Iot20180120 = iot20180120_1;
var $OpenApi = require("@alicloud/openapi-client");
var tea_util_1 = require("@alicloud/tea-util"), $Util = tea_util_1;
var Client = /** @class */ (function () {
    function Client() {
    }
    /**
     * 使用AK&SK初始化账号Client
     * @param accessKeyId
     * @param accessKeySecret
     * @return Client
     * @throws Exception
     */
    Client.createClient = function (accessKeyId, accessKeySecret) {
        var config = new $OpenApi.Config({
            // 必填，您的 AccessKey ID
            accessKeyId: accessKeyId,
            // 必填，您的 AccessKey Secret
            accessKeySecret: accessKeySecret,
        });
        // 访问的域名
        config.endpoint = "iot.cn-shanghai.aliyuncs.com";
        return new iot20180120_1.default(config);
    };
    /**
    * 使用STS鉴权方式初始化账号Client，推荐此方式。
    * @param accessKeyId
    * @param accessKeySecret
    * @param securityToken
    * @return Client
    * @throws Exception
    */
    Client.createClientWithSTS = function (accessKeyId, accessKeySecret, securityToken) {
        var config = new $OpenApi.Config({
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
        config.endpoint = "iot.cn-shanghai.aliyuncs.com";
        return new iot20180120_1.default(config);
    };
    Client.main = function (args) {
        return __awaiter(this, void 0, void 0, function () {
            var client, batchGetDeviceStateRequest, runtime, resp, json;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        client = Client.createClient('LTAI5tEwzrp956rhcAuDZYwy', '0IHbDcb5UZWJhTIKIfrkSbGmswNLA0');
                        batchGetDeviceStateRequest = new $Iot20180120.BatchGetDeviceStateRequest({
                            iotInstanceId: "iot-06z00gwmda9q68k",
                            productKey: "iym4xOYxNVB",
                            // Array, 可选, 要查看运行状态的设备的名称列表。 ><notice> 如果传入该参数，需同时传入**ProductKey**。 ></notice> 
                            deviceName: [
                                "Wx_app",
                                "MQTT_Device",
                                "ESP82661"
                            ],
                        });
                        runtime = new $Util.RuntimeOptions({});
                        return [4 /*yield*/, client.batchGetDeviceStateWithOptions(batchGetDeviceStateRequest, runtime)];
                    case 1:
                        resp = _a.sent();
                        json = tea_util_1.default.toJSONString(resp);
                        return [2 /*return*/, json];
                }
            });
        });
    };
    return Client;
}());
exports.default = Client;
Client.main(process.argv.slice(2));

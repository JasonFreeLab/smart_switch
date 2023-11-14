/*
 * JasonFreeLab
 *
 */
#include "iot_export_linkkit.h"
#include "cJSON.h"

#include "switch.h"
#include "led.h"

#include "esp_log.h"

static const char* TAG = "linkkit_switch_solo";

void HAL_Printf(const char *fmt, ...);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);

#define LOG_TRACE(...)                                          \
    do {                                                            \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__);     \
        HAL_Printf(__VA_ARGS__);                                    \
        HAL_Printf("\033[0m\r\n");                                  \
    } while (0)

#define Switch_MASTER_DEVID            (0)
#define Switch_YIELD_TIMEOUT_MS        (200)

typedef struct {
    int master_devid;
    int cloud_connected;
    int master_initialized;
} user_switch_ctx_t;

char PRODUCT_KEY[PRODUCT_KEY_MAXLEN] = {0};
char PRODUCT_SECRET[PRODUCT_SECRET_MAXLEN] = {0};
char DEVICE_NAME[DEVICE_NAME_MAXLEN] = {0};
char DEVICE_SECRET[DEVICE_SECRET_MAXLEN] = {0};

static user_switch_ctx_t g_user_Switch_ctx;

/** Awss Status event callback 智能配网状态事件回调函数*/
static int user_awss_status_event_handler(int status)
{
    LOG_TRACE("Awss Status %d", status);

    return SUCCESS_RETURN;
}

/** cloud connected event callback 连接成功事件回调函数*/
static int user_connected_event_handler(void)
{
    LOG_TRACE("Cloud Connected");
    g_user_Switch_ctx.cloud_connected = 1;

    return 0;
}

/** cloud connect fail event callback 连接失败事件回调函数*/
static int user_connect_fail_event_handler(void) 
{
    LOG_TRACE("Cloud Connect Fail");

    return SUCCESS_RETURN;
}

/** cloud disconnected event callback 断开连接事件回调函数*/
static int user_disconnected_event_handler(void)
{
    LOG_TRACE("Cloud Disconnected");
    g_user_Switch_ctx.cloud_connected = 0;

    return 0;
}

/** cloud raw_data arrived event callback 数据送达事件回调函数*/
static int user_rawdata_arrived_event_handler(const int devid, const unsigned char *request, const int request_len)
{
    LOG_TRACE("Cloud Rawdata Arrived");

    return 0;
}

/* device initialized event callback 设备完成初始化事件回调函数*/
static int user_initialized(const int devid)
{
    LOG_TRACE("Device Initialized");
    g_user_Switch_ctx.master_initialized = 1;

    return 0;
}

/** recv property post response message from cloud 来自云端的接收属性的发布响应消息**/
static int user_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
        const int reply_len)
{
    LOG_TRACE("Message Post Reply Received, Message ID: %d, Code: %d, Reply: %.*s", msgid, code,
                  reply_len,
                  (reply == NULL)? ("NULL") : (reply));
    return 0;
}

/** recv event post response message from cloud 来自云端的接收事件的发布响应消息**/
static int user_trigger_event_reply_event_handler(const int devid, const int msgid, const int code, const char *eventid,
        const int eventid_len, const char *message, const int message_len)
{
    LOG_TRACE("Trigger Event Reply Received, Message ID: %d, Code: %d, EventID: %.*s, Message: %.*s",
                  msgid, code,
                  eventid_len,
                  eventid, message_len, message);

    return 0;
}

//属性设置事件回调函数
static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    cJSON *root = NULL, *PowerSwitch = NULL;
    ESP_LOGI(TAG,"Property Set Received, Devid: %d, Request: %s", devid, request);
    
    if (!request) {
        return NULL_VALUE_ERROR;
    }

    /* Parse Root 获取解析json根句柄*/
    root = cJSON_Parse(request);
    if (!root) {
        ESP_LOGI(TAG,"JSON Parse Error");
        return FAIL_RETURN;
    }

    /** Switch switch/led On/Off   */
    PowerSwitch = cJSON_GetObjectItem(root, "powerstate");
    if (PowerSwitch) {
        switch_set_on_off(PowerSwitch->valueint);
        led_set_on_off(PowerSwitch->valueint);
    }
    
    cJSON_Delete(root);

    res = IOT_Linkkit_Report(Switch_MASTER_DEVID, ITM_MSG_POST_PROPERTY,
                             (unsigned char *)request, request_len);
    ESP_LOGI(TAG,"Post Property Message ID: %d", res);

    return SUCCESS_RETURN;
}

//获取属性事件回调函数
static int user_property_get_event_handler(const int devid, const char *request, const int request_len, char **response,
                                           int *response_len)
{
    cJSON *request_root = NULL, *item_propertyid = NULL;
    cJSON *response_root = NULL;
    int index = 0;
    LOG_TRACE("Property Get Received, Devid: %d, Request: %s", devid, request);

    /* Parse Request 获取解析json根句柄*/
    request_root = cJSON_Parse(request);
    if (request_root == NULL || !cJSON_IsArray(request_root)) {
        LOG_TRACE("JSON Parse Error");
        return -1;
    }

    /* Prepare Response 准备响应*/
    response_root = cJSON_CreateObject();
    if (response_root == NULL) {
        LOG_TRACE("No Enough Memory");
        cJSON_Delete(request_root);
        return -1;
    }

    for (index = 0; index < cJSON_GetArraySize(request_root); index++) {
        item_propertyid = cJSON_GetArrayItem(request_root, index);
        if (item_propertyid == NULL || !cJSON_IsString(item_propertyid)) {
            LOG_TRACE("JSON Parse Error");
            cJSON_Delete(request_root);
            cJSON_Delete(response_root);
            return -1;
        }

        LOG_TRACE("Property ID, index: %d, Value: %s", index, item_propertyid->valuestring);

        if (strcmp("WIFI_Tx_Rate", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "WIFI_Tx_Rate", 1111);
        } else if (strcmp("WIFI_Rx_Rate", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "WIFI_Rx_Rate", 2222);
        } else if (strcmp("powerstate", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "powerstate", 1);
        } else if (strcmp("DeviceTimer", item_propertyid->valuestring) == 0) {
            cJSON *array_localtimer = cJSON_CreateArray();
            if (array_localtimer == NULL) {
                cJSON_Delete(request_root);
                cJSON_Delete(response_root);
                return -1;
            }

            cJSON *item_localtimer = cJSON_CreateObject();
            if (item_localtimer == NULL) {
                cJSON_Delete(request_root);
                cJSON_Delete(response_root);
                cJSON_Delete(array_localtimer);
                return -1;
            }
            cJSON_AddStringToObject(item_localtimer, "Timer", "10 11 * * * 1 2 3 4 5");
            cJSON_AddNumberToObject(item_localtimer, "Enable", 1);
            cJSON_AddNumberToObject(item_localtimer, "IsValid", 1);
            cJSON_AddItemToArray(array_localtimer, item_localtimer);
            cJSON_AddItemToObject(response_root, "DeviceTimer", array_localtimer);
        }
    }
    cJSON_Delete(request_root);

    *response = cJSON_PrintUnformatted(response_root);
    if (*response == NULL) {
        LOG_TRACE("No Enough Memory");
        cJSON_Delete(response_root);
        return -1;
    }
    cJSON_Delete(response_root);
    *response_len = strlen(*response);

    LOG_TRACE("Property Get Response: %s", *response);

    return SUCCESS_RETURN;
}

//服务器请求事件回调函数
static int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
        const char *request, const int request_len,
        char **response, int *response_len)
{
    int contrastratio = 0, to_cloud = 0;
    cJSON *root = NULL, *item_transparency = NULL, *item_from_cloud = NULL;
    ESP_LOGI(TAG,"Service Request Received, Devid: %d, Service ID: %.*s, Payload: %s", devid, serviceid_len, serviceid, request);

    /* Parse Root 获取解析json根句柄*/
    root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root)) {
        ESP_LOGI(TAG,"JSON Parse Error");
        return -1;
    }

    if (strlen("Custom") == serviceid_len && memcmp("Custom", serviceid, serviceid_len) == 0) {
        /* Parse Item 解析项目*/
        const char *response_fmt = "{\"Contrastratio\":%d}";
        item_transparency = cJSON_GetObjectItem(root, "transparency");
        if (item_transparency == NULL || !cJSON_IsNumber(item_transparency)) {
            cJSON_Delete(root);
            return -1;
        }
        ESP_LOGI(TAG,"transparency: %d", item_transparency->valueint);
        contrastratio = item_transparency->valueint + 1;

        /* Send Service Response To Cloud 发送服务应答到云端*/
        *response_len = strlen(response_fmt) + 10 + 1;
        *response = malloc(*response_len);
        if (*response == NULL) {
            ESP_LOGW(TAG,"Memory Not Enough");
            return -1;
        }
        memset(*response, 0, *response_len);
        snprintf(*response, *response_len, response_fmt, contrastratio);
        *response_len = strlen(*response);
    } else if (strlen("SyncService") == serviceid_len && memcmp("SyncService", serviceid, serviceid_len) == 0) {
        /* Parse Item 解析项目*/
        const char *response_fmt = "{\"ToCloud\":%d}";
        item_from_cloud = cJSON_GetObjectItem(root, "FromCloud");
        if (item_from_cloud == NULL || !cJSON_IsNumber(item_from_cloud)) {
            cJSON_Delete(root);
            return -1;
        }
        ESP_LOGI(TAG,"FromCloud: %d", item_from_cloud->valueint);
        to_cloud = item_from_cloud->valueint + 1;

        /* Send Service Response To Cloud 发送服务应答到云端*/
        *response_len = strlen(response_fmt) + 10 + 1;
        *response = malloc(*response_len);
        if (*response == NULL) {
            ESP_LOGW(TAG,"Memory Not Enough");
            return -1;
        }
        memset(*response, 0, *response_len);
        snprintf(*response, *response_len, response_fmt, to_cloud);
        *response_len = strlen(*response);
    }
    cJSON_Delete(root);

    return 0;
}

//时间戳回复事件回调函数
static int user_timestamp_reply_event_handler(const char *timestamp)
{
    LOG_TRACE("Current Timestamp: %s", timestamp);

    return SUCCESS_RETURN;
}

//拓扑列表回复事件回调函数
static int user_topolist_reply_handler(const int devid, const int id, const int code, const char *payload, const int payload_len)
{
    LOG_TRACE("ITE_TOPOLIST_REPLY");

    return SUCCESS_RETURN;
}

//加入许可事件回调函数
static int user_permit_join_event_handler(const char *product_key, const int time)
{
    LOG_TRACE("ITE_PERMIT_JOIN");
    
    return SUCCESS_RETURN;
}

/** fota event handler fota事件回调函数**/
static int user_fota_event_handler(int type, const char *version)
{
    char buffer[1025 + 1] = {0};
    int buffer_length = 1025; //must set want read len to len + 1

    /* 0 - new firmware exist, query the new firmware */
    if (type == 0) {
        LOG_TRACE("New Firmware Version: %s", version);

        if (IOT_Linkkit_Query(Switch_MASTER_DEVID, ITM_MSG_QUERY_FOTA_DATA, (unsigned char *)buffer, buffer_length) == SUCCESS_RETURN) {
            HAL_Reboot();
        }
    }

    return 0;
}

/* cota event handler cota事件回调函数*/
static int user_cota_event_handler(int type, const char *config_id, int config_size, const char *get_type,
                                   const char *sign, const char *sign_method, const char *url)
{
    char buffer[128] = {0};
    int buffer_length = 128;

    /* type = 0, new config exist, query the new config */
    if (type == 0) {
        LOG_TRACE("New Config ID: %s", config_id);
        LOG_TRACE("New Config Size: %d", config_size);
        LOG_TRACE("New Config Type: %s", get_type);
        LOG_TRACE("New Config Sign: %s", sign);
        LOG_TRACE("New Config Sign Method: %s", sign_method);
        LOG_TRACE("New Config URL: %s", url);

        IOT_Linkkit_Query(Switch_MASTER_DEVID, ITM_MSG_QUERY_COTA_DATA, (unsigned char *)buffer, buffer_length);
    }

    return 0;
}

//mqtt连接成功事件回调函数
static int user_mqtt_connect_succ_event_handler(void)
{
    LOG_TRACE("ITE_MQTT_CONNECT_SUCC");
    
    return SUCCESS_RETURN;
}

//事件通知回调函数
static int user_event_notify_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    LOG_TRACE("Event notify Received, Devid: %d, Request: %s", devid, request);

    res = IOT_Linkkit_Report(Switch_MASTER_DEVID, ITM_MSG_EVENT_NOTIFY_REPLY,
                             (unsigned char *)request, request_len);
    LOG_TRACE("Post Property Message ID: %d", res);

    return 0;
}

//linkkit线程
static int linkkit_thread(void *paras)
{
    int res = 0;
    iotx_linkkit_dev_meta_info_t master_meta_info;
    int domain_type = 0, dynamic_register = 0, post_reply_need = 0;

#ifdef ATM_ENABLED
    if (IOT_ATM_Init() < 0) {
        LOG_TRACE("IOT ATM init failed!\n");
        return -1;
    }
#endif

    memset(&g_user_Switch_ctx, 0, sizeof(user_switch_ctx_t));

    HAL_GetProductKey(PRODUCT_KEY);
    HAL_GetProductSecret(PRODUCT_SECRET);
    HAL_GetDeviceName(DEVICE_NAME);
    HAL_GetDeviceSecret(DEVICE_SECRET);
    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
    memcpy(master_meta_info.product_key, PRODUCT_KEY, strlen(PRODUCT_KEY));
    memcpy(master_meta_info.product_secret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
    memcpy(master_meta_info.device_name, DEVICE_NAME, strlen(DEVICE_NAME));
    memcpy(master_meta_info.device_secret, DEVICE_SECRET, strlen(DEVICE_SECRET));

    /* Register Callback */
    IOT_RegisterCallback(ITE_AWSS_STATUS, user_awss_status_event_handler);          //注册智能配网状态事件回调函数
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);           //注册连接成功事件回调函数
    IOT_RegisterCallback(ITE_CONNECT_FAIL, user_connect_fail_event_handler);        //注册连接失败事件回调函数
    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);        //注册断开连接事件回调函数
    IOT_RegisterCallback(ITE_RAWDATA_ARRIVED, user_rawdata_arrived_event_handler);  //注册数据送达事件回调函数
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);  //注册服务器请求事件回调函数
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);        //注册属性设置事件回调函数
    /*Only for local communication service(ALCS)*/
    IOT_RegisterCallback(ITE_PROPERTY_GET, user_property_get_event_handler);        //注册获取属性事件回调函数
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);        //注册报告回复事件回调函数
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, user_trigger_event_reply_event_handler);  //注册事件触发回复事件回调函数
    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, user_timestamp_reply_event_handler);  //注册时间戳回复事件回调函数
    IOT_RegisterCallback(ITE_TOPOLIST_REPLY, user_topolist_reply_handler);          //注册拓扑列表回复事件回调函数
    IOT_RegisterCallback(ITE_PERMIT_JOIN, user_permit_join_event_handler);          //注册加入许可事件回调函数
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);               //注册初始化事件回调函数
    IOT_RegisterCallback(ITE_FOTA, user_fota_event_handler);                        //注册fota事件回调函数
    IOT_RegisterCallback(ITE_COTA, user_cota_event_handler);                        //注册cota事件回调函数
    IOT_RegisterCallback(ITE_MQTT_CONNECT_SUCC, user_mqtt_connect_succ_event_handler);  //注册mqtt连接成功事件回调函数
    IOT_RegisterCallback(ITE_EVENT_NOTIFY, user_event_notify_handler);              //注册事件通知回调函数

    domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);

    /* Choose Login Method */
    dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    /* post reply doesn't need */
    post_reply_need = 1;
    IOT_Ioctl(IOTX_IOCTL_RECV_EVENT_REPLY, (void *)&post_reply_need);

    /* Create Master Device Resources */
    g_user_Switch_ctx.master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
    if (g_user_Switch_ctx.master_devid < 0) {
        LOG_TRACE("IOT_Linkkit_Open Failed\n");
        return -1;
    }

    /* Start Connect Aliyun Server */
    res = IOT_Linkkit_Connect(g_user_Switch_ctx.master_devid);
    if (res < 0) {
        LOG_TRACE("IOT_Linkkit_Connect Failed\n");
        IOT_Linkkit_Close(g_user_Switch_ctx.master_devid);
        return -1;
    }

    while (1) {
        IOT_Linkkit_Yield(Switch_YIELD_TIMEOUT_MS);
    }

    IOT_Linkkit_Close(g_user_Switch_ctx.master_devid);

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);
    return 0;
}

//linkkit_main任务
void linkkit_main(void *paras)
{
    while (1) {
        linkkit_thread(NULL);
    }
}

/******************************************************************************
 * File Name : main_response_only_example.c
 * Function  : 响应JSON库使用示例
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#include "cjson_response.h"

#include <stdbool.h>
#include <stdio.h>

    typedef struct {
    int vol;
    int cap;
} VoltageInfo_S;

typedef struct {
    int num1;
    int num2;
    char version[64];
    bool flags;
} Test_S;

CJSON_REGISTER_OBJECT_TYPE(VoltageInfo_S, vol, cap);
CJSON_REGISTER_OBJECT_TYPE(Test_S, num1, num2, version, flags);

int main(void)
{
    VoltageInfo_S voltageInfo = {300, 1200};
    VoltageInfo_S parsedVoltageInfo = {0};
    Test_S testInfo = {42, 84, "v1.0.0", true};
    Test_S parsedTestInfo = {0};
    CJSONResponse_S response = {0};
    char dataString[256] = {0};
    char* emptyResponse = NULL;
    char* stringResponse = NULL;
    char* objectResponse = NULL;
    char* objectResponse1 = NULL;

    emptyResponse = CJSON_RESPONSE_BUILD_EMPTY(CJSON_RESULT_FAIL);
    stringResponse = CJSON_RESPONSE_BUILD_STRING(CJSON_RESULT_PASS, "SN123456");
    objectResponse = CJSON_RESPONSE_BUILD_OBJECT(CJSON_RESULT_PASS, &voltageInfo, VoltageInfo_S);
    objectResponse1 = CJSON_RESPONSE_BUILD_OBJECT(CJSON_RESULT_PASS, &testInfo, Test_S);

    printf("%s\n", emptyResponse);
    printf("%s\n", stringResponse);
    printf("%s\n", objectResponse);
    printf("%s\n", objectResponse1);

    response.dataString = dataString;
    if (CJSON_ParseResponse(stringResponse, &response, NULL, 0U, NULL) == CJSON_BOOL_TRUE) {
        printf("parsed result=%d data=%s\n", response.result, response.dataString);
    }

    if (CJSON_PARSE_RESPONSE_OBJECT(objectResponse, &response, &parsedVoltageInfo, sizeof(parsedVoltageInfo), VoltageInfo_S) == CJSON_BOOL_TRUE) {
        printf("parsed vol=%d cap=%d\n", parsedVoltageInfo.vol, parsedVoltageInfo.cap);
    }

    if (CJSON_PARSE_RESPONSE_OBJECT(objectResponse1, &response, &parsedTestInfo, sizeof(parsedTestInfo), Test_S) == CJSON_BOOL_TRUE) {
        printf("parsed num1=%d num2=%d version=%s flags=%d\n", parsedTestInfo.num1, parsedTestInfo.num2, parsedTestInfo.version, parsedTestInfo.flags);
    }

    CJSON_Log(CJSON_LOG_LEVEL_INFO, "factory_test", "read voltage success");

    CJSON_Free(emptyResponse);
    CJSON_Free(stringResponse);
    CJSON_Free(objectResponse);
    CJSON_Free(objectResponse1);

    return 0;
}

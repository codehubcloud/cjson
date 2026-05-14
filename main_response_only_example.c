/******************************************************************************
 * File Name : main_response_only_example.c
 * Function  : 响应JSON库使用示例
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#include "cjson_response.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef struct {
    int vol;
    int cap;
    u8 channel;
    u16 temperature;
    u32 count;
    u64 timestamp;
    char version[64];
    bool flag;
    double ratio;
    char grade;
} VoltageInfo_S;

typedef struct {
    s8 s8Value;
    s16 s16Value;
    s32 s32Value;
    s64 s64Value;
    u8 u8Value;
    u16 u16Value;
    u32 u32Value;
    u64 u64Value;
    bool boolTrueValue;
    bool boolFalseValue;
    f32 floatValue;
    f64 doubleValue;
    char charValue;
} BasicTypeInfo_S;

typedef struct {
    char emptyResponse[256];
    char stringResponse[256];
    char objectResponse[1024];
    char intResponse[256];
    char uintResponse[256];
    char boolResponse[256];
    char doubleResponse[256];
    char charResponse[256];
    char s8Response[256];
    char s16Response[256];
    char s32Response[256];
    char s64Response[256];
    char u8Response[256];
    char u16Response[256];
    char u32Response[256];
    char u64Response[256];
    char boolTrueResponse[256];
    char boolFalseResponse[256];
    char floatResponse[256];
    char doubleVarResponse[256];
    char charVarResponse[256];
    char basicObjectResponse[1024];
} ResponseBuffer_S;

typedef struct {
    VoltageInfo_S voltageInfo;
    VoltageInfo_S parsedVoltageInfo;
    BasicTypeInfo_S basicTypeInfo;
    BasicTypeInfo_S parsedBasicTypeInfo;
    char dataString[256];
} TestData_S;

typedef struct {
    ResponseBuffer_S responseBuf;
    TestData_S testData;
} TestContext_S;

#undef FT_RESULT_OBJECT_LIST
#define FT_RESULT_OBJECT_LIST(X)                                                                                                                             \
    X(VoltageInfo_S, vol, cap, channel, temperature, count, timestamp, version, flag, ratio, grade)                                                          \
    X(BasicTypeInfo_S, s8Value, s16Value, s32Value, s64Value, u8Value, u16Value, u32Value, u64Value, boolTrueValue, boolFalseValue, floatValue, doubleValue, \
      charValue)

FT_RESULT_REGISTER_OBJECTS()

/******************************************************************************
 * @brief      : 打印JSON字符串
 * @param[in]  : title --打印标题, jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 统一格式打印构建后的JSON字符串
 ******************************************************************************/
static void PrintJson(const char *title, const char *jsonString)
{
    printf("%-28s: %s\n", title, jsonString);
}

/******************************************************************************
 * @brief      : 解析并打印有符号整数响应
 * @param[in]  : title --打印标题, jsonString --待解析JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于验证DATA_INT类型响应解析结果
 ******************************************************************************/
static void ParseAndPrintInt(const char *title, const char *jsonString)
{
    CJSONResponse_S response = {0};

    if (FT_ResultParse(DATA_INT, jsonString, &response, NULL) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d value=%lld\n", title, response.result, response.dataType, (long long)response.intValue);
    } else {
        printf("%-28s: parse failed\n", title);
    }
}

/******************************************************************************
 * @brief      : 解析并打印无符号整数响应
 * @param[in]  : title --打印标题, jsonString --待解析JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于验证DATA_UINT类型响应解析结果
 ******************************************************************************/
static void ParseAndPrintUInt(const char *title, const char *jsonString)
{
    CJSONResponse_S response = {0};

    if (FT_ResultParse(DATA_UINT, jsonString, &response, NULL) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d value=%llu\n", title, response.result, response.dataType, (unsigned long long)response.uintValue);
    } else {
        printf("%-28s: parse failed\n", title);
    }
}

/******************************************************************************
 * @brief      : 解析并打印布尔响应
 * @param[in]  : title --打印标题, jsonString --待解析JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于验证DATA_BOOL类型响应解析结果
 ******************************************************************************/
static void ParseAndPrintBool(const char *title, const char *jsonString)
{
    CJSONResponse_S response = {0};

    if (FT_ResultParse(DATA_BOOL, jsonString, &response, NULL) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d value=%d\n", title, response.result, response.dataType, response.boolValue);
    } else {
        printf("%-28s: parse failed\n", title);
    }
}

/******************************************************************************
 * @brief      : 解析并打印浮点响应
 * @param[in]  : title --打印标题, jsonString --待解析JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于验证DATA_DOUBLE类型响应解析结果，float也按DATA_DOUBLE解析
 ******************************************************************************/
static void ParseAndPrintDouble(const char *title, const char *jsonString)
{
    CJSONResponse_S response = {0};

    if (FT_ResultParse(DATA_DOUBLE, jsonString, &response, NULL) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d value=%Lf\n", title, response.result, response.dataType, response.doubleValue);
    } else {
        printf("%-28s: parse failed\n", title);
    }
}

/******************************************************************************
 * @brief      : 解析并打印字符响应
 * @param[in]  : title --打印标题, jsonString --待解析JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于验证DATA_CHAR类型响应解析结果
 ******************************************************************************/
static void ParseAndPrintChar(const char *title, const char *jsonString)
{
    CJSONResponse_S response = {0};

    if (FT_ResultParse(DATA_CHAR, jsonString, &response, NULL) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d value=%c\n", title, response.result, response.dataType, response.charValue);
    } else {
        printf("%-28s: parse failed\n", title);
    }
}

/******************************************************************************
 * @brief      : 初始化测试上下文
 * @param[in]  : 无
 * @param[out] : context --测试上下文
 * @return     : 无
 * @note       : 初始化原始对象数据和基础类型对象数据
 ******************************************************************************/
static void InitTestContext(TestContext_S *context)
{
    if (context == NULL) {
        return;
    }

    *context = (TestContext_S){0};

    context->testData.voltageInfo = (VoltageInfo_S){
        300, 1200, 2U, 35U, 99U, 123456789ULL, "v1.0.0", true, 0.75, 'A',
    };

    context->testData.basicTypeInfo = (BasicTypeInfo_S){
        (s8)-12, (s16)-1234, (s32)-12345678, (s64)-123456789012345LL, (u8)12U, (u16)1234U, (u32)12345678UL, (u64)123456789012345ULL,
        true,    false,      (f32)1.25F,     (f64)3.1415926,          'Z',
    };
}

/******************************************************************************
 * @brief      : 构建原始示例响应JSON
 * @param[in]  : 无
 * @param[out] : context --测试上下文中的响应缓冲区
 * @return     : 无
 * @note       : 构建空响应、字符串响应、对象响应、整数、无符号整数、布尔、浮点和字符响应
 ******************************************************************************/
static void BuildOriginalExampleResponses(TestContext_S *context)
{
    u64 timestamp = 123456789ULL;

    if (context == NULL) {
        return;
    }

    FT_ResultPrint(DATA_EMPTY, context->responseBuf.emptyResponse, sizeof(context->responseBuf.emptyResponse), RESULT_FAIL, NULL);
    FT_ResultPrint(DATA_STRING, context->responseBuf.stringResponse, sizeof(context->responseBuf.stringResponse), RESULT_PASS, "SN123456");
    FT_ResultPrint(DATA_OBJECT, context->responseBuf.objectResponse, sizeof(context->responseBuf.objectResponse), RESULT_PASS, &context->testData.voltageInfo);
    FT_ResultPrint(DATA_INT, context->responseBuf.intResponse, sizeof(context->responseBuf.intResponse), RESULT_PASS, 55);
    FT_ResultPrint(DATA_UINT, context->responseBuf.uintResponse, sizeof(context->responseBuf.uintResponse), RESULT_PASS, timestamp);
    FT_ResultPrint(DATA_BOOL, context->responseBuf.boolResponse, sizeof(context->responseBuf.boolResponse), RESULT_PASS, true);
    FT_ResultPrint(DATA_DOUBLE, context->responseBuf.doubleResponse, sizeof(context->responseBuf.doubleResponse), RESULT_PASS, 3.1415926);
    FT_ResultPrint(DATA_CHAR, context->responseBuf.charResponse, sizeof(context->responseBuf.charResponse), RESULT_PASS, 'A');
}

/******************************************************************************
 * @brief      : 构建所有基础类型响应JSON
 * @param[in]  : 无
 * @param[out] : context --测试上下文中的响应缓冲区
 * @return     : 无
 * @note       : 覆盖s8~s64、u8~u64、bool、float、double和char
 ******************************************************************************/
static void BuildBasicTypeResponses(TestContext_S *context)
{
    s8 s8Value = (s8)-12;
    s16 s16Value = (s16)-1234;
    s32 s32Value = (s32)-12345678;
    s64 s64Value = (s64)-123456789012345LL;
    u8 u8Value = (u8)12U;
    u16 u16Value = (u16)1234U;
    u32 u32Value = (u32)12345678UL;
    u64 u64Value = (u64)123456789012345ULL;
    bool boolTrueValue = true;
    bool boolFalseValue = false;
    f32 floatValue = (f32)1.25F;
    f64 doubleValue = (f64)3.1415926;
    char charValue = 'Z';

    if (context == NULL) {
        return;
    }

    FT_ResultPrint(DATA_INT, context->responseBuf.s8Response, sizeof(context->responseBuf.s8Response), RESULT_PASS, s8Value);
    FT_ResultPrint(DATA_INT, context->responseBuf.s16Response, sizeof(context->responseBuf.s16Response), RESULT_PASS, s16Value);
    FT_ResultPrint(DATA_INT, context->responseBuf.s32Response, sizeof(context->responseBuf.s32Response), RESULT_PASS, s32Value);
    FT_ResultPrint(DATA_INT, context->responseBuf.s64Response, sizeof(context->responseBuf.s64Response), RESULT_PASS, s64Value);

    FT_ResultPrint(DATA_UINT, context->responseBuf.u8Response, sizeof(context->responseBuf.u8Response), RESULT_PASS, u8Value);
    FT_ResultPrint(DATA_UINT, context->responseBuf.u16Response, sizeof(context->responseBuf.u16Response), RESULT_PASS, u16Value);
    FT_ResultPrint(DATA_UINT, context->responseBuf.u32Response, sizeof(context->responseBuf.u32Response), RESULT_PASS, u32Value);
    FT_ResultPrint(DATA_UINT, context->responseBuf.u64Response, sizeof(context->responseBuf.u64Response), RESULT_PASS, u64Value);

    FT_ResultPrint(DATA_BOOL, context->responseBuf.boolTrueResponse, sizeof(context->responseBuf.boolTrueResponse), RESULT_PASS, boolTrueValue);
    FT_ResultPrint(DATA_BOOL, context->responseBuf.boolFalseResponse, sizeof(context->responseBuf.boolFalseResponse), RESULT_PASS, boolFalseValue);

    FT_ResultPrint(DATA_DOUBLE, context->responseBuf.floatResponse, sizeof(context->responseBuf.floatResponse), RESULT_PASS, floatValue);
    FT_ResultPrint(DATA_DOUBLE, context->responseBuf.doubleVarResponse, sizeof(context->responseBuf.doubleVarResponse), RESULT_PASS, doubleValue);

    FT_ResultPrint(DATA_CHAR, context->responseBuf.charVarResponse, sizeof(context->responseBuf.charVarResponse), RESULT_PASS, charValue);
}

/******************************************************************************
 * @brief      : 构建包含所有基础字段的对象响应JSON
 * @param[in]  : 无
 * @param[out] : context --测试上下文中的基础类型对象响应缓冲区
 * @return     : 无
 * @note       : 用于验证对象中s8~s64、u8~u64、bool、float、double和char字段
 ******************************************************************************/
static void BuildBasicObjectResponse(TestContext_S *context)
{
    if (context == NULL) {
        return;
    }

    FT_ResultPrint(DATA_OBJECT, context->responseBuf.basicObjectResponse, sizeof(context->responseBuf.basicObjectResponse), RESULT_PASS,
                   &context->testData.basicTypeInfo);
}

/******************************************************************************
 * @brief      : 打印原始示例构建结果
 * @param[in]  : context --测试上下文
 * @param[out] : 无
 * @return     : 无
 * @note       : 打印空响应、字符串响应、对象响应和常用基础类型响应
 ******************************************************************************/
static void PrintOriginalExampleBuildResult(const TestContext_S *context)
{
    if (context == NULL) {
        return;
    }

    printf("========== build original example ==========\n");
    PrintJson("build empty response", context->responseBuf.emptyResponse);
    PrintJson("build string response", context->responseBuf.stringResponse);
    PrintJson("build object response", context->responseBuf.objectResponse);
    PrintJson("build int response", context->responseBuf.intResponse);
    PrintJson("build uint response", context->responseBuf.uintResponse);
    PrintJson("build bool response", context->responseBuf.boolResponse);
    PrintJson("build double response", context->responseBuf.doubleResponse);
    PrintJson("build char response", context->responseBuf.charResponse);
}

/******************************************************************************
 * @brief      : 打印所有基础类型构建结果
 * @param[in]  : context --测试上下文
 * @param[out] : 无
 * @return     : 无
 * @note       : 打印s8~s64、u8~u64、bool、float、double、char和基础类型对象响应
 ******************************************************************************/
static void PrintBasicTypeBuildResult(const TestContext_S *context)
{
    if (context == NULL) {
        return;
    }

    printf("\n========== build all basic types ==========\n");
    PrintJson("build s8 response", context->responseBuf.s8Response);
    PrintJson("build s16 response", context->responseBuf.s16Response);
    PrintJson("build s32 response", context->responseBuf.s32Response);
    PrintJson("build s64 response", context->responseBuf.s64Response);
    PrintJson("build u8 response", context->responseBuf.u8Response);
    PrintJson("build u16 response", context->responseBuf.u16Response);
    PrintJson("build u32 response", context->responseBuf.u32Response);
    PrintJson("build u64 response", context->responseBuf.u64Response);
    PrintJson("build bool true response", context->responseBuf.boolTrueResponse);
    PrintJson("build bool false response", context->responseBuf.boolFalseResponse);
    PrintJson("build float response", context->responseBuf.floatResponse);
    PrintJson("build double var response", context->responseBuf.doubleVarResponse);
    PrintJson("build char var response", context->responseBuf.charVarResponse);
    PrintJson("build basic object", context->responseBuf.basicObjectResponse);
}

/******************************************************************************
 * @brief      : 解析并打印空响应
 * @param[in]  : context --测试上下文
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于验证DATA_EMPTY响应解析
 ******************************************************************************/
static void ParseAndPrintEmptyResponse(const TestContext_S *context)
{
    CJSONResponse_S response = {0};

    if (context == NULL) {
        return;
    }

    if (FT_ResultParse(DATA_EMPTY, context->responseBuf.emptyResponse, &response, NULL) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d\n", "parse empty response", response.result, response.dataType);
    }
}

/******************************************************************************
 * @brief      : 解析并打印字符串响应
 * @param[in]  : 无
 * @param[out] : context --测试上下文中的dataString缓冲区
 * @return     : 无
 * @note       : 用于验证DATA_STRING响应解析
 ******************************************************************************/
static void ParseAndPrintStringResponse(TestContext_S *context)
{
    CJSONResponse_S response = {0};

    if (context == NULL) {
        return;
    }

    context->testData.dataString[0] = '\0';
    if (FT_ResultParse(DATA_STRING, context->responseBuf.stringResponse, &response, context->testData.dataString) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d data=%s\n", "parse string response", response.result, response.dataType, response.dataString);
    }
}

/******************************************************************************
 * @brief      : 解析并打印VoltageInfo_S对象响应
 * @param[in]  : 无
 * @param[out] : context --测试上下文中的parsedVoltageInfo对象
 * @return     : 无
 * @note       : 用于验证对象字段反序列化，包括字符串、布尔、浮点和字符字段
 ******************************************************************************/
static void ParseAndPrintVoltageObject(TestContext_S *context)
{
    CJSONResponse_S response = {0};

    if (context == NULL) {
        return;
    }

    context->testData.parsedVoltageInfo = (VoltageInfo_S){0};
    if (FT_ResultParse(DATA_OBJECT, context->responseBuf.objectResponse, &response, &context->testData.parsedVoltageInfo) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d vol=%d cap=%d channel=%u temperature=%u count=%u timestamp=%llu version=%s flag=%d ratio=%Lf grade=%c\n",
               "parse object response", response.result, response.dataType, context->testData.parsedVoltageInfo.vol, context->testData.parsedVoltageInfo.cap,
               context->testData.parsedVoltageInfo.channel, context->testData.parsedVoltageInfo.temperature, context->testData.parsedVoltageInfo.count,
               (unsigned long long)context->testData.parsedVoltageInfo.timestamp, context->testData.parsedVoltageInfo.version,
               context->testData.parsedVoltageInfo.flag, (long double)context->testData.parsedVoltageInfo.ratio, context->testData.parsedVoltageInfo.grade);
    }
}

/******************************************************************************
 * @brief      : 解析并打印原始示例响应
 * @param[in]  : 无
 * @param[out] : context --测试上下文中的解析结果对象和字符串缓冲区
 * @return     : 无
 * @note       : 依次解析空、字符串、对象、整数、无符号整数、布尔、浮点和字符响应
 ******************************************************************************/
static void ParseOriginalExampleResponses(TestContext_S *context)
{
    if (context == NULL) {
        return;
    }

    printf("\n========== parse original example ==========\n");

    ParseAndPrintEmptyResponse(context);
    ParseAndPrintStringResponse(context);
    ParseAndPrintVoltageObject(context);

    ParseAndPrintInt("parse int response", context->responseBuf.intResponse);
    ParseAndPrintUInt("parse uint response", context->responseBuf.uintResponse);
    ParseAndPrintBool("parse bool response", context->responseBuf.boolResponse);
    ParseAndPrintDouble("parse double response", context->responseBuf.doubleResponse);
    ParseAndPrintChar("parse char response", context->responseBuf.charResponse);
}

/******************************************************************************
 * @brief      : 解析并打印基础整数类型响应
 * @param[in]  : context --测试上下文
 * @param[out] : 无
 * @return     : 无
 * @note       : 依次解析s8~s64和u8~u64响应
 ******************************************************************************/
static void ParseBasicIntegerResponses(const TestContext_S *context)
{
    if (context == NULL) {
        return;
    }

    ParseAndPrintInt("parse s8 response", context->responseBuf.s8Response);
    ParseAndPrintInt("parse s16 response", context->responseBuf.s16Response);
    ParseAndPrintInt("parse s32 response", context->responseBuf.s32Response);
    ParseAndPrintInt("parse s64 response", context->responseBuf.s64Response);
    ParseAndPrintUInt("parse u8 response", context->responseBuf.u8Response);
    ParseAndPrintUInt("parse u16 response", context->responseBuf.u16Response);
    ParseAndPrintUInt("parse u32 response", context->responseBuf.u32Response);
    ParseAndPrintUInt("parse u64 response", context->responseBuf.u64Response);
}

/******************************************************************************
 * @brief      : 解析并打印基础布尔、浮点和字符响应
 * @param[in]  : context --测试上下文
 * @param[out] : 无
 * @return     : 无
 * @note       : 依次解析bool true、bool false、float、double和char响应
 ******************************************************************************/
static void ParseBasicBoolFloatCharResponses(const TestContext_S *context)
{
    if (context == NULL) {
        return;
    }

    ParseAndPrintBool("parse bool true", context->responseBuf.boolTrueResponse);
    ParseAndPrintBool("parse bool false", context->responseBuf.boolFalseResponse);
    ParseAndPrintDouble("parse float response", context->responseBuf.floatResponse);
    ParseAndPrintDouble("parse double var response", context->responseBuf.doubleVarResponse);
    ParseAndPrintChar("parse char var response", context->responseBuf.charVarResponse);
}

/******************************************************************************
 * @brief      : 解析并打印BasicTypeInfo_S对象响应
 * @param[in]  : 无
 * @param[out] : context --测试上下文中的parsedBasicTypeInfo对象
 * @return     : 无
 * @note       : 用于验证结构体对象中所有基础类型字段的反序列化
 ******************************************************************************/
static void ParseAndPrintBasicObject(TestContext_S *context)
{
    CJSONResponse_S response = {0};

    if (context == NULL) {
        return;
    }

    context->testData.parsedBasicTypeInfo = (BasicTypeInfo_S){0};
    if (FT_ResultParse(DATA_OBJECT, context->responseBuf.basicObjectResponse, &response, &context->testData.parsedBasicTypeInfo) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d s8=%d s16=%d s32=%d s64=%lld u8=%u u16=%u u32=%u u64=%llu boolTrue=%d boolFalse=%d float=%f double=%Lf char=%c\n",
               "parse basic object", response.result, response.dataType, context->testData.parsedBasicTypeInfo.s8Value,
               context->testData.parsedBasicTypeInfo.s16Value, context->testData.parsedBasicTypeInfo.s32Value,
               (long long)context->testData.parsedBasicTypeInfo.s64Value, context->testData.parsedBasicTypeInfo.u8Value,
               context->testData.parsedBasicTypeInfo.u16Value, context->testData.parsedBasicTypeInfo.u32Value,
               (unsigned long long)context->testData.parsedBasicTypeInfo.u64Value, context->testData.parsedBasicTypeInfo.boolTrueValue,
               context->testData.parsedBasicTypeInfo.boolFalseValue, context->testData.parsedBasicTypeInfo.floatValue,
               (long double)context->testData.parsedBasicTypeInfo.doubleValue, context->testData.parsedBasicTypeInfo.charValue);
    }
}

/******************************************************************************
 * @brief      : 解析并打印所有基础类型响应
 * @param[in]  : 无
 * @param[out] : context --测试上下文中的基础类型对象解析结果
 * @return     : 无
 * @note       : 解析基础整数、布尔、浮点、字符和基础类型对象响应
 ******************************************************************************/
static void ParseBasicTypeResponses(TestContext_S *context)
{
    if (context == NULL) {
        return;
    }

    printf("\n========== parse all basic types ==========\n");

    ParseBasicIntegerResponses(context);
    ParseBasicBoolFloatCharResponses(context);
    ParseAndPrintBasicObject(context);
}

/******************************************************************************
 * @brief      : 主函数
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 0 --程序正常结束
 * @note       : 完成测试数据初始化、JSON构建、构建结果打印、JSON解析和解析结果打印
 ******************************************************************************/
int main(void)
{
    TestContext_S context = {0};

    InitTestContext(&context);

    BuildOriginalExampleResponses(&context);
    BuildBasicTypeResponses(&context);
    BuildBasicObjectResponse(&context);

    PrintOriginalExampleBuildResult(&context);
    PrintBasicTypeBuildResult(&context);

    ParseOriginalExampleResponses(&context);
    ParseBasicTypeResponses(&context);

    return 0;
}

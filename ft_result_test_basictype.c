/******************************************************************************
 * File Name : ft_result_test_basictype.c
 * Function  : BasicType相关测试代码实现。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/15
 ******************************************************************************/

#include "ft_result_test_basictype.h"

#include <stdio.h>

FT_RESULT_DEFINE_OBJECT_TYPE(BasicTypeInfo_S, s8Value, s16Value, s32Value, s64Value, u8Value, u16Value, u32Value, u64Value, boolTrueValue, boolFalseValue,
                             floatValue, doubleValue, charValue)

/******************************************************************************
 * @brief      : 注册BasicType测试对象类型
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 测试程序启动时调用一次
 ******************************************************************************/
CJSONBool_E FT_ResultRegisterBasicTypeObjects(void)
{
    if (FT_ResultRegister_BasicTypeInfo_S() != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 打印JSON字符串
 * @param[in]  : title --标题, jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 统一格式打印
 ******************************************************************************/
static void PrintJson(const char* title, const char* jsonString)
{
    printf("%-28s: %s\n", title, jsonString);
}

/******************************************************************************
 * @brief      : 初始化BasicType测试数据
 * @param[in]  : 无
 * @param[out] : basicData --测试数据
 * @return     : 无
 * @note       : 初始化BasicType测试对象
 ******************************************************************************/
void FT_ResultInitBasicTypeData(BasicTypeTestData_S* basicData)
{
    if (basicData == NULL) {
        return;
    }

    *basicData = (BasicTypeTestData_S){0};
    basicData->basicTypeInfo =
        (BasicTypeInfo_S){(s8)-12, (s16)-1234, (s32)-12345678, (s64)-123456789012345LL, (u8)12U, (u16)1234U, (u32)12345678UL, (u64)123456789012345ULL,
                          true,    false,      (f32)1.25F,     (f64)3.1415926,          'Z'};
}

/******************************************************************************
 * @brief      : 构建原始示例响应
 * @param[in]  : 无
 * @param[out] : responseBuf --响应缓冲区
 * @return     : 无
 * @note       : 构建空、字符串和常用基础类型响应
 ******************************************************************************/
void FT_ResultBuildOriginalResponses(BasicTypeResponseBuffer_S* responseBuf)
{
    u64 timestamp = 123456789ULL;

    if (responseBuf == NULL) {
        return;
    }

    FT_ResultPrint(DATA_EMPTY, responseBuf->emptyResponse, sizeof(responseBuf->emptyResponse), RESULT_FAIL, (const void*)NULL);
    FT_ResultPrint(DATA_STRING, responseBuf->stringResponse, sizeof(responseBuf->stringResponse), RESULT_PASS, "SN123456");
    FT_ResultPrint(DATA_INT, responseBuf->intResponse, sizeof(responseBuf->intResponse), RESULT_PASS, 55);
    FT_ResultPrint(DATA_UINT, responseBuf->uintResponse, sizeof(responseBuf->uintResponse), RESULT_PASS, timestamp);
    FT_ResultPrint(DATA_BOOL, responseBuf->boolResponse, sizeof(responseBuf->boolResponse), RESULT_PASS, true);
    FT_ResultPrint(DATA_DOUBLE, responseBuf->doubleResponse, sizeof(responseBuf->doubleResponse), RESULT_PASS, 3.1415926);
    FT_ResultPrint(DATA_CHAR, responseBuf->charResponse, sizeof(responseBuf->charResponse), RESULT_PASS, (char)'A');
}

/******************************************************************************
 * @brief      : 构建基础类型响应
 * @param[in]  : basicData --测试数据
 * @param[out] : responseBuf --响应缓冲区
 * @return     : 无
 * @note       : 覆盖s8~s64、u8~u64、bool、float、double、char
 ******************************************************************************/
void FT_ResultBuildBasicTypeResponses(const BasicTypeTestData_S* basicData, BasicTypeResponseBuffer_S* responseBuf)
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

    if (basicData == NULL || responseBuf == NULL) {
        return;
    }

    FT_ResultPrint(DATA_INT, responseBuf->s8Response, sizeof(responseBuf->s8Response), RESULT_PASS, s8Value);
    FT_ResultPrint(DATA_INT, responseBuf->s16Response, sizeof(responseBuf->s16Response), RESULT_PASS, s16Value);
    FT_ResultPrint(DATA_INT, responseBuf->s32Response, sizeof(responseBuf->s32Response), RESULT_PASS, s32Value);
    FT_ResultPrint(DATA_INT, responseBuf->s64Response, sizeof(responseBuf->s64Response), RESULT_PASS, s64Value);
    FT_ResultPrint(DATA_UINT, responseBuf->u8Response, sizeof(responseBuf->u8Response), RESULT_PASS, u8Value);
    FT_ResultPrint(DATA_UINT, responseBuf->u16Response, sizeof(responseBuf->u16Response), RESULT_PASS, u16Value);
    FT_ResultPrint(DATA_UINT, responseBuf->u32Response, sizeof(responseBuf->u32Response), RESULT_PASS, u32Value);
    FT_ResultPrint(DATA_UINT, responseBuf->u64Response, sizeof(responseBuf->u64Response), RESULT_PASS, u64Value);
    FT_ResultPrint(DATA_BOOL, responseBuf->boolTrueResponse, sizeof(responseBuf->boolTrueResponse), RESULT_PASS, boolTrueValue);
    FT_ResultPrint(DATA_BOOL, responseBuf->boolFalseResponse, sizeof(responseBuf->boolFalseResponse), RESULT_PASS, boolFalseValue);
    FT_ResultPrint(DATA_DOUBLE, responseBuf->floatResponse, sizeof(responseBuf->floatResponse), RESULT_PASS, floatValue);
    FT_ResultPrint(DATA_DOUBLE, responseBuf->doubleVarResponse, sizeof(responseBuf->doubleVarResponse), RESULT_PASS, doubleValue);
    FT_ResultPrint(DATA_CHAR, responseBuf->charVarResponse, sizeof(responseBuf->charVarResponse), RESULT_PASS, charValue);
    FT_ResultPrint(BasicTypeInfo_S, responseBuf->basicObjectResponse, sizeof(responseBuf->basicObjectResponse), RESULT_PASS, &basicData->basicTypeInfo);
}

/******************************************************************************
 * @brief      : 打印构建结果
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : 无
 * @return     : 无
 * @note       : 打印所有构建出的JSON
 ******************************************************************************/
void FT_ResultPrintBasicTypeBuildResult(const BasicTypeResponseBuffer_S* responseBuf)
{
    if (responseBuf == NULL) {
        return;
    }

    printf("========== build original example ==========\n");
    PrintJson("build empty response", responseBuf->emptyResponse);
    PrintJson("build string response", responseBuf->stringResponse);
    PrintJson("build int response", responseBuf->intResponse);
    PrintJson("build uint response", responseBuf->uintResponse);
    PrintJson("build bool response", responseBuf->boolResponse);
    PrintJson("build double response", responseBuf->doubleResponse);
    PrintJson("build char response", responseBuf->charResponse);

    printf("\n========== build all basic types ==========\n");
    PrintJson("build s8 response", responseBuf->s8Response);
    PrintJson("build s16 response", responseBuf->s16Response);
    PrintJson("build s32 response", responseBuf->s32Response);
    PrintJson("build s64 response", responseBuf->s64Response);
    PrintJson("build u8 response", responseBuf->u8Response);
    PrintJson("build u16 response", responseBuf->u16Response);
    PrintJson("build u32 response", responseBuf->u32Response);
    PrintJson("build u64 response", responseBuf->u64Response);
    PrintJson("build bool true response", responseBuf->boolTrueResponse);
    PrintJson("build bool false response", responseBuf->boolFalseResponse);
    PrintJson("build float response", responseBuf->floatResponse);
    PrintJson("build double var response", responseBuf->doubleVarResponse);
    PrintJson("build char var response", responseBuf->charVarResponse);
    PrintJson("build basic object", responseBuf->basicObjectResponse);
}

/******************************************************************************
 * @brief      : 解析并打印有符号整数响应
 * @param[in]  : title --标题, jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 验证DATA_INT解析
 ******************************************************************************/
static void ParseAndPrintInt(const char* title, const char* jsonString)
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
 * @param[in]  : title --标题, jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 验证DATA_UINT解析
 ******************************************************************************/
static void ParseAndPrintUInt(const char* title, const char* jsonString)
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
 * @param[in]  : title --标题, jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 验证DATA_BOOL解析
 ******************************************************************************/
static void ParseAndPrintBool(const char* title, const char* jsonString)
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
 * @param[in]  : title --标题, jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 验证DATA_DOUBLE解析
 ******************************************************************************/
static void ParseAndPrintDouble(const char* title, const char* jsonString)
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
 * @param[in]  : title --标题, jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 验证DATA_CHAR解析
 ******************************************************************************/
static void ParseAndPrintChar(const char* title, const char* jsonString)
{
    CJSONResponse_S response = {0};

    if (FT_ResultParse(DATA_CHAR, jsonString, &response, NULL) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d value=%c\n", title, response.result, response.dataType, response.charValue);
    } else {
        printf("%-28s: parse failed\n", title);
    }
}

/******************************************************************************
 * @brief      : 解析原始示例响应
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : dataString --字符串缓冲区
 * @return     : 无
 * @note       : 解析空、字符串和常用基础类型
 ******************************************************************************/
void FT_ResultParseOriginalResponses(const BasicTypeResponseBuffer_S* responseBuf, char* dataString)
{
    CJSONResponse_S response = {0};

    if (responseBuf == NULL) {
        return;
    }

    printf("\n========== parse original example ==========\n");

    if (FT_ResultParse(DATA_EMPTY, responseBuf->emptyResponse, &response, NULL) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d\n", "parse empty response", response.result, response.dataType);
    }

    response = (CJSONResponse_S){0};
    if (FT_ResultParse(DATA_STRING, responseBuf->stringResponse, &response, dataString) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d data=%s\n", "parse string response", response.result, response.dataType, dataString);
    }

    ParseAndPrintInt("parse int response", responseBuf->intResponse);
    ParseAndPrintUInt("parse uint response", responseBuf->uintResponse);
    ParseAndPrintBool("parse bool response", responseBuf->boolResponse);
    ParseAndPrintDouble("parse double response", responseBuf->doubleResponse);
    ParseAndPrintChar("parse char response", responseBuf->charResponse);
}

/******************************************************************************
 * @brief      : 解析基础类型响应
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : basicData --测试数据
 * @return     : 无
 * @note       : 解析所有基础类型和基础类型对象
 ******************************************************************************/
void FT_ResultParseBasicTypeResponses(const BasicTypeResponseBuffer_S* responseBuf, BasicTypeTestData_S* basicData)
{
    CJSONResponse_S response = {0};

    if (responseBuf == NULL || basicData == NULL) {
        return;
    }

    printf("\n========== parse all basic types ==========\n");

    ParseAndPrintInt("parse s8 response", responseBuf->s8Response);
    ParseAndPrintInt("parse s16 response", responseBuf->s16Response);
    ParseAndPrintInt("parse s32 response", responseBuf->s32Response);
    ParseAndPrintInt("parse s64 response", responseBuf->s64Response);
    ParseAndPrintUInt("parse u8 response", responseBuf->u8Response);
    ParseAndPrintUInt("parse u16 response", responseBuf->u16Response);
    ParseAndPrintUInt("parse u32 response", responseBuf->u32Response);
    ParseAndPrintUInt("parse u64 response", responseBuf->u64Response);
    ParseAndPrintBool("parse bool true", responseBuf->boolTrueResponse);
    ParseAndPrintBool("parse bool false", responseBuf->boolFalseResponse);
    ParseAndPrintDouble("parse float response", responseBuf->floatResponse);
    ParseAndPrintDouble("parse double var response", responseBuf->doubleVarResponse);
    ParseAndPrintChar("parse char var response", responseBuf->charVarResponse);

    if (FT_ResultParse(BasicTypeInfo_S, responseBuf->basicObjectResponse, &response, &basicData->parsedBasicTypeInfo) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d s8=%d s16=%d s32=%d s64=%lld u8=%u "
               "u16=%u u32=%u u64=%llu boolTrue=%d boolFalse=%d float=%f "
               "double=%Lf char=%c\n",
               "parse basic object", response.result, response.dataType, basicData->parsedBasicTypeInfo.s8Value, basicData->parsedBasicTypeInfo.s16Value,
               basicData->parsedBasicTypeInfo.s32Value, (long long)basicData->parsedBasicTypeInfo.s64Value, basicData->parsedBasicTypeInfo.u8Value,
               basicData->parsedBasicTypeInfo.u16Value, basicData->parsedBasicTypeInfo.u32Value, (unsigned long long)basicData->parsedBasicTypeInfo.u64Value,
               basicData->parsedBasicTypeInfo.boolTrueValue, basicData->parsedBasicTypeInfo.boolFalseValue, basicData->parsedBasicTypeInfo.floatValue,
               (long double)basicData->parsedBasicTypeInfo.doubleValue, basicData->parsedBasicTypeInfo.charValue);
    }
}

/******************************************************************************
 * @brief      : 运行BasicType测试
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 0 --成功，非0 --失败
 * @note       : 完成注册、构建、打印、解析
 ******************************************************************************/
int FT_ResultRunBasicTypeTests(void)
{
    BasicTypeTestData_S basicData = {0};
    BasicTypeResponseBuffer_S responseBuf = {0};

    if (FT_ResultRegisterBasicTypeObjects() != BOOL_TRUE) {
        printf("register basictype object failed\n");
        return -1;
    }

    FT_ResultInitBasicTypeData(&basicData);
    FT_ResultBuildOriginalResponses(&responseBuf);
    FT_ResultBuildBasicTypeResponses(&basicData, &responseBuf);
    FT_ResultPrintBasicTypeBuildResult(&responseBuf);
    FT_ResultParseOriginalResponses(&responseBuf, basicData.dataString);
    FT_ResultParseBasicTypeResponses(&responseBuf, &basicData);

    return 0;
}

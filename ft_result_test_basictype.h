/******************************************************************************
 * File Name : ft_result_test_basictype.h
 * Function  : BasicType相关测试类型和测试接口声明。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/15
 ******************************************************************************/

#ifndef FT_RESULT_TEST_BASICTYPE_H
#define FT_RESULT_TEST_BASICTYPE_H

#include "cjson_response_api.h"

#include <stdbool.h>
#include <stdint.h>

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
} BasicTypeResponseBuffer_S;

typedef struct {
    BasicTypeInfo_S basicTypeInfo;
    BasicTypeInfo_S parsedBasicTypeInfo;
    char dataString[256];
} BasicTypeTestData_S;

FT_RESULT_DECLARE_OBJECT_TYPE(BasicTypeInfo_S);

/******************************************************************************
 * @brief      : 注册BasicType测试对象类型
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 测试程序启动时调用一次
 ******************************************************************************/
CJSONBool_E FT_ResultRegisterBasicTypeObjects(void);

/******************************************************************************
 * @brief      : 初始化BasicType测试数据
 * @param[in]  : 无
 * @param[out] : basicData --测试数据
 * @return     : 无
 * @note       : 初始化BasicType测试对象
 ******************************************************************************/
void FT_ResultInitBasicTypeData(BasicTypeTestData_S* basicData);

/******************************************************************************
 * @brief      : 构建原始示例响应
 * @param[in]  : 无
 * @param[out] : responseBuf --响应缓冲区
 * @return     : 无
 * @note       : 构建空、字符串和常用基础类型响应
 ******************************************************************************/
void FT_ResultBuildOriginalResponses(BasicTypeResponseBuffer_S* responseBuf);

/******************************************************************************
 * @brief      : 构建基础类型响应
 * @param[in]  : basicData --测试数据
 * @param[out] : responseBuf --响应缓冲区
 * @return     : 无
 * @note       : 覆盖s8~s64、u8~u64、bool、float、double、char
 ******************************************************************************/
void FT_ResultBuildBasicTypeResponses(const BasicTypeTestData_S* basicData, BasicTypeResponseBuffer_S* responseBuf);

/******************************************************************************
 * @brief      : 打印构建结果
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : 无
 * @return     : 无
 * @note       : 打印所有构建出的JSON
 ******************************************************************************/
void FT_ResultPrintBasicTypeBuildResult(const BasicTypeResponseBuffer_S* responseBuf);

/******************************************************************************
 * @brief      : 解析原始示例响应
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : dataString --字符串缓冲区
 * @return     : 无
 * @note       : 解析空、字符串和常用基础类型
 ******************************************************************************/
void FT_ResultParseOriginalResponses(const BasicTypeResponseBuffer_S* responseBuf, char* dataString);

/******************************************************************************
 * @brief      : 解析基础类型响应
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : basicData --测试数据
 * @return     : 无
 * @note       : 解析所有基础类型和基础类型对象
 ******************************************************************************/
void FT_ResultParseBasicTypeResponses(const BasicTypeResponseBuffer_S* responseBuf, BasicTypeTestData_S* basicData);

/******************************************************************************
 * @brief      : 运行BasicType测试
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 0 --成功，非0 --失败
 * @note       : 完成注册、构建、打印、解析
 ******************************************************************************/
int FT_ResultRunBasicTypeTests(void);

#endif

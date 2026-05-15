/******************************************************************************
 * File Name : cjson_response.c
 * Function  : 响应JSON构造、解析和结构体字段注册接口实现。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#include "cjson_response_api.h"
#include "securec.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define CJSON_KEY_RESULT                  ("result")
#define CJSON_KEY_DATA                    ("data")
#define CJSON_RESULT_PASS_STRING          ("pass")
#define CJSON_RESULT_FAIL_STRING          ("fail")
#define CJSON_STRING_EMPTY                ("")
#define CJSON_NUMBER_BUFFER_SIZE          (64U)
#define CJSON_RESPONSE_STRING_BUFFER_SIZE (256U)
#define CJSON_MAX_SCAN_SIZE               (4096U)
#define CJSON_MAX_OBJECT_TYPE_COUNT       (64U)
#define CJSON_SNPRINTF_COUNT(destMax)     (((destMax) > 0U) ? ((destMax) - 1U) : 0U)

typedef struct {
    char* buffer;
    size_t length;
    size_t capacity;
} CJSONBuffer_S;

static CJSONObjectType_S g_cjsonObjectTypeTable[CJSON_MAX_OBJECT_TYPE_COUNT] = {0};
static size_t g_cjsonObjectTypeCount = 0U;

/******************************************************************************
 * @brief      : 判断JSON数值是否结束
 * @param[in]  : value --当前字符
 * @param[out] : 无
 * @return     : BOOL_TRUE --结束，BOOL_FALSE --未结束
 * @note       : 用于strtoll/strtoull/strtold解析后检查尾部字符
 ******************************************************************************/
static CJSONBool_E CJSON_IsJsonValueEnd(char value)
{
    if ((value == '\0') || (value == ',') || (value == '}') || (isspace((unsigned char)value) != 0)) {
        return BOOL_TRUE;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 比较对象类型名
 * @param[in]  : left --左类型名, right --右类型名
 * @param[out] : 无
 * @return     : BOOL_TRUE --相同，BOOL_FALSE --不同
 * @note       : 内部使用strcmp比较
 ******************************************************************************/
static CJSONBool_E CJSON_IsSameObjectTypeName(const char* left, const char* right)
{
    if ((left == NULL) || (right == NULL)) {
        return BOOL_FALSE;
    }

    return (strcmp(left, right) == 0) ? BOOL_TRUE : BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 注册对象类型字段表
 * @param[in]  : typeName --类型名, fields --字段描述表
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 同名类型重复注册时直接返回成功
 ******************************************************************************/
CJSONBool_E CJSON_RegisterObjectType(const char* typeName, const CJSONField_S* fields)
{
    size_t index = 0U;

    if ((typeName == NULL) || (fields == NULL)) {
        return BOOL_FALSE;
    }

    for (index = 0U; index < g_cjsonObjectTypeCount; index++) {
        if (CJSON_IsSameObjectTypeName(g_cjsonObjectTypeTable[index].typeName, typeName) == BOOL_TRUE) {
            return BOOL_TRUE;
        }
    }

    if (g_cjsonObjectTypeCount >= CJSON_MAX_OBJECT_TYPE_COUNT) {
        return BOOL_FALSE;
    }

    g_cjsonObjectTypeTable[g_cjsonObjectTypeCount].typeName = typeName;
    g_cjsonObjectTypeTable[g_cjsonObjectTypeCount].fields = fields;
    g_cjsonObjectTypeCount++;

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 查找对象字段表
 * @param[in]  : typeName --类型名
 * @param[out] : 无
 * @return     : 字段描述表，失败返回NULL
 * @note       : 对象构建和对象解析使用
 ******************************************************************************/
const CJSONField_S* CJSON_FindObjectFields(const char* typeName)
{
    size_t index = 0U;

    if (typeName == NULL) {
        return NULL;
    }

    for (index = 0U; index < g_cjsonObjectTypeCount; index++) {
        if (CJSON_IsSameObjectTypeName(g_cjsonObjectTypeTable[index].typeName, typeName) == BOOL_TRUE) {
            return g_cjsonObjectTypeTable[index].fields;
        }
    }

    return NULL;
}

/******************************************************************************
 * @brief      : 构建参数设置为有符号整数
 * @param[in]  : value --有符号整数值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_ResultPrint宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetInt(CJSONBuildParam_S* buildParam, int64_t value)
{
    if (buildParam != NULL) {
        buildParam->intValue = value;
        buildParam->uintValue = (uint64_t)value;
        buildParam->doubleValue = (long double)value;
        buildParam->boolValue = (value != 0) ? BOOL_TRUE : BOOL_FALSE;
        buildParam->charValue = (char)value;
    }
}

/******************************************************************************
 * @brief      : 构建参数设置为无符号整数
 * @param[in]  : value --无符号整数值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_ResultPrint宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetUInt(CJSONBuildParam_S* buildParam, uint64_t value)
{
    if (buildParam != NULL) {
        buildParam->intValue = (int64_t)value;
        buildParam->uintValue = value;
        buildParam->doubleValue = (long double)value;
        buildParam->boolValue = (value != 0U) ? BOOL_TRUE : BOOL_FALSE;
        buildParam->charValue = (char)value;
    }
}

/******************************************************************************
 * @brief      : 构建参数设置为布尔值
 * @param[in]  : value --布尔值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_ResultPrint宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetBool(CJSONBuildParam_S* buildParam, int value)
{
    if (buildParam != NULL) {
        buildParam->intValue = (value != 0) ? 1 : 0;
        buildParam->uintValue = (value != 0) ? 1U : 0U;
        buildParam->doubleValue = (value != 0) ? 1.0L : 0.0L;
        buildParam->boolValue = (value != 0) ? BOOL_TRUE : BOOL_FALSE;
        buildParam->charValue = (value != 0) ? (char)1 : (char)0;
    }
}

/******************************************************************************
 * @brief      : 构建参数设置为浮点数
 * @param[in]  : value --浮点数值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_ResultPrint宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetDouble(CJSONBuildParam_S* buildParam, long double value)
{
    if (buildParam != NULL) {
        buildParam->intValue = (int64_t)value;
        buildParam->uintValue = (uint64_t)value;
        buildParam->doubleValue = value;
        buildParam->boolValue = (value != 0.0L) ? BOOL_TRUE : BOOL_FALSE;
        buildParam->charValue = (char)value;
    }
}

/******************************************************************************
 * @brief      : 构建参数设置为单字符
 * @param[in]  : value --字符值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_ResultPrint宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetChar(CJSONBuildParam_S* buildParam, char value)
{
    if (buildParam != NULL) {
        buildParam->charValue = value;
    }
}

/******************************************************************************
 * @brief      : 构建参数设置为指针数据
 * @param[in]  : value --字符串或指针数据
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_ResultPrint宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetPointer(CJSONBuildParam_S* buildParam, const void* value)
{
    if (buildParam != NULL) {
        buildParam->data = value;
    }
}

/******************************************************************************
 * @brief      : 内存拷贝封装
 * @param[in]  : destMax --目标缓冲区长度, src --源地址, count --拷贝长度
 * @param[out] : dest --目标地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 内部调用memcpy_s
 ******************************************************************************/
static CJSONBool_E CJSON_CopyMemory(void* dest, size_t destMax, const void* src, size_t count)
{
    if ((dest == NULL) || (destMax == 0U)) {
        return BOOL_FALSE;
    }

    if (count == 0U) {
        return BOOL_TRUE;
    }

    if ((src == NULL) || (count > destMax)) {
        return BOOL_FALSE;
    }

    return (memcpy_s(dest, destMax, src, count) == EOK) ? BOOL_TRUE : BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 清空输出缓冲区
 * @param[in]  : 无
 * @param[out] : dataBuf --输出缓冲区
 * @return     : 无
 * @note       : 构建失败时置为空字符串
 ******************************************************************************/
static void CJSON_ClearOutput(char* dataBuf)
{
    if (dataBuf != NULL) {
        dataBuf[0] = '\0';
    }
}

/******************************************************************************
 * @brief      : 获取有限长度字符串长度
 * @param[in]  : input --输入字符串, maxLen --最大扫描长度
 * @param[out] : 无
 * @return     : 字符串长度
 * @note       : 避免异常输入导致无界扫描
 ******************************************************************************/
static size_t CJSON_Strnlen(const char* input, size_t maxLen)
{
    size_t index = 0U;

    if (input == NULL) {
        return 0U;
    }

    while ((index < maxLen) && (input[index] != '\0')) {
        index++;
    }

    return index;
}

/******************************************************************************
 * @brief      : 格式化有符号整数
 * @param[in]  : bufferSize --缓冲区大小, value --整数值
 * @param[out] : buffer --输出缓冲区
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 内部调用snprintf_s
 ******************************************************************************/
static CJSONBool_E CJSON_FormatSignedInteger(char* buffer, size_t bufferSize, int64_t value)
{
    if ((buffer == NULL) || (bufferSize == 0U)) {
        return BOOL_FALSE;
    }

    buffer[0] = '\0';
    if (snprintf_s(buffer, bufferSize, CJSON_SNPRINTF_COUNT(bufferSize), "%" PRId64, value) < 0) {
        buffer[0] = '\0';
        return BOOL_FALSE;
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 格式化无符号整数
 * @param[in]  : bufferSize --缓冲区大小, value --整数值
 * @param[out] : buffer --输出缓冲区
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 内部调用snprintf_s
 ******************************************************************************/
static CJSONBool_E CJSON_FormatUnsignedInteger(char* buffer, size_t bufferSize, uint64_t value)
{
    if ((buffer == NULL) || (bufferSize == 0U)) {
        return BOOL_FALSE;
    }

    buffer[0] = '\0';
    if (snprintf_s(buffer, bufferSize, CJSON_SNPRINTF_COUNT(bufferSize), "%" PRIu64, value) < 0) {
        buffer[0] = '\0';
        return BOOL_FALSE;
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 格式化浮点数
 * @param[in]  : bufferSize --缓冲区大小, value --浮点值
 * @param[out] : buffer --输出缓冲区
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输出使用%.21Lg
 ******************************************************************************/
static CJSONBool_E CJSON_FormatDouble(char* buffer, size_t bufferSize, long double value)
{
    if ((buffer == NULL) || (bufferSize == 0U)) {
        return BOOL_FALSE;
    }

    buffer[0] = '\0';
    if (snprintf_s(buffer, bufferSize, CJSON_SNPRINTF_COUNT(bufferSize), "%.21Lg", value) < 0) {
        buffer[0] = '\0';
        return BOOL_FALSE;
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 格式化Unicode转义
 * @param[in]  : bufferSize --缓冲区大小, value --字符值
 * @param[out] : buffer --输出缓冲区
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 用于小于32的控制字符
 ******************************************************************************/
static CJSONBool_E CJSON_FormatUnicode(char* buffer, size_t bufferSize, unsigned char value)
{
    if ((buffer == NULL) || (bufferSize == 0U)) {
        return BOOL_FALSE;
    }

    buffer[0] = '\0';
    if (snprintf_s(buffer, bufferSize, CJSON_SNPRINTF_COUNT(bufferSize), "\\u%04x", (unsigned int)value) < 0) {
        buffer[0] = '\0';
        return BOOL_FALSE;
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 获取响应结果字符串
 * @param[in]  : result --响应结果枚举
 * @param[out] : 无
 * @return     : pass/fail字符串
 * @note       : 返回静态字符串
 ******************************************************************************/
static const char* CJSON_GetResultString(CJSONResult_E result)
{
    return (result == RESULT_PASS) ? CJSON_RESULT_PASS_STRING : CJSON_RESULT_FAIL_STRING;
}

/******************************************************************************
 * @brief      : 初始化JSON缓冲区
 * @param[in]  : dataBuf --外部缓冲区, dataLen --缓冲区长度
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 不申请堆内存
 ******************************************************************************/
static CJSONBool_E CJSON_BufferInit(CJSONBuffer_S* jsonBuffer, char* dataBuf, size_t dataLen)
{
    if ((jsonBuffer == NULL) || (dataBuf == NULL) || (dataLen == 0U)) {
        return BOOL_FALSE;
    }

    jsonBuffer->buffer = dataBuf;
    jsonBuffer->length = 0U;
    jsonBuffer->capacity = dataLen;
    jsonBuffer->buffer[0] = '\0';

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 检查缓冲区空间
 * @param[in]  : needSize --需要追加的字节数
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --足够，BOOL_FALSE --不足
 * @note       : needSize不包含结尾'\0'
 ******************************************************************************/
static CJSONBool_E CJSON_BufferEnsure(CJSONBuffer_S* jsonBuffer, size_t needSize)
{
    size_t requiredSize = 0U;

    if ((jsonBuffer == NULL) || (jsonBuffer->buffer == NULL) || (jsonBuffer->capacity == 0U)) {
        return BOOL_FALSE;
    }

    if (needSize > (((size_t)-1) - jsonBuffer->length - 1U)) {
        return BOOL_FALSE;
    }

    requiredSize = jsonBuffer->length + needSize + 1U;
    return (requiredSize <= jsonBuffer->capacity) ? BOOL_TRUE : BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 追加普通字符串
 * @param[in]  : text --待追加字符串
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : text为NULL时按空字符串处理
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppend(CJSONBuffer_S* jsonBuffer, const char* text)
{
    size_t textLength = 0U;
    size_t remainSize = 0U;

    if (jsonBuffer == NULL) {
        return BOOL_FALSE;
    }

    text = (text == NULL) ? CJSON_STRING_EMPTY : text;
    textLength = strlen(text);
    if (CJSON_BufferEnsure(jsonBuffer, textLength) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    remainSize = jsonBuffer->capacity - jsonBuffer->length;
    if (CJSON_CopyMemory(jsonBuffer->buffer + jsonBuffer->length, remainSize, text, textLength + 1U) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    jsonBuffer->length += textLength;
    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 追加单个字符
 * @param[in]  : value --字符
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 自动维护'\0'
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendChar(CJSONBuffer_S* jsonBuffer, char value)
{
    if ((jsonBuffer == NULL) || (CJSON_BufferEnsure(jsonBuffer, 1U) != BOOL_TRUE)) {
        return BOOL_FALSE;
    }

    jsonBuffer->buffer[jsonBuffer->length] = value;
    jsonBuffer->length++;
    jsonBuffer->buffer[jsonBuffer->length] = '\0';

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 追加JSON转义字符
 * @param[in]  : value --字符
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 处理常见JSON转义
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendEscapedChar(CJSONBuffer_S* jsonBuffer, unsigned char value)
{
    char unicodeBuffer[8] = {0};

    switch (value) {
        case '\"':
            return CJSON_BufferAppend(jsonBuffer, "\\\"");
        case '\\':
            return CJSON_BufferAppend(jsonBuffer, "\\\\");
        case '\b':
            return CJSON_BufferAppend(jsonBuffer, "\\b");
        case '\f':
            return CJSON_BufferAppend(jsonBuffer, "\\f");
        case '\n':
            return CJSON_BufferAppend(jsonBuffer, "\\n");
        case '\r':
            return CJSON_BufferAppend(jsonBuffer, "\\r");
        case '\t':
            return CJSON_BufferAppend(jsonBuffer, "\\t");
        default:
            break;
    }

    if (value < 32U) {
        if (CJSON_FormatUnicode(unicodeBuffer, sizeof(unicodeBuffer), value) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        return CJSON_BufferAppend(jsonBuffer, unicodeBuffer);
    }

    return CJSON_BufferAppendChar(jsonBuffer, (char)value);
}

/******************************************************************************
 * @brief      : 追加JSON字符串
 * @param[in]  : text --普通字符串
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 自动添加双引号
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendEscapedString(CJSONBuffer_S* jsonBuffer, const char* text)
{
    const unsigned char* current = NULL;

    text = (text == NULL) ? CJSON_STRING_EMPTY : text;
    if (CJSON_BufferAppendChar(jsonBuffer, '\"') != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    for (current = (const unsigned char*)text; *current != '\0'; current++) {
        if (CJSON_BufferAppendEscapedChar(jsonBuffer, *current) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
    }

    return CJSON_BufferAppendChar(jsonBuffer, '\"');
}

/******************************************************************************
 * @brief      : 追加JSON字符值
 * @param[in]  : value --字符
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输出格式为"A"
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendEscapedCharValue(CJSONBuffer_S* jsonBuffer, char value)
{
    if (CJSON_BufferAppendChar(jsonBuffer, '\"') != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedChar(jsonBuffer, (unsigned char)value) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    return CJSON_BufferAppendChar(jsonBuffer, '\"');
}

/******************************************************************************
 * @brief      : 读取有符号整数字段
 * @param[in]  : fieldAddress --字段地址, fieldSize --字段大小
 * @param[out] : value --整数值
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持8/16/32/64位整数
 ******************************************************************************/
static CJSONBool_E CJSON_ReadSignedInteger(const void* fieldAddress, size_t fieldSize, int64_t* value)
{
    if ((fieldAddress == NULL) || (value == NULL)) {
        return BOOL_FALSE;
    }

    if (fieldSize == sizeof(int8_t)) {
        int8_t data = 0;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    if (fieldSize == sizeof(int16_t)) {
        int16_t data = 0;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    if (fieldSize == sizeof(int32_t)) {
        int32_t data = 0;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    if (fieldSize == sizeof(int64_t)) {
        int64_t data = 0;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 读取无符号整数字段
 * @param[in]  : fieldAddress --字段地址, fieldSize --字段大小
 * @param[out] : value --整数值
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持8/16/32/64位整数
 ******************************************************************************/
static CJSONBool_E CJSON_ReadUnsignedInteger(const void* fieldAddress, size_t fieldSize, uint64_t* value)
{
    if ((fieldAddress == NULL) || (value == NULL)) {
        return BOOL_FALSE;
    }

    if (fieldSize == sizeof(uint8_t)) {
        uint8_t data = 0U;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint16_t)) {
        uint16_t data = 0U;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint32_t)) {
        uint32_t data = 0U;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint64_t)) {
        uint64_t data = 0U;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 读取浮点字段
 * @param[in]  : fieldAddress --字段地址, fieldSize --字段大小
 * @param[out] : value --浮点值
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持float/double/long double
 ******************************************************************************/
static CJSONBool_E CJSON_ReadFloatingPoint(const void* fieldAddress, size_t fieldSize, long double* value)
{
    if ((fieldAddress == NULL) || (value == NULL)) {
        return BOOL_FALSE;
    }

    if (fieldSize == sizeof(float)) {
        float data = 0.0F;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = (long double)data;
        return BOOL_TRUE;
    }

    if (fieldSize == sizeof(double)) {
        double data = 0.0;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = (long double)data;
        return BOOL_TRUE;
    }

    if (fieldSize == sizeof(long double)) {
        long double data = 0.0L;
        if (CJSON_CopyMemory(&data, sizeof(data), fieldAddress, sizeof(data)) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        *value = data;
        return BOOL_TRUE;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 写入有符号整数字段
 * @param[in]  : fieldSize --字段大小, value --整数值
 * @param[out] : fieldAddress --字段地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持8/16/32/64位整数
 ******************************************************************************/
static CJSONBool_E CJSON_WriteSignedInteger(void* fieldAddress, size_t fieldSize, int64_t value)
{
    if (fieldAddress == NULL) {
        return BOOL_FALSE;
    }

    if (fieldSize == sizeof(int8_t)) {
        int8_t data = (int8_t)value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    if (fieldSize == sizeof(int16_t)) {
        int16_t data = (int16_t)value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    if (fieldSize == sizeof(int32_t)) {
        int32_t data = (int32_t)value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    if (fieldSize == sizeof(int64_t)) {
        int64_t data = value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 写入无符号整数字段
 * @param[in]  : fieldSize --字段大小, value --整数值
 * @param[out] : fieldAddress --字段地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持8/16/32/64位整数
 ******************************************************************************/
static CJSONBool_E CJSON_WriteUnsignedInteger(void* fieldAddress, size_t fieldSize, uint64_t value)
{
    if (fieldAddress == NULL) {
        return BOOL_FALSE;
    }

    if (fieldSize == sizeof(uint8_t)) {
        uint8_t data = (uint8_t)value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    if (fieldSize == sizeof(uint16_t)) {
        uint16_t data = (uint16_t)value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    if (fieldSize == sizeof(uint32_t)) {
        uint32_t data = (uint32_t)value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    if (fieldSize == sizeof(uint64_t)) {
        uint64_t data = value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 写入浮点字段
 * @param[in]  : fieldSize --字段大小, value --浮点值
 * @param[out] : fieldAddress --字段地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持float/double/long double
 ******************************************************************************/
static CJSONBool_E CJSON_WriteFloatingPoint(void* fieldAddress, size_t fieldSize, long double value)
{
    if (fieldAddress == NULL) {
        return BOOL_FALSE;
    }

    if (fieldSize == sizeof(float)) {
        float data = (float)value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    if (fieldSize == sizeof(double)) {
        double data = (double)value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    if (fieldSize == sizeof(long double)) {
        long double data = value;
        return CJSON_CopyMemory(fieldAddress, fieldSize, &data, sizeof(data));
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 追加整数字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持有符号和无符号整数
 ******************************************************************************/
static CJSONBool_E CJSON_AppendIntegerField(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};
    int64_t signedValue = 0;
    uint64_t unsignedValue = 0U;

    if (field->fieldType == FIELD_INT) {
        if (CJSON_ReadSignedInteger(fieldAddress, field->fieldSize, &signedValue) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        if (CJSON_FormatSignedInteger(numberBuffer, sizeof(numberBuffer), signedValue) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
    } else {
        if (CJSON_ReadUnsignedInteger(fieldAddress, field->fieldSize, &unsignedValue) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        if (CJSON_FormatUnsignedInteger(numberBuffer, sizeof(numberBuffer), unsignedValue) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
    }

    return CJSON_BufferAppend(jsonBuffer, numberBuffer);
}

/******************************************************************************
 * @brief      : 追加浮点字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持float/double/long double
 ******************************************************************************/
static CJSONBool_E CJSON_AppendDoubleField(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};
    long double doubleValue = 0.0L;

    if (CJSON_ReadFloatingPoint(fieldAddress, field->fieldSize, &doubleValue) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (CJSON_FormatDouble(numberBuffer, sizeof(numberBuffer), doubleValue) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    return CJSON_BufferAppend(jsonBuffer, numberBuffer);
}

/******************************************************************************
 * @brief      : 追加布尔字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持_Bool和CJSONBool_E
 ******************************************************************************/
static CJSONBool_E CJSON_AppendBoolField(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    CJSONBool_E boolValue = BOOL_FALSE;

    if ((jsonBuffer == NULL) || (fieldAddress == NULL) || (field == NULL)) {
        return BOOL_FALSE;
    }

    if (field->fieldSize == sizeof(CJSONBool_E)) {
        boolValue = (*(const CJSONBool_E*)fieldAddress == BOOL_TRUE) ? BOOL_TRUE : BOOL_FALSE;
    } else {
        boolValue = (*(const _Bool*)fieldAddress) ? BOOL_TRUE : BOOL_FALSE;
    }

    return CJSON_BufferAppend(jsonBuffer, (boolValue == BOOL_TRUE) ? "true" : "false");
}

/******************************************************************************
 * @brief      : 追加字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段类型分发
 ******************************************************************************/
static CJSONBool_E CJSON_AppendFieldValue(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    if ((jsonBuffer == NULL) || (fieldAddress == NULL) || (field == NULL)) {
        return BOOL_FALSE;
    }

    switch (field->fieldType) {
        case FIELD_INT:
        case FIELD_UINT:
            return CJSON_AppendIntegerField(jsonBuffer, fieldAddress, field);
        case FIELD_BOOL:
            return CJSON_AppendBoolField(jsonBuffer, fieldAddress, field);
        case FIELD_STRING:
            return CJSON_BufferAppendEscapedString(jsonBuffer, (const char*)fieldAddress);
        case FIELD_DOUBLE:
            return CJSON_AppendDoubleField(jsonBuffer, fieldAddress, field);
        case FIELD_CHAR:
            return CJSON_BufferAppendEscapedCharValue(jsonBuffer, *(const char*)fieldAddress);
        default:
            return BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 追加结构体字段
 * @param[in]  : object --对象地址, field --字段描述, isFirst --是否首字段
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输出格式为"key":value
 ******************************************************************************/
static CJSONBool_E CJSON_AppendStructField(CJSONBuffer_S* jsonBuffer, const void* object, const CJSONField_S* field, CJSONBool_E isFirst)
{
    const void* fieldAddress = NULL;

    if ((jsonBuffer == NULL) || (object == NULL) || (field == NULL) || (field->jsonKey == NULL)) {
        return BOOL_FALSE;
    }

    fieldAddress = (const unsigned char*)object + field->fieldOffset;
    if ((isFirst != BOOL_TRUE) && (CJSON_BufferAppendChar(jsonBuffer, ',') != BOOL_TRUE)) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedString(jsonBuffer, field->jsonKey) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendChar(jsonBuffer, ':') != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    return CJSON_AppendFieldValue(jsonBuffer, fieldAddress, field);
}

/******************************************************************************
 * @brief      : 追加结构体对象JSON
 * @param[in]  : object --对象地址, fields --字段描述表
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输出{"key":value,...}
 ******************************************************************************/
static CJSONBool_E CJSON_AppendStructObject(CJSONBuffer_S* jsonBuffer, const void* object, const CJSONField_S* fields)
{
    size_t fieldIndex = 0U;
    CJSONBool_E isFirst = BOOL_TRUE;

    if ((jsonBuffer == NULL) || (object == NULL) || (fields == NULL)) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendChar(jsonBuffer, '{') != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    for (fieldIndex = 0U; fields[fieldIndex].jsonKey != NULL; fieldIndex++) {
        if (CJSON_AppendStructField(jsonBuffer, object, &fields[fieldIndex], isFirst) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        isFirst = BOOL_FALSE;
    }

    return CJSON_BufferAppendChar(jsonBuffer, '}');
}

/******************************************************************************
 * @brief      : 追加响应头
 * @param[in]  : result --响应结果
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输出{"result":"pass/fail","data":
 ******************************************************************************/
static CJSONBool_E CJSON_AppendResponseHead(CJSONBuffer_S* jsonBuffer, CJSONResult_E result)
{
    if (CJSON_BufferAppendChar(jsonBuffer, '{') != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedString(jsonBuffer, CJSON_KEY_RESULT) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendChar(jsonBuffer, ':') != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedString(jsonBuffer, CJSON_GetResultString(result)) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendChar(jsonBuffer, ',') != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedString(jsonBuffer, CJSON_KEY_DATA) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    return CJSON_BufferAppendChar(jsonBuffer, ':');
}

/******************************************************************************
 * @brief      : 追加响应data字段
 * @param[in]  : buildParam --构建参数
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持空、字符串、对象、整数、布尔、浮点和字符
 ******************************************************************************/
static CJSONBool_E CJSON_AppendResponseData(CJSONBuffer_S* jsonBuffer, const CJSONBuildParam_S* buildParam)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};

    if ((jsonBuffer == NULL) || (buildParam == NULL)) {
        return BOOL_FALSE;
    }

    switch (buildParam->dataType) {
        case DATA_EMPTY:
            return CJSON_BufferAppendEscapedString(jsonBuffer, CJSON_STRING_EMPTY);
        case DATA_STRING:
            return CJSON_BufferAppendEscapedString(jsonBuffer, (const char*)buildParam->data);
        case DATA_OBJECT:
            return CJSON_AppendStructObject(jsonBuffer, buildParam->data, buildParam->fields);
        case DATA_INT:
            if (CJSON_FormatSignedInteger(numberBuffer, sizeof(numberBuffer), buildParam->intValue) != BOOL_TRUE) {
                return BOOL_FALSE;
            }
            return CJSON_BufferAppend(jsonBuffer, numberBuffer);
        case DATA_UINT:
            if (CJSON_FormatUnsignedInteger(numberBuffer, sizeof(numberBuffer), buildParam->uintValue) != BOOL_TRUE) {
                return BOOL_FALSE;
            }
            return CJSON_BufferAppend(jsonBuffer, numberBuffer);
        case DATA_BOOL:
            return CJSON_BufferAppend(jsonBuffer, (buildParam->boolValue == BOOL_TRUE) ? "true" : "false");
        case DATA_DOUBLE:
            if (CJSON_FormatDouble(numberBuffer, sizeof(numberBuffer), buildParam->doubleValue) != BOOL_TRUE) {
                return BOOL_FALSE;
            }
            return CJSON_BufferAppend(jsonBuffer, numberBuffer);
        case DATA_CHAR:
            return CJSON_BufferAppendEscapedCharValue(jsonBuffer, buildParam->charValue);
        default:
            return BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 构建响应JSON
 * @param[in]  : buildParam --构建参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 失败时输出缓冲区置为空字符串
 ******************************************************************************/
void CJSON_ResultBuild(const CJSONBuildParam_S* buildParam)
{
    CJSONBuffer_S jsonBuffer = {0};

    if (buildParam == NULL) {
        return;
    }

    if (CJSON_BufferInit(&jsonBuffer, buildParam->dataBuf, buildParam->dataLen) != BOOL_TRUE) {
        CJSON_ClearOutput(buildParam->dataBuf);
        return;
    }

    if (CJSON_AppendResponseHead(&jsonBuffer, buildParam->result) != BOOL_TRUE) {
        CJSON_ClearOutput(buildParam->dataBuf);
        return;
    }

    if (CJSON_AppendResponseData(&jsonBuffer, buildParam) != BOOL_TRUE) {
        CJSON_ClearOutput(buildParam->dataBuf);
        return;
    }

    if (CJSON_BufferAppendChar(&jsonBuffer, '}') != BOOL_TRUE) {
        CJSON_ClearOutput(buildParam->dataBuf);
    }
}

/******************************************************************************
 * @brief      : 查找JSON字符串结束双引号
 * @param[in]  : jsonString --JSON字符串, stringBegin --起始位置, jsonLen --JSON长度
 * @param[out] : stringEnd --结束位置
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用下标避免越界
 ******************************************************************************/
static CJSONBool_E CJSON_FindStringEndIndex(const char* jsonString, size_t stringBegin, size_t jsonLen, size_t* stringEnd)
{
    size_t index = stringBegin + 1U;

    if ((jsonString == NULL) || (stringEnd == NULL) || (stringBegin >= jsonLen) || (jsonString[stringBegin] != '\"')) {
        return BOOL_FALSE;
    }

    while (index < jsonLen) {
        if (jsonString[index] == '\\') {
            index += 2U;
            continue;
        }

        if (jsonString[index] == '\"') {
            *stringEnd = index;
            return BOOL_TRUE;
        }

        index++;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 查找下一个双引号
 * @param[in]  : jsonString --JSON字符串, jsonLen --JSON长度
 * @param[out] : index --扫描位置
 * @return     : BOOL_TRUE --找到，BOOL_FALSE --未找到
 * @note       : 只在jsonLen范围内扫描
 ******************************************************************************/
static CJSONBool_E CJSON_FindNextQuoteIndex(const char* jsonString, size_t jsonLen, size_t* index)
{
    if ((jsonString == NULL) || (index == NULL)) {
        return BOOL_FALSE;
    }

    while (*index < jsonLen) {
        if (jsonString[*index] == '\"') {
            return BOOL_TRUE;
        }

        (*index)++;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 跳过空白字符
 * @param[in]  : jsonString --JSON字符串, jsonLen --JSON长度
 * @param[out] : index --扫描位置
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只在jsonLen范围内扫描
 ******************************************************************************/
static CJSONBool_E CJSON_SkipBlankIndex(const char* jsonString, size_t jsonLen, size_t* index)
{
    if ((jsonString == NULL) || (index == NULL)) {
        return BOOL_FALSE;
    }

    while ((*index < jsonLen) && (isspace((unsigned char)jsonString[*index]) != 0)) {
        (*index)++;
    }

    return (*index < jsonLen) ? BOOL_TRUE : BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 判断键名是否匹配
 * @param[in]  : keyStart --键名起始, keyLen --键长, expectedKey --期望键
 * @param[out] : 无
 * @return     : BOOL_TRUE --匹配，BOOL_FALSE --不匹配
 * @note       : 只比较普通键名
 ******************************************************************************/
static CJSONBool_E CJSON_IsExpectedKey(const char* keyStart, size_t keyLen, const char* expectedKey)
{
    size_t expectedKeyLength = 0U;

    if ((keyStart == NULL) || (expectedKey == NULL)) {
        return BOOL_FALSE;
    }

    expectedKeyLength = strlen(expectedKey);
    if (keyLen != expectedKeyLength) {
        return BOOL_FALSE;
    }

    return (strncmp(keyStart, expectedKey, expectedKeyLength) == 0) ? BOOL_TRUE : BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 查找指定键的值起始位置
 * @param[in]  : jsonObject --JSON对象字符串, key --键名
 * @param[out] : 无
 * @return     : 值起始位置，失败返回NULL
 * @note       : 适用于本模块生成的简单对象
 ******************************************************************************/
static const char* CJSON_FindValueByKey(const char* jsonObject, const char* key)
{
    size_t jsonLen = CJSON_Strnlen(jsonObject, CJSON_MAX_SCAN_SIZE);
    size_t index = 0U;
    size_t keyEnd = 0U;
    size_t valueIndex = 0U;

    if ((jsonObject == NULL) || (key == NULL) || (jsonLen == 0U) || (jsonLen >= CJSON_MAX_SCAN_SIZE)) {
        return NULL;
    }

    while (CJSON_FindNextQuoteIndex(jsonObject, jsonLen, &index) == BOOL_TRUE) {
        if (CJSON_FindStringEndIndex(jsonObject, index, jsonLen, &keyEnd) != BOOL_TRUE) {
            return NULL;
        }

        valueIndex = keyEnd + 1U;
        if ((CJSON_SkipBlankIndex(jsonObject, jsonLen, &valueIndex) == BOOL_TRUE) && (jsonObject[valueIndex] == ':')) {
            valueIndex++;
            if (CJSON_SkipBlankIndex(jsonObject, jsonLen, &valueIndex) != BOOL_TRUE) {
                return NULL;
            }

            if (CJSON_IsExpectedKey(jsonObject + index + 1U, keyEnd - index - 1U, key) == BOOL_TRUE) {
                return jsonObject + valueIndex;
            }
        }

        index = keyEnd + 1U;
    }

    return NULL;
}

/******************************************************************************
 * @brief      : 写入JSON转义字符
 * @param[in]  : escapeChar --转义字符
 * @param[out] : output --输出缓冲区, outputIndex --输出位置
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 不处理完整Unicode反解析
 ******************************************************************************/
static CJSONBool_E CJSON_WriteParsedEscapeChar(char escapeChar, char* output, size_t* outputIndex)
{
    if ((output == NULL) || (outputIndex == NULL)) {
        return BOOL_FALSE;
    }

    switch (escapeChar) {
        case '\"':
            output[*outputIndex] = '\"';
            break;
        case '\\':
            output[*outputIndex] = '\\';
            break;
        case 'n':
            output[*outputIndex] = '\n';
            break;
        case 'r':
            output[*outputIndex] = '\r';
            break;
        case 't':
            output[*outputIndex] = '\t';
            break;
        case 'b':
            output[*outputIndex] = '\b';
            break;
        case 'f':
            output[*outputIndex] = '\f';
            break;
        default:
            output[*outputIndex] = escapeChar;
            break;
    }

    (*outputIndex)++;
    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析JSON字符串值
 * @param[in]  : valueStart --JSON字符串值起始, outputSize --输出大小
 * @param[out] : output --输出字符串
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用下标限制避免越界
 ******************************************************************************/
static CJSONBool_E CJSON_ParseJsonStringValue(const char* valueStart, char* output, size_t outputSize)
{
    size_t inputLen = CJSON_Strnlen(valueStart, CJSON_MAX_SCAN_SIZE);
    size_t inputIndex = 1U;
    size_t outputIndex = 0U;

    if ((valueStart == NULL) || (output == NULL) || (outputSize == 0U) || (inputLen == 0U) || (inputLen >= CJSON_MAX_SCAN_SIZE) || (valueStart[0] != '\"')) {
        return BOOL_FALSE;
    }

    output[0] = '\0';
    while (inputIndex < inputLen) {
        char currentChar = valueStart[inputIndex];

        if (currentChar == '\"') {
            output[outputIndex] = '\0';
            return BOOL_TRUE;
        }

        if ((outputIndex + 1U) >= outputSize) {
            return BOOL_FALSE;
        }

        if (currentChar == '\\') {
            inputIndex++;
            if (inputIndex >= inputLen) {
                return BOOL_FALSE;
            }

            if (CJSON_WriteParsedEscapeChar(valueStart[inputIndex], output, &outputIndex) != BOOL_TRUE) {
                return BOOL_FALSE;
            }
        } else {
            output[outputIndex] = currentChar;
            outputIndex++;
        }

        inputIndex++;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 解析有符号整数文本
 * @param[in]  : valueStart --数值起始
 * @param[out] : value --解析结果
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 检查strtoll返回值和尾部字符
 ******************************************************************************/
static CJSONBool_E CJSON_ParseSignedIntegerText(const char* valueStart, int64_t* value)
{
    char* endPtr = NULL;
    long long parsedValue = 0;

    if ((valueStart == NULL) || (value == NULL)) {
        return BOOL_FALSE;
    }

    errno = 0;
    parsedValue = strtoll(valueStart, &endPtr, 10);
    if ((endPtr == valueStart) || (errno == ERANGE) || (CJSON_IsJsonValueEnd(*endPtr) != BOOL_TRUE)) {
        return BOOL_FALSE;
    }

    *value = (int64_t)parsedValue;
    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析无符号整数文本
 * @param[in]  : valueStart --数值起始
 * @param[out] : value --解析结果
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 检查strtoull返回值和尾部字符
 ******************************************************************************/
static CJSONBool_E CJSON_ParseUnsignedIntegerText(const char* valueStart, uint64_t* value)
{
    char* endPtr = NULL;
    unsigned long long parsedValue = 0U;

    if ((valueStart == NULL) || (value == NULL) || (valueStart[0] == '-')) {
        return BOOL_FALSE;
    }

    errno = 0;
    parsedValue = strtoull(valueStart, &endPtr, 10);
    if ((endPtr == valueStart) || (errno == ERANGE) || (CJSON_IsJsonValueEnd(*endPtr) != BOOL_TRUE)) {
        return BOOL_FALSE;
    }

    *value = (uint64_t)parsedValue;
    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析浮点文本
 * @param[in]  : valueStart --数值起始
 * @param[out] : value --解析结果
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 检查strtold返回值和尾部字符
 ******************************************************************************/
static CJSONBool_E CJSON_ParseDoubleText(const char* valueStart, long double* value)
{
    char* endPtr = NULL;
    long double parsedValue = 0.0L;

    if ((valueStart == NULL) || (value == NULL)) {
        return BOOL_FALSE;
    }

    errno = 0;
    parsedValue = strtold(valueStart, &endPtr);
    if ((endPtr == valueStart) || (errno == ERANGE) || (CJSON_IsJsonValueEnd(*endPtr) != BOOL_TRUE)) {
        return BOOL_FALSE;
    }

    *value = parsedValue;
    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析布尔文本
 * @param[in]  : valueStart --布尔值起始
 * @param[out] : value --解析结果
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只接受true/false
 ******************************************************************************/
static CJSONBool_E CJSON_ParseBoolText(const char* valueStart, CJSONBool_E* value)
{
    if ((valueStart == NULL) || (value == NULL)) {
        return BOOL_FALSE;
    }

    if ((strncmp(valueStart, "true", 4U) == 0) && (CJSON_IsJsonValueEnd(valueStart[4]) == BOOL_TRUE)) {
        *value = BOOL_TRUE;
        return BOOL_TRUE;
    }

    if ((strncmp(valueStart, "false", 5U) == 0) && (CJSON_IsJsonValueEnd(valueStart[5]) == BOOL_TRUE)) {
        *value = BOOL_FALSE;
        return BOOL_TRUE;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 解析响应结果字段
 * @param[in]  : jsonString --JSON字符串
 * @param[out] : result --结果枚举
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只接受pass/fail
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResultField(const char* jsonString, CJSONResult_E* result)
{
    const char* resultStart = NULL;
    char resultText[16] = {0};

    if ((jsonString == NULL) || (result == NULL)) {
        return BOOL_FALSE;
    }

    resultStart = CJSON_FindValueByKey(jsonString, CJSON_KEY_RESULT);
    if (resultStart == NULL) {
        return BOOL_FALSE;
    }

    if (CJSON_ParseJsonStringValue(resultStart, resultText, sizeof(resultText)) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (strcmp(resultText, CJSON_RESULT_PASS_STRING) == 0) {
        *result = RESULT_PASS;
        return BOOL_TRUE;
    }

    if (strcmp(resultText, CJSON_RESULT_FAIL_STRING) == 0) {
        *result = RESULT_FAIL;
        return BOOL_TRUE;
    }

    return BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 解析data字符串
 * @param[in]  : dataStart --data字段起始
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 空字符串时dataType为DATA_EMPTY
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResponseStringData(const char* dataStart, CJSONResponse_S* response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    if ((dataStart[0] == '\"') && (dataStart[1] == '\"')) {
        response->dataType = DATA_EMPTY;
        if (response->dataString != NULL) {
            response->dataString[0] = '\0';
        }
        return BOOL_TRUE;
    }

    if (response->dataString == NULL) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_STRING;
    return CJSON_ParseJsonStringValue(dataStart, response->dataString, CJSON_RESPONSE_STRING_BUFFER_SIZE);
}

/******************************************************************************
 * @brief      : 解析JSON字符值
 * @param[in]  : valueStart --JSON值起始
 * @param[out] : charValue --解析出的字符
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输入格式为"A"
 ******************************************************************************/
static CJSONBool_E CJSON_ParseJsonCharValue(const char* valueStart, char* charValue)
{
    char buffer[2] = {0};

    if (charValue == NULL) {
        return BOOL_FALSE;
    }

    if (CJSON_ParseJsonStringValue(valueStart, buffer, sizeof(buffer)) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    *charValue = buffer[0];
    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析字段值
 * @param[in]  : valueStart --JSON值起始, field --字段描述
 * @param[out] : fieldAddress --字段地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段类型分发
 ******************************************************************************/
static CJSONBool_E CJSON_ParseFieldValue(const char* valueStart, void* fieldAddress, const CJSONField_S* field)
{
    int64_t signedValue = 0;
    uint64_t unsignedValue = 0U;
    long double doubleValue = 0.0L;
    CJSONBool_E boolValue = BOOL_FALSE;

    if ((valueStart == NULL) || (fieldAddress == NULL) || (field == NULL)) {
        return BOOL_FALSE;
    }

    switch (field->fieldType) {
        case FIELD_INT:
            if (CJSON_ParseSignedIntegerText(valueStart, &signedValue) != BOOL_TRUE) {
                return BOOL_FALSE;
            }
            return CJSON_WriteSignedInteger(fieldAddress, field->fieldSize, signedValue);
        case FIELD_UINT:
            if (CJSON_ParseUnsignedIntegerText(valueStart, &unsignedValue) != BOOL_TRUE) {
                return BOOL_FALSE;
            }
            return CJSON_WriteUnsignedInteger(fieldAddress, field->fieldSize, unsignedValue);
        case FIELD_BOOL:
            if (CJSON_ParseBoolText(valueStart, &boolValue) != BOOL_TRUE) {
                return BOOL_FALSE;
            }
            if (field->fieldSize == sizeof(CJSONBool_E)) {
                *(CJSONBool_E*)fieldAddress = boolValue;
            } else {
                *(_Bool*)fieldAddress = (boolValue == BOOL_TRUE);
            }
            return BOOL_TRUE;
        case FIELD_STRING:
            return CJSON_ParseJsonStringValue(valueStart, (char*)fieldAddress, field->fieldSize);
        case FIELD_DOUBLE:
            if (CJSON_ParseDoubleText(valueStart, &doubleValue) != BOOL_TRUE) {
                return BOOL_FALSE;
            }
            return CJSON_WriteFloatingPoint(fieldAddress, field->fieldSize, doubleValue);
        case FIELD_CHAR:
            return CJSON_ParseJsonCharValue(valueStart, (char*)fieldAddress);
        default:
            return BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 反序列化结构体对象
 * @param[in]  : jsonObject --JSON对象字符串, fields --字段描述表
 * @param[out] : object --对象地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只支持扁平对象
 ******************************************************************************/
static CJSONBool_E CJSON_DeserializeStructObject(const char* jsonObject, void* object, const CJSONField_S* fields)
{
    const char* valueStart = NULL;
    size_t fieldIndex = 0U;

    if ((jsonObject == NULL) || (object == NULL) || (fields == NULL)) {
        return BOOL_FALSE;
    }

    for (fieldIndex = 0U; fields[fieldIndex].jsonKey != NULL; fieldIndex++) {
        valueStart = CJSON_FindValueByKey(jsonObject, fields[fieldIndex].jsonKey);
        if (valueStart == NULL) {
            return BOOL_FALSE;
        }

        if (CJSON_ParseFieldValue(valueStart, (unsigned char*)object + fields[fieldIndex].fieldOffset, &fields[fieldIndex]) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析空data
 * @param[in]  : dataStart --data字段起始
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只接受空字符串data
 ******************************************************************************/
static CJSONBool_E CJSON_ParseEmptyResponseData(const char* dataStart, CJSONResponse_S* response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    response->dataString = NULL;
    response->dataObject = NULL;

    return CJSON_ParseResponseStringData(dataStart, response);
}

/******************************************************************************
 * @brief      : 解析字符串data
 * @param[in]  : dataStart --data字段起始, parseParam --解析参数
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 字符串缓冲区由parseParam->data提供
 ******************************************************************************/
static CJSONBool_E CJSON_ParseStringResponseData(const char* dataStart, CJSONResponse_S* response, const CJSONParseParam_S* parseParam)
{
    if ((dataStart == NULL) || (response == NULL) || (parseParam == NULL)) {
        return BOOL_FALSE;
    }

    response->dataString = (char*)parseParam->data;
    response->dataObject = NULL;

    return CJSON_ParseResponseStringData(dataStart, response);
}

/******************************************************************************
 * @brief      : 解析对象data
 * @param[in]  : dataStart --data字段起始, parseParam --解析参数
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 对象地址和字段表由parseParam提供
 ******************************************************************************/
static CJSONBool_E CJSON_ParseObjectResponseData(const char* dataStart, CJSONResponse_S* response, const CJSONParseParam_S* parseParam)
{
    if ((dataStart == NULL) || (response == NULL) || (parseParam == NULL)) {
        return BOOL_FALSE;
    }

    if ((parseParam->data == NULL) || (parseParam->fields == NULL) || (*dataStart != '{')) {
        return BOOL_FALSE;
    }

    if (CJSON_DeserializeStructObject(dataStart, parseParam->data, parseParam->fields) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_OBJECT;
    response->dataString = NULL;
    response->dataObject = parseParam->data;

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析有符号整数data
 * @param[in]  : dataStart --data字段起始
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用安全封装解析strtoll
 ******************************************************************************/
static CJSONBool_E CJSON_ParseIntResponseData(const char* dataStart, CJSONResponse_S* response)
{
    int64_t value = 0;

    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    if (CJSON_ParseSignedIntegerText(dataStart, &value) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_INT;
    response->intValue = value;

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析无符号整数data
 * @param[in]  : dataStart --data字段起始
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用安全封装解析strtoull
 ******************************************************************************/
static CJSONBool_E CJSON_ParseUIntResponseData(const char* dataStart, CJSONResponse_S* response)
{
    uint64_t value = 0U;

    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    if (CJSON_ParseUnsignedIntegerText(dataStart, &value) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_UINT;
    response->uintValue = value;

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析布尔data
 * @param[in]  : dataStart --data字段起始
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只接受true/false
 ******************************************************************************/
static CJSONBool_E CJSON_ParseBoolResponseData(const char* dataStart, CJSONResponse_S* response)
{
    CJSONBool_E value = BOOL_FALSE;

    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    if (CJSON_ParseBoolText(dataStart, &value) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_BOOL;
    response->boolValue = value;

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析浮点data
 * @param[in]  : dataStart --data字段起始
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用安全封装解析strtold
 ******************************************************************************/
static CJSONBool_E CJSON_ParseDoubleResponseData(const char* dataStart, CJSONResponse_S* response)
{
    long double value = 0.0L;

    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    if (CJSON_ParseDoubleText(dataStart, &value) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_DOUBLE;
    response->doubleValue = value;

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析字符data
 * @param[in]  : dataStart --data字段起始
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 字符按JSON字符串解析
 ******************************************************************************/
static CJSONBool_E CJSON_ParseCharResponseData(const char* dataStart, CJSONResponse_S* response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_CHAR;

    return CJSON_ParseJsonCharValue(dataStart, &response->charValue);
}

/******************************************************************************
 * @brief      : 解析data字段
 * @param[in]  : dataStart --data字段起始, parseParam --解析参数
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只负责分发
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResponseData(const char* dataStart, const CJSONParseParam_S* parseParam)
{
    CJSONResponse_S* response = NULL;

    if ((dataStart == NULL) || (parseParam == NULL) || (parseParam->response == NULL)) {
        return BOOL_FALSE;
    }

    response = parseParam->response;
    switch (parseParam->dataType) {
        case DATA_EMPTY:
            return CJSON_ParseEmptyResponseData(dataStart, response);
        case DATA_STRING:
            return CJSON_ParseStringResponseData(dataStart, response, parseParam);
        case DATA_OBJECT:
            return CJSON_ParseObjectResponseData(dataStart, response, parseParam);
        case DATA_INT:
            return CJSON_ParseIntResponseData(dataStart, response);
        case DATA_UINT:
            return CJSON_ParseUIntResponseData(dataStart, response);
        case DATA_BOOL:
            return CJSON_ParseBoolResponseData(dataStart, response);
        case DATA_DOUBLE:
            return CJSON_ParseDoubleResponseData(dataStart, response);
        case DATA_CHAR:
            return CJSON_ParseCharResponseData(dataStart, response);
        default:
            return BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 解析响应JSON
 * @param[in]  : parseParam --解析参数
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持空、字符串、对象、整数、布尔、浮点和字符
 ******************************************************************************/
CJSONBool_E CJSON_ResultParse(const CJSONParseParam_S* parseParam)
{
    const char* dataStart = NULL;

    if ((parseParam == NULL) || (parseParam->jsonString == NULL) || (parseParam->response == NULL)) {
        return BOOL_FALSE;
    }

    if (CJSON_ParseResultField(parseParam->jsonString, &parseParam->response->result) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    dataStart = CJSON_FindValueByKey(parseParam->jsonString, CJSON_KEY_DATA);
    if (dataStart == NULL) {
        return BOOL_FALSE;
    }

    if (CJSON_ParseResponseData(dataStart, parseParam) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    return (parseParam->response->dataType == parseParam->dataType) ? BOOL_TRUE : BOOL_FALSE;
}

/******************************************************************************
 * File Name : cjson_response.c
 * Function  : 响应JSON构造、解析和结构体字段注册接口实现。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#include "cjson_response.h"
#include "securec.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define CJSON_KEY_RESULT ("result")
#define CJSON_KEY_DATA ("data")
#define CJSON_RESULT_PASS_STRING ("pass")
#define CJSON_RESULT_FAIL_STRING ("fail")
#define CJSON_STRING_EMPTY ("")
#define CJSON_NUMBER_BUFFER_SIZE (64U)
#define CJSON_RESPONSE_STRING_BUFFER_SIZE (256U)
#define CJSON_MAX_SCAN_SIZE (4096U)
#define CJSON_SNPRINTF_COUNT(destMax) (((destMax) > 0U) ? ((destMax) - 1U) : 0U)

typedef struct {
    char *buffer;
    size_t length;
    size_t capacity;
} CJSONBuffer_S;

/******************************************************************************
 * @brief      : 构建参数设置为有符号整数
 * @param[in]  : value --有符号整数值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_RESULT_BUILD宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetInt(CJSONBuildParam_S *buildParam, int64_t value)
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
 * @note       : 供FT_RESULT_BUILD宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetUInt(CJSONBuildParam_S *buildParam, uint64_t value)
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
 * @note       : 供FT_RESULT_BUILD宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetBool(CJSONBuildParam_S *buildParam, int value)
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
 * @note       : 供FT_RESULT_BUILD宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetDouble(CJSONBuildParam_S *buildParam, long double value)
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
 * @note       : 供FT_RESULT_BUILD宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetChar(CJSONBuildParam_S *buildParam, char value)
{
    if (buildParam != NULL) {
        buildParam->charValue = value;
    }
}

/******************************************************************************
 * @brief      : 构建参数设置为指针数据
 * @param[in]  : value --字符串或对象指针
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_RESULT_BUILD宏内部使用
 ******************************************************************************/
void CJSON_BuildParamSetPointer(CJSONBuildParam_S *buildParam, const void *value)
{
    if (buildParam != NULL) {
        buildParam->data = value;
    }
}

/******************************************************************************
 * @brief      : 内存拷贝封装
 * @param[in]  : destMax --目标缓冲区最大长度, src --源地址, count --拷贝长度
 * @param[out] : dest --目标地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 内部调用memcpy_s并统一判断返回值
 ******************************************************************************/
static CJSONBool_E CJSON_CopyMemory(void *dest, size_t destMax, const void *src, size_t count)
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
 * @note       : 构建失败时用于输出空字符串
 ******************************************************************************/
static void CJSON_ClearOutput(char *dataBuf)
{
    if (dataBuf != NULL) {
        dataBuf[0] = '\0';
    }
}

/******************************************************************************
 * @brief      : 获取有限长度字符串长度
 * @param[in]  : input --输入字符串, maxLen --最大扫描长度
 * @param[out] : 无
 * @return     : 字符串长度；未在maxLen内遇到'\0'时返回maxLen
 * @note       : 避免异常输入导致无界扫描
 ******************************************************************************/
static size_t CJSON_Strnlen(const char *input, size_t maxLen)
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
 * @brief      : 格式化到缓冲区
 * @param[in]  : format --格式化字符串
 * @param[out] : buffer --输出缓冲区
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 内部调用snprintf_s并判断返回值
 ******************************************************************************/
#define CJSON_FORMAT_TO_BUFFER(buffer, bufferSize, format, value)                                          \
    do {                                                                                                   \
        if (((buffer) == NULL) || ((bufferSize) == 0U)) {                                                  \
            return BOOL_FALSE;                                                                             \
        }                                                                                                  \
        (buffer)[0] = '\0';                                                                                \
        if (snprintf_s((buffer), (bufferSize), CJSON_SNPRINTF_COUNT(bufferSize), (format), (value)) < 0) { \
            (buffer)[0] = '\0';                                                                            \
            return BOOL_FALSE;                                                                             \
        }                                                                                                  \
    } while (0)

/******************************************************************************
 * @brief      : 获取响应结果字符串
 * @param[in]  : result --结果枚举
 * @param[out] : 无
 * @return     : 返回pass/fail字符串
 * @note       : 返回静态字符串，不需要释放
 ******************************************************************************/
static const char *CJSON_GetResultString(CJSONResult_E result)
{
    return (result == RESULT_PASS) ? CJSON_RESULT_PASS_STRING : CJSON_RESULT_FAIL_STRING;
}

/******************************************************************************
 * @brief      : 初始化外部JSON缓冲区
 * @param[in]  : dataBuf --外部缓冲区, dataLen --外部缓冲区长度
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 不申请内存，只绑定外部缓冲区
 ******************************************************************************/
static CJSONBool_E CJSON_BufferInit(CJSONBuffer_S *jsonBuffer, char *dataBuf, size_t dataLen)
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
 * @brief      : 检查外部缓冲区剩余空间是否足够
 * @param[in]  : needSize --需要追加的字节数，不包含结尾'\0'
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 外部缓冲区模式不扩容，空间不足直接失败
 ******************************************************************************/
static CJSONBool_E CJSON_BufferEnsure(CJSONBuffer_S *jsonBuffer, size_t needSize)
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
static CJSONBool_E CJSON_BufferAppend(CJSONBuffer_S *jsonBuffer, const char *text)
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
 * @param[in]  : value --待追加字符
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 会自动维护字符串结尾'\0'
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendChar(CJSONBuffer_S *jsonBuffer, char value)
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
 * @param[in]  : value --待追加字符
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 处理JSON字符串内部常见转义字符
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendEscapedChar(CJSONBuffer_S *jsonBuffer, unsigned char value)
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
        CJSON_FORMAT_TO_BUFFER(unicodeBuffer, sizeof(unicodeBuffer), "\\u%04x", value);
        return CJSON_BufferAppend(jsonBuffer, unicodeBuffer);
    }

    return CJSON_BufferAppendChar(jsonBuffer, (char)value);
}

/******************************************************************************
 * @brief      : 追加转义后的JSON字符串值
 * @param[in]  : text --普通字符串
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 会自动添加双引号
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendEscapedString(CJSONBuffer_S *jsonBuffer, const char *text)
{
    const unsigned char *current = NULL;

    text = (text == NULL) ? CJSON_STRING_EMPTY : text;
    if (CJSON_BufferAppendChar(jsonBuffer, '\"') != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    for (current = (const unsigned char *)text; *current != '\0'; current++) {
        if (CJSON_BufferAppendEscapedChar(jsonBuffer, *current) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
    }

    return CJSON_BufferAppendChar(jsonBuffer, '\"');
}

/******************************************************************************
 * @brief      : 追加单字符JSON字符串值
 * @param[in]  : value --字符值
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输出格式为"A"，并处理必要的JSON转义
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendEscapedCharValue(CJSONBuffer_S *jsonBuffer, char value)
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
 * @param[out] : value --读取到的值
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段大小读取int8_t/int16_t/int32_t/int64_t等类型
 ******************************************************************************/
static CJSONBool_E CJSON_ReadSignedInteger(const void *fieldAddress, size_t fieldSize, int64_t *value)
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
 * @param[out] : value --读取到的值
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段大小读取uint8_t/uint16_t/uint32_t/uint64_t等类型
 ******************************************************************************/
static CJSONBool_E CJSON_ReadUnsignedInteger(const void *fieldAddress, size_t fieldSize, uint64_t *value)
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
 * @param[out] : value --读取到的值
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段大小读取float/double/long double
 ******************************************************************************/
static CJSONBool_E CJSON_ReadFloatingPoint(const void *fieldAddress, size_t fieldSize, long double *value)
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
 * @param[in]  : fieldSize --字段大小, value --待写入值
 * @param[out] : fieldAddress --字段地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段大小写入int8_t/int16_t/int32_t/int64_t等类型
 ******************************************************************************/
static CJSONBool_E CJSON_WriteSignedInteger(void *fieldAddress, size_t fieldSize, int64_t value)
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
 * @param[in]  : fieldSize --字段大小, value --待写入值
 * @param[out] : fieldAddress --字段地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段大小写入uint8_t/uint16_t/uint32_t/uint64_t等类型
 ******************************************************************************/
static CJSONBool_E CJSON_WriteUnsignedInteger(void *fieldAddress, size_t fieldSize, uint64_t value)
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
 * @param[in]  : fieldSize --字段大小, value --待写入值
 * @param[out] : fieldAddress --字段地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段大小写入float/double/long double
 ******************************************************************************/
static CJSONBool_E CJSON_WriteFloatingPoint(void *fieldAddress, size_t fieldSize, long double value)
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
static CJSONBool_E CJSON_AppendIntegerField(CJSONBuffer_S *jsonBuffer, const void *fieldAddress, const CJSONField_S *field)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};
    int64_t signedValue = 0;
    uint64_t unsignedValue = 0U;

    if (field->fieldType == FIELD_INT) {
        if (CJSON_ReadSignedInteger(fieldAddress, field->fieldSize, &signedValue) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        CJSON_FORMAT_TO_BUFFER(numberBuffer, sizeof(numberBuffer), "%" PRId64, signedValue);
    } else {
        if (CJSON_ReadUnsignedInteger(fieldAddress, field->fieldSize, &unsignedValue) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
        CJSON_FORMAT_TO_BUFFER(numberBuffer, sizeof(numberBuffer), "%" PRIu64, unsignedValue);
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
static CJSONBool_E CJSON_AppendDoubleField(CJSONBuffer_S *jsonBuffer, const void *fieldAddress, const CJSONField_S *field)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};
    long double doubleValue = 0.0L;

    if (CJSON_ReadFloatingPoint(fieldAddress, field->fieldSize, &doubleValue) != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    CJSON_FORMAT_TO_BUFFER(numberBuffer, sizeof(numberBuffer), "%.21Lg", doubleValue);
    return CJSON_BufferAppend(jsonBuffer, numberBuffer);
}

/******************************************************************************
 * @brief      : 追加布尔字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持_Bool和CJSONBool_E
 ******************************************************************************/
static CJSONBool_E CJSON_AppendBoolField(CJSONBuffer_S *jsonBuffer, const void *fieldAddress, const CJSONField_S *field)
{
    CJSONBool_E boolValue = BOOL_FALSE;

    if ((jsonBuffer == NULL) || (fieldAddress == NULL) || (field == NULL)) {
        return BOOL_FALSE;
    }

    if (field->fieldSize == sizeof(CJSONBool_E)) {
        boolValue = (*(const CJSONBool_E *)fieldAddress == BOOL_TRUE) ? BOOL_TRUE : BOOL_FALSE;
    } else {
        boolValue = (*(const _Bool *)fieldAddress) ? BOOL_TRUE : BOOL_FALSE;
    }

    return CJSON_BufferAppend(jsonBuffer, (boolValue == BOOL_TRUE) ? "true" : "false");
}

/******************************************************************************
 * @brief      : 追加字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段类型分发到具体序列化逻辑
 ******************************************************************************/
static CJSONBool_E CJSON_AppendFieldValue(CJSONBuffer_S *jsonBuffer, const void *fieldAddress, const CJSONField_S *field)
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
            return CJSON_BufferAppendEscapedString(jsonBuffer, (const char *)fieldAddress);
        case FIELD_DOUBLE:
            return CJSON_AppendDoubleField(jsonBuffer, fieldAddress, field);
        case FIELD_CHAR:
            return CJSON_BufferAppendEscapedCharValue(jsonBuffer, *(const char *)fieldAddress);
        default:
            return BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 根据字段描述追加一个结构体字段
 * @param[in]  : object --结构体地址, field --字段描述, isFirst --是否为第一个字段
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输出格式为"key":value
 ******************************************************************************/
static CJSONBool_E CJSON_AppendStructField(CJSONBuffer_S *jsonBuffer, const void *object, const CJSONField_S *field, CJSONBool_E isFirst)
{
    const void *fieldAddress = NULL;

    if ((jsonBuffer == NULL) || (object == NULL) || (field == NULL) || (field->jsonKey == NULL)) {
        return BOOL_FALSE;
    }

    fieldAddress = (const unsigned char *)object + field->fieldOffset;
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
 * @param[in]  : object --结构体地址, fields --字段描述表
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 直接追加{"key":value,...}，不申请临时堆内存
 ******************************************************************************/
static CJSONBool_E CJSON_AppendStructObject(CJSONBuffer_S *jsonBuffer, const void *object, const CJSONField_S *fields)
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
 * @brief      : 追加响应JSON头部
 * @param[in]  : result --结果枚举
 * @param[out] : jsonBuffer --JSON缓冲区对象
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输出{"result":"pass/fail","data":
 ******************************************************************************/
static CJSONBool_E CJSON_AppendResponseHead(CJSONBuffer_S *jsonBuffer, CJSONResult_E result)
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
 * @note       : 根据dataType追加空字符串、字符串、对象、整数、布尔值、浮点数或字符
 ******************************************************************************/
static CJSONBool_E CJSON_AppendResponseData(CJSONBuffer_S *jsonBuffer, const CJSONBuildParam_S *buildParam)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};

    if ((jsonBuffer == NULL) || (buildParam == NULL)) {
        return BOOL_FALSE;
    }

    switch (buildParam->dataType) {
        case DATA_EMPTY:
            return CJSON_BufferAppendEscapedString(jsonBuffer, CJSON_STRING_EMPTY);
        case DATA_STRING:
            return CJSON_BufferAppendEscapedString(jsonBuffer, (const char *)buildParam->data);
        case DATA_OBJECT:
            return CJSON_AppendStructObject(jsonBuffer, buildParam->data, buildParam->fields);
        case DATA_INT:
            CJSON_FORMAT_TO_BUFFER(numberBuffer, sizeof(numberBuffer), "%" PRId64, buildParam->intValue);
            return CJSON_BufferAppend(jsonBuffer, numberBuffer);
        case DATA_UINT:
            CJSON_FORMAT_TO_BUFFER(numberBuffer, sizeof(numberBuffer), "%" PRIu64, buildParam->uintValue);
            return CJSON_BufferAppend(jsonBuffer, numberBuffer);
        case DATA_BOOL:
            return CJSON_BufferAppend(jsonBuffer, (buildParam->boolValue == BOOL_TRUE) ? "true" : "false");
        case DATA_DOUBLE:
            CJSON_FORMAT_TO_BUFFER(numberBuffer, sizeof(numberBuffer), "%.21Lg", buildParam->doubleValue);
            return CJSON_BufferAppend(jsonBuffer, numberBuffer);
        case DATA_CHAR:
            return CJSON_BufferAppendEscapedCharValue(jsonBuffer, buildParam->charValue);
        default:
            return BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 构建响应JSON到外部缓冲区
 * @param[in]  : buildParam --构建参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 构建失败时buildParam->dataBuf会被置为空字符串
 ******************************************************************************/
void CJSON_ResultBuild(const CJSONBuildParam_S *buildParam)
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
        return;
    }
}

/******************************************************************************
 * @brief      : 查找JSON字符串结束双引号
 * @param[in]  : jsonString --JSON字符串, stringBegin --起始双引号位置, jsonLen --JSON字符串长度
 * @param[out] : stringEnd --结束双引号位置
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 通过下标和长度限制避免越界读取
 ******************************************************************************/
static CJSONBool_E CJSON_FindStringEndIndex(const char *jsonString, size_t stringBegin, size_t jsonLen, size_t *stringEnd)
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
 * @brief      : 查找下一个JSON字符串起始双引号
 * @param[in]  : jsonString --JSON字符串, jsonLen --JSON字符串长度
 * @param[out] : index --输入扫描起始位置，输出双引号位置
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只在jsonLen范围内扫描
 ******************************************************************************/
static CJSONBool_E CJSON_FindNextQuoteIndex(const char *jsonString, size_t jsonLen, size_t *index)
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
 * @param[in]  : jsonString --JSON字符串, jsonLen --JSON字符串长度
 * @param[out] : index --输入起始位置，输出非空白位置
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只在jsonLen范围内扫描
 ******************************************************************************/
static CJSONBool_E CJSON_SkipBlankIndex(const char *jsonString, size_t jsonLen, size_t *index)
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
 * @param[in]  : keyStart --键名起始位置, keyLen --键名长度, expectedKey --期望键名
 * @param[out] : 无
 * @return     : BOOL_TRUE --匹配，BOOL_FALSE --不匹配
 * @note       : 只比较普通键名
 ******************************************************************************/
static CJSONBool_E CJSON_IsExpectedKey(const char *keyStart, size_t keyLen, const char *expectedKey)
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
 * @brief      : 从JSON对象中查找指定键对应的值起始位置
 * @param[in]  : jsonObject --JSON对象字符串, key --键名
 * @param[out] : 无
 * @return     : 返回值起始位置，失败返回NULL
 * @note       : 只处理扁平对象，不支持嵌套键查找
 ******************************************************************************/
static const char *CJSON_FindValueByKey(const char *jsonObject, const char *key)
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
 * @brief      : 写入解析后的JSON转义字符
 * @param[in]  : escapeChar --转义字符
 * @param[out] : output --输出缓冲区, outputIndex --输出位置
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 不处理完整Unicode反解析
 ******************************************************************************/
static CJSONBool_E CJSON_WriteParsedEscapeChar(char escapeChar, char *output, size_t *outputIndex)
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
 * @brief      : 解析JSON字符串值到缓冲区
 * @param[in]  : valueStart --JSON字符串值起始位置, outputSize --输出缓冲区大小
 * @param[out] : output --输出字符串
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用下标和长度边界解析，避免越界读取
 ******************************************************************************/
static CJSONBool_E CJSON_ParseJsonStringValue(const char *valueStart, char *output, size_t outputSize)
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
 * @brief      : 解析响应结果字段
 * @param[in]  : jsonString --响应JSON字符串
 * @param[out] : result --响应结果枚举
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只接受pass/fail字符串
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResultField(const char *jsonString, CJSONResult_E *result)
{
    const char *resultStart = NULL;
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
 * @brief      : 解析data字符串到响应结构体
 * @param[in]  : dataStart --data字段值起始位置
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : data为空字符串时不要求response->dataString非空
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResponseStringData(const char *dataStart, CJSONResponse_S *response)
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
 * @param[in]  : valueStart --JSON值起始位置
 * @param[out] : charValue --解析出的字符
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 输入格式必须是单字符字符串，例如"A"
 ******************************************************************************/
static CJSONBool_E CJSON_ParseJsonCharValue(const char *valueStart, char *charValue)
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
 * @param[in]  : valueStart --JSON值起始位置, field --字段描述
 * @param[out] : fieldAddress --字段地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 根据字段类型分发到具体解析逻辑
 ******************************************************************************/
static CJSONBool_E CJSON_ParseFieldValue(const char *valueStart, void *fieldAddress, const CJSONField_S *field)
{
    CJSONBool_E boolValue = BOOL_FALSE;

    if ((valueStart == NULL) || (fieldAddress == NULL) || (field == NULL)) {
        return BOOL_FALSE;
    }

    switch (field->fieldType) {
        case FIELD_INT:
            return CJSON_WriteSignedInteger(fieldAddress, field->fieldSize, (int64_t)strtoll(valueStart, NULL, 10));
        case FIELD_UINT:
            return CJSON_WriteUnsignedInteger(fieldAddress, field->fieldSize, (uint64_t)strtoull(valueStart, NULL, 10));
        case FIELD_BOOL:
            boolValue = (strncmp(valueStart, "true", 4) == 0) ? BOOL_TRUE : BOOL_FALSE;
            if (field->fieldSize == sizeof(CJSONBool_E)) {
                *(CJSONBool_E *)fieldAddress = boolValue;
            } else {
                *(_Bool *)fieldAddress = (boolValue == BOOL_TRUE);
            }
            return BOOL_TRUE;
        case FIELD_STRING:
            return CJSON_ParseJsonStringValue(valueStart, (char *)fieldAddress, field->fieldSize);
        case FIELD_DOUBLE:
            return CJSON_WriteFloatingPoint(fieldAddress, field->fieldSize, strtold(valueStart, NULL));
        case FIELD_CHAR:
            return CJSON_ParseJsonCharValue(valueStart, (char *)fieldAddress);
        default:
            return BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 根据字段描述把JSON对象反序列化到结构体
 * @param[in]  : jsonObject --JSON对象字符串, fields --字段描述表
 * @param[out] : object --结构体地址
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只支持扁平对象，如{"key1":1,"key2":"abc"}
 ******************************************************************************/
static CJSONBool_E CJSON_DeserializeStructObject(const char *jsonObject, void *object, const CJSONField_S *fields)
{
    const char *valueStart = NULL;
    size_t fieldIndex = 0U;

    if ((jsonObject == NULL) || (object == NULL) || (fields == NULL)) {
        return BOOL_FALSE;
    }

    for (fieldIndex = 0U; fields[fieldIndex].jsonKey != NULL; fieldIndex++) {
        valueStart = CJSON_FindValueByKey(jsonObject, fields[fieldIndex].jsonKey);
        if (valueStart == NULL) {
            return BOOL_FALSE;
        }

        if (CJSON_ParseFieldValue(valueStart, (unsigned char *)object + fields[fieldIndex].fieldOffset, &fields[fieldIndex]) != BOOL_TRUE) {
            return BOOL_FALSE;
        }
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析data为空字符串的响应字段
 * @param[in]  : dataStart --data字段值起始位置
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只接受JSON字符串形式的空data
 ******************************************************************************/
static CJSONBool_E CJSON_ParseEmptyResponseData(const char *dataStart, CJSONResponse_S *response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    response->dataString = NULL;
    response->dataObject = NULL;

    return CJSON_ParseResponseStringData(dataStart, response);
}

/******************************************************************************
 * @brief      : 解析data为字符串的响应字段
 * @param[in]  : dataStart --data字段值起始位置, parseParam --解析参数
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 字符串输出缓冲区由parseParam->data提供
 ******************************************************************************/
static CJSONBool_E CJSON_ParseStringResponseData(const char *dataStart, CJSONResponse_S *response, const CJSONParseParam_S *parseParam)
{
    if ((dataStart == NULL) || (response == NULL) || (parseParam == NULL)) {
        return BOOL_FALSE;
    }

    response->dataString = (char *)parseParam->data;
    response->dataObject = NULL;

    return CJSON_ParseResponseStringData(dataStart, response);
}

/******************************************************************************
 * @brief      : 解析data为对象的响应字段
 * @param[in]  : dataStart --data字段值起始位置, parseParam --解析参数
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 对象输出地址和字段描述表由parseParam提供
 ******************************************************************************/
static CJSONBool_E CJSON_ParseObjectResponseData(const char *dataStart, CJSONResponse_S *response, const CJSONParseParam_S *parseParam)
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
 * @brief      : 解析data为有符号整数的响应字段
 * @param[in]  : dataStart --data字段值起始位置
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用strtoll解析十进制整数
 ******************************************************************************/
static CJSONBool_E CJSON_ParseIntResponseData(const char *dataStart, CJSONResponse_S *response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_INT;
    response->intValue = (int64_t)strtoll(dataStart, NULL, 10);

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析data为无符号整数的响应字段
 * @param[in]  : dataStart --data字段值起始位置
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用strtoull解析十进制整数
 ******************************************************************************/
static CJSONBool_E CJSON_ParseUIntResponseData(const char *dataStart, CJSONResponse_S *response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_UINT;
    response->uintValue = (uint64_t)strtoull(dataStart, NULL, 10);

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析data为布尔值的响应字段
 * @param[in]  : dataStart --data字段值起始位置
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只把true解析为BOOL_TRUE，其他值解析为BOOL_FALSE
 ******************************************************************************/
static CJSONBool_E CJSON_ParseBoolResponseData(const char *dataStart, CJSONResponse_S *response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_BOOL;
    response->boolValue = (strncmp(dataStart, "true", 4) == 0) ? BOOL_TRUE : BOOL_FALSE;

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析data为浮点数的响应字段
 * @param[in]  : dataStart --data字段值起始位置
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 使用strtold解析浮点数
 ******************************************************************************/
static CJSONBool_E CJSON_ParseDoubleResponseData(const char *dataStart, CJSONResponse_S *response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_DOUBLE;
    response->doubleValue = strtold(dataStart, NULL);

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析data为字符的响应字段
 * @param[in]  : dataStart --data字段值起始位置
 * @param[out] : response --响应结构体
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 字符按JSON字符串解析，例如"A"
 ******************************************************************************/
static CJSONBool_E CJSON_ParseCharResponseData(const char *dataStart, CJSONResponse_S *response)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return BOOL_FALSE;
    }

    response->dataType = DATA_CHAR;

    return CJSON_ParseJsonCharValue(dataStart, &response->charValue);
}

/******************************************************************************
 * @brief      : 解析响应data字段
 * @param[in]  : dataStart --data字段值起始位置, parseParam --解析参数
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 只负责按dataType分发，具体解析由小函数完成
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResponseData(const char *dataStart, const CJSONParseParam_S *parseParam)
{
    CJSONResponse_S *response = NULL;

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
 * @note       : 支持空字符串、字符串、对象、整数、无符号整数、布尔值、浮点数和字符
 ******************************************************************************/
CJSONBool_E CJSON_ResultParse(const CJSONParseParam_S *parseParam)
{
    const char *dataStart = NULL;

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

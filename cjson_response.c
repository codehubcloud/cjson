/******************************************************************************
 * File Name : cjson_response.c
 * Function  : 响应JSON构造、解析、结构体序列化/反序列化、批量字段宏和日志输出接口实现。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#include "cjson_response.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 初始化缓冲区大小 */
#define CJSON_INIT_BUFFER_SIZE            (256U)

/* 缓冲区增长因子 */
#define CJSON_GROW_FACTOR                 (2U)

/* 数字临时字符串缓冲区大小 */
#define CJSON_NUMBER_BUFFER_SIZE          (64U)

/* 解析data字符串时的默认输出缓冲区大小。注意：接口当前没有传入dataString大小，只能保持兼容处理。 */
#define CJSON_RESPONSE_STRING_BUFFER_SIZE (256U)

/* 日志字段缓冲区大小 */
#define CJSON_LOG_LEVEL_BUFFER_SIZE       (16U)
#define CJSON_LOG_MODULE_BUFFER_SIZE      (64U)
#define CJSON_LOG_MESSAGE_BUFFER_SIZE     (160U)

/* JSON动态字符串缓冲区结构体 */
typedef struct {
    char* buffer;
    size_t length;
    size_t capacity;
} CJSONBuffer_S;

/* 日志data对象结构体 */
typedef struct {
    char level[CJSON_LOG_LEVEL_BUFFER_SIZE];
    char module[CJSON_LOG_MODULE_BUFFER_SIZE];
    char message[CJSON_LOG_MESSAGE_BUFFER_SIZE];
} CJSONLogData_S;

/* 日志data对象字段描述表 */
CJSON_FIELD_LIST_BEGIN(g_cjsonLogFields)
CJSON_FIELD_STRING_DESC(CJSONLogData_S, level, "level"), CJSON_FIELD_STRING_DESC(CJSONLogData_S, module, "module"),
    CJSON_FIELD_STRING_DESC(CJSONLogData_S, message, "message"), CJSON_FIELD_LIST_END();

/******************************************************************************
 * @brief      : 释放JSON字符串
 * @param[in]  : jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 只释放本模块返回的堆内存字符串
 ******************************************************************************/
void CJSON_Free(char* jsonString)
{
    if (jsonString != NULL) {
        free(jsonString);
    }
}

/******************************************************************************
 * @brief      : 获取响应结果字符串
 * @param[in]  : result --结果枚举
 * @param[out] : 无
 * @return     : 返回pass/fail字符串
 * @note       : 返回静态字符串，不需要释放
 ******************************************************************************/
static const char* CJSON_GetResultString(CJSONResult_E result)
{
    if (result == CJSON_RESULT_PASS) {
        return CJSON_RESULT_PASS_STRING;
    }

    return CJSON_RESULT_FAIL_STRING;
}

/******************************************************************************
 * @brief      : 获取日志级别字符串
 * @param[in]  : level --日志级别
 * @param[out] : 无
 * @return     : 返回日志级别字符串
 * @note       : 返回静态字符串，不需要释放
 ******************************************************************************/
static const char* CJSON_GetLogLevelString(CJSONLogLevel_E level)
{
    if (level == CJSON_LOG_LEVEL_WARN) {
        return CJSON_LOG_LEVEL_WARN_STRING;
    }

    if (level == CJSON_LOG_LEVEL_ERROR) {
        return CJSON_LOG_LEVEL_ERROR_STRING;
    }

    return CJSON_LOG_LEVEL_INFO_STRING;
}

/******************************************************************************
 * @brief      : 跳过空白字符
 * @param[in]  : input --输入字符串
 * @param[out] : 无
 * @return     : 返回跳过空白后的指针
 * @note       : 支持空格、制表符、换行等isspace识别的空白字符
 ******************************************************************************/
static const char* CJSON_SkipBlank(const char* input)
{
    const char* current = input;

    while ((current != NULL) && (*current != '\0') && (isspace((unsigned char)*current) != 0)) {
        current++;
    }

    return current;
}

/******************************************************************************
 * @brief      : 初始化动态字符串缓冲区
 * @param[in]  : 无
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 初始化成功后，buffer所有权属于jsonBuffer
 ******************************************************************************/
static CJSONBool_E CJSON_BufferInit(CJSONBuffer_S* jsonBuffer)
{
    if (jsonBuffer == NULL) {
        return CJSON_BOOL_FALSE;
    }

    jsonBuffer->buffer = (char*)malloc(CJSON_INIT_BUFFER_SIZE);
    if (jsonBuffer->buffer == NULL) {
        jsonBuffer->length = 0U;
        jsonBuffer->capacity = 0U;
        return CJSON_BOOL_FALSE;
    }

    jsonBuffer->length = 0U;
    jsonBuffer->capacity = CJSON_INIT_BUFFER_SIZE;
    jsonBuffer->buffer[0] = '\0';

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 计算扩容后的缓冲区大小
 * @param[in]  : oldCapacity --旧容量, requiredSize --需要的最小容量
 * @param[out] : newCapacity --新容量
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 内部按倍数增长，并避免size_t溢出
 ******************************************************************************/
static CJSONBool_E CJSON_BufferCalcNewCapacity(size_t oldCapacity, size_t requiredSize, size_t* newCapacity)
{
    size_t capacity = oldCapacity;

    if ((capacity == 0U) || (newCapacity == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    while (capacity < requiredSize) {
        if (capacity > (((size_t)-1) / CJSON_GROW_FACTOR)) {
            return CJSON_BOOL_FALSE;
        }

        capacity *= CJSON_GROW_FACTOR;
    }

    *newCapacity = capacity;

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 确保动态字符串缓冲区容量足够
 * @param[in]  : needSize --需要追加的字节数，不包含结尾'\0'
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 容量不足时自动扩容
 ******************************************************************************/
static CJSONBool_E CJSON_BufferEnsure(CJSONBuffer_S* jsonBuffer, size_t needSize)
{
    char* newBuffer = NULL;
    size_t newCapacity = 0U;
    size_t requiredSize = 0U;

    if ((jsonBuffer == NULL) || (jsonBuffer->buffer == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    if (needSize > (((size_t)-1) - jsonBuffer->length - 1U)) {
        return CJSON_BOOL_FALSE;
    }

    requiredSize = jsonBuffer->length + needSize + 1U;
    if (requiredSize <= jsonBuffer->capacity) {
        return CJSON_BOOL_TRUE;
    }

    if (CJSON_BufferCalcNewCapacity(jsonBuffer->capacity, requiredSize, &newCapacity) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    newBuffer = (char*)realloc(jsonBuffer->buffer, newCapacity);
    if (newBuffer == NULL) {
        return CJSON_BOOL_FALSE;
    }

    jsonBuffer->buffer = newBuffer;
    jsonBuffer->capacity = newCapacity;

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 追加普通字符串
 * @param[in]  : text --待追加字符串
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : text为NULL时按空字符串处理
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppend(CJSONBuffer_S* jsonBuffer, const char* text)
{
    size_t textLength = 0U;

    if (jsonBuffer == NULL) {
        return CJSON_BOOL_FALSE;
    }

    if (text == NULL) {
        text = CJSON_STRING_EMPTY;
    }

    textLength = strlen(text);
    if (CJSON_BufferEnsure(jsonBuffer, textLength) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    memcpy(jsonBuffer->buffer + jsonBuffer->length, text, textLength + 1U);
    jsonBuffer->length += textLength;

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 追加单个字符
 * @param[in]  : value --待追加字符
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 会自动维护字符串结尾'\0'
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendChar(CJSONBuffer_S* jsonBuffer, char value)
{
    if ((jsonBuffer == NULL) || (CJSON_BufferEnsure(jsonBuffer, 1U) != CJSON_BOOL_TRUE)) {
        return CJSON_BOOL_FALSE;
    }

    jsonBuffer->buffer[jsonBuffer->length] = value;
    jsonBuffer->length++;
    jsonBuffer->buffer[jsonBuffer->length] = '\0';

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 追加控制字符的unicode转义文本
 * @param[in]  : value --待转义控制字符
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 输出格式如\u0001
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendUnicodeEscape(CJSONBuffer_S* jsonBuffer, unsigned char value)
{
    char unicodeBuffer[8] = {0};

    (void)snprintf(unicodeBuffer, sizeof(unicodeBuffer), "\\u%04x", value);

    return CJSON_BufferAppend(jsonBuffer, unicodeBuffer);
}

/******************************************************************************
 * @brief      : 追加一个转义后的JSON字符
 * @param[in]  : value --待追加字符
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 只处理JSON字符串内部的单字符转义
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendEscapedChar(CJSONBuffer_S* jsonBuffer, unsigned char value)
{
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
        return CJSON_BufferAppendUnicodeEscape(jsonBuffer, value);
    }

    return CJSON_BufferAppendChar(jsonBuffer, (char)value);
}

/******************************************************************************
 * @brief      : 追加转义后的JSON字符串值
 * @param[in]  : text --普通字符串
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 会自动添加双引号，并处理常见JSON转义字符
 ******************************************************************************/
static CJSONBool_E CJSON_BufferAppendEscapedString(CJSONBuffer_S* jsonBuffer, const char* text)
{
    const unsigned char* current = NULL;

    if (text == NULL) {
        text = CJSON_STRING_EMPTY;
    }

    if (CJSON_BufferAppendChar(jsonBuffer, '\"') != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    for (current = (const unsigned char*)text; *current != '\0'; current++) {
        if (CJSON_BufferAppendEscapedChar(jsonBuffer, *current) != CJSON_BOOL_TRUE) {
            return CJSON_BOOL_FALSE;
        }
    }

    return CJSON_BufferAppendChar(jsonBuffer, '\"');
}

/******************************************************************************
 * @brief      : 读取有符号整数
 * @param[in]  : fieldAddress --字段地址, fieldSize --字段大小
 * @param[out] : value --读取到的值
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 根据字段大小读取int8_t/int16_t/int32_t/int64_t
 ******************************************************************************/
static CJSONBool_E CJSON_ReadSignedInteger(const void* fieldAddress, size_t fieldSize, int64_t* value)
{
    if ((fieldAddress == NULL) || (value == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    if (fieldSize == sizeof(int8_t)) {
        int8_t data = 0;
        memcpy(&data, fieldAddress, sizeof(data));
        *value = data;
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(int16_t)) {
        int16_t data = 0;
        memcpy(&data, fieldAddress, sizeof(data));
        *value = data;
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(int32_t)) {
        int32_t data = 0;
        memcpy(&data, fieldAddress, sizeof(data));
        *value = data;
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(int64_t)) {
        int64_t data = 0;
        memcpy(&data, fieldAddress, sizeof(data));
        *value = data;
        return CJSON_BOOL_TRUE;
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 读取无符号整数
 * @param[in]  : fieldAddress --字段地址, fieldSize --字段大小
 * @param[out] : value --读取到的值
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 根据字段大小读取uint8_t/uint16_t/uint32_t/uint64_t
 ******************************************************************************/
static CJSONBool_E CJSON_ReadUnsignedInteger(const void* fieldAddress, size_t fieldSize, uint64_t* value)
{
    if ((fieldAddress == NULL) || (value == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    if (fieldSize == sizeof(uint8_t)) {
        uint8_t data = 0U;
        memcpy(&data, fieldAddress, sizeof(data));
        *value = data;
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint16_t)) {
        uint16_t data = 0U;
        memcpy(&data, fieldAddress, sizeof(data));
        *value = data;
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint32_t)) {
        uint32_t data = 0U;
        memcpy(&data, fieldAddress, sizeof(data));
        *value = data;
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint64_t)) {
        uint64_t data = 0U;
        memcpy(&data, fieldAddress, sizeof(data));
        *value = data;
        return CJSON_BOOL_TRUE;
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 写入有符号整数
 * @param[in]  : fieldSize --字段大小, value --待写入值
 * @param[out] : fieldAddress --字段地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 根据字段大小写入int8_t/int16_t/int32_t/int64_t
 ******************************************************************************/
static CJSONBool_E CJSON_WriteSignedInteger(void* fieldAddress, size_t fieldSize, int64_t value)
{
    if (fieldAddress == NULL) {
        return CJSON_BOOL_FALSE;
    }

    if (fieldSize == sizeof(int8_t)) {
        int8_t data = (int8_t)value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(int16_t)) {
        int16_t data = (int16_t)value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(int32_t)) {
        int32_t data = (int32_t)value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(int64_t)) {
        int64_t data = value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 写入无符号整数
 * @param[in]  : fieldSize --字段大小, value --待写入值
 * @param[out] : fieldAddress --字段地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 根据字段大小写入uint8_t/uint16_t/uint32_t/uint64_t
 ******************************************************************************/
static CJSONBool_E CJSON_WriteUnsignedInteger(void* fieldAddress, size_t fieldSize, uint64_t value)
{
    if (fieldAddress == NULL) {
        return CJSON_BOOL_FALSE;
    }

    if (fieldSize == sizeof(uint8_t)) {
        uint8_t data = (uint8_t)value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint16_t)) {
        uint16_t data = (uint16_t)value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint32_t)) {
        uint32_t data = (uint32_t)value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(uint64_t)) {
        uint64_t data = value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 写入浮点数
 * @param[in]  : fieldSize --字段大小, value --待写入值
 * @param[out] : fieldAddress --字段地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 根据字段大小写入float/double/long double
 ******************************************************************************/
static CJSONBool_E CJSON_WriteFloatingPoint(void* fieldAddress, size_t fieldSize, long double value)
{
    if (fieldAddress == NULL) {
        return CJSON_BOOL_FALSE;
    }

    if (fieldSize == sizeof(float)) {
        float data = (float)value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(double)) {
        double data = (double)value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    if (fieldSize == sizeof(long double)) {
        long double data = value;
        memcpy(fieldAddress, &data, sizeof(data));
        return CJSON_BOOL_TRUE;
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 完成动态字符串构造并返回字符串
 * @param[in]  : 无
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 成功后调用者接管buffer所有权
 ******************************************************************************/
static char* CJSON_BufferDetach(CJSONBuffer_S* jsonBuffer)
{
    char* jsonString = NULL;

    if (jsonBuffer == NULL) {
        return NULL;
    }

    jsonString = jsonBuffer->buffer;
    jsonBuffer->buffer = NULL;
    jsonBuffer->length = 0U;
    jsonBuffer->capacity = 0U;

    return jsonString;
}

/******************************************************************************
 * @brief      : 追加结构体字段名称和值分隔符
 * @param[in]  : field --字段描述, isFirst --是否为第一个字段
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 输出格式为"key":
 ******************************************************************************/
static CJSONBool_E CJSON_AppendFieldKey(CJSONBuffer_S* jsonBuffer, const CJSONField_S* field, CJSONBool_E isFirst)
{
    if ((jsonBuffer == NULL) || (field == NULL) || (field->jsonKey == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    if ((isFirst != CJSON_BOOL_TRUE) && (CJSON_BufferAppendChar(jsonBuffer, ',') != CJSON_BOOL_TRUE)) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedString(jsonBuffer, field->jsonKey) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    return CJSON_BufferAppendChar(jsonBuffer, ':');
}

/******************************************************************************
 * @brief      : 追加有符号整数字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 按字段大小读取并转为十进制字符串
 ******************************************************************************/
static CJSONBool_E CJSON_AppendSignedField(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};
    int64_t signedValue = 0;

    if (CJSON_ReadSignedInteger(fieldAddress, field->fieldSize, &signedValue) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    (void)snprintf(numberBuffer, sizeof(numberBuffer), "%" PRId64, signedValue);

    return CJSON_BufferAppend(jsonBuffer, numberBuffer);
}

/******************************************************************************
 * @brief      : 追加无符号整数字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 按字段大小读取并转为十进制字符串
 ******************************************************************************/
static CJSONBool_E CJSON_AppendUnsignedField(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};
    uint64_t unsignedValue = 0U;

    if (CJSON_ReadUnsignedInteger(fieldAddress, field->fieldSize, &unsignedValue) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    (void)snprintf(numberBuffer, sizeof(numberBuffer), "%" PRIu64, unsignedValue);

    return CJSON_BufferAppend(jsonBuffer, numberBuffer);
}

/******************************************************************************
 * @brief      : 追加浮点字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : float/double/long double分别使用不同精度格式化
 ******************************************************************************/
static CJSONBool_E CJSON_AppendDoubleField(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    char numberBuffer[CJSON_NUMBER_BUFFER_SIZE] = {0};

    if (field->fieldSize == sizeof(float)) {
        float floatValue = 0.0F;
        memcpy(&floatValue, fieldAddress, sizeof(floatValue));
        (void)snprintf(numberBuffer, sizeof(numberBuffer), "%.9g", (double)floatValue);
        return CJSON_BufferAppend(jsonBuffer, numberBuffer);
    }

    if (field->fieldSize == sizeof(double)) {
        double doubleValue = 0.0;
        memcpy(&doubleValue, fieldAddress, sizeof(doubleValue));
        (void)snprintf(numberBuffer, sizeof(numberBuffer), "%.17g", doubleValue);
        return CJSON_BufferAppend(jsonBuffer, numberBuffer);
    }

    if (field->fieldSize == sizeof(long double)) {
        long double longDoubleValue = 0.0L;
        memcpy(&longDoubleValue, fieldAddress, sizeof(longDoubleValue));
        (void)snprintf(numberBuffer, sizeof(numberBuffer), "%.21Lg", longDoubleValue);
        return CJSON_BufferAppend(jsonBuffer, numberBuffer);
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 追加布尔字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 支持C11 bool/_Bool和CJSONBool_E
 ******************************************************************************/
static CJSONBool_E CJSON_AppendBoolField(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    CJSONBool_E boolValue = CJSON_BOOL_FALSE;

    if (field->fieldSize == sizeof(_Bool)) {
        boolValue = (*(const _Bool*)fieldAddress) ? CJSON_BOOL_TRUE : CJSON_BOOL_FALSE;
    } else {
        boolValue = (*(const CJSONBool_E*)fieldAddress == CJSON_BOOL_TRUE) ? CJSON_BOOL_TRUE : CJSON_BOOL_FALSE;
    }

    return CJSON_BufferAppend(jsonBuffer, (boolValue == CJSON_BOOL_TRUE) ? "true" : "false");
}

/******************************************************************************
 * @brief      : 追加字段值
 * @param[in]  : fieldAddress --字段地址, field --字段描述
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 根据字段类型分发到具体序列化函数
 ******************************************************************************/
static CJSONBool_E CJSON_AppendFieldValue(CJSONBuffer_S* jsonBuffer, const void* fieldAddress, const CJSONField_S* field)
{
    switch (field->fieldType) {
        case CJSON_FIELD_INT:
            return CJSON_AppendSignedField(jsonBuffer, fieldAddress, field);
        case CJSON_FIELD_UINT:
            return CJSON_AppendUnsignedField(jsonBuffer, fieldAddress, field);
        case CJSON_FIELD_DOUBLE:
            return CJSON_AppendDoubleField(jsonBuffer, fieldAddress, field);
        case CJSON_FIELD_BOOL:
            return CJSON_AppendBoolField(jsonBuffer, fieldAddress, field);
        case CJSON_FIELD_STRING:
            return CJSON_BufferAppendEscapedString(jsonBuffer, (const char*)fieldAddress);
        default:
            return CJSON_BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 根据字段描述追加一个结构体字段
 * @param[in]  : object --结构体地址, field --字段描述, isFirst --是否为第一个字段
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 输出格式为"key":value
 ******************************************************************************/
static CJSONBool_E CJSON_AppendStructField(CJSONBuffer_S* jsonBuffer, const void* object, const CJSONField_S* field, CJSONBool_E isFirst)
{
    const void* fieldAddress = NULL;

    if ((jsonBuffer == NULL) || (object == NULL) || (field == NULL) || (field->jsonKey == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    fieldAddress = (const unsigned char*)object + field->fieldOffset;

    if (CJSON_AppendFieldKey(jsonBuffer, field, isFirst) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    return CJSON_AppendFieldValue(jsonBuffer, fieldAddress, field);
}

/******************************************************************************
 * @brief      : 根据结构体字段描述序列化为JSON对象字符串
 * @param[in]  : object --结构体地址, fields --字段描述表
 * @param[out] : 无
 * @return     : 返回JSON对象字符串，失败返回NULL
 * @note       : 只生成{"key":value,...}对象片段，返回值需要CJSON_Free释放
 ******************************************************************************/
char* CJSON_SerializeStructObject(const void* object, const CJSONField_S* fields)
{
    CJSONBuffer_S jsonBuffer = {0};
    size_t fieldIndex = 0U;
    CJSONBool_E isFirst = CJSON_BOOL_TRUE;

    if ((object == NULL) || (fields == NULL) || (CJSON_BufferInit(&jsonBuffer) != CJSON_BOOL_TRUE)) {
        return NULL;
    }

    if (CJSON_BufferAppendChar(&jsonBuffer, '{') != CJSON_BOOL_TRUE) {
        CJSON_Free(jsonBuffer.buffer);
        return NULL;
    }

    for (fieldIndex = 0U; fields[fieldIndex].jsonKey != NULL; fieldIndex++) {
        if (CJSON_AppendStructField(&jsonBuffer, object, &fields[fieldIndex], isFirst) != CJSON_BOOL_TRUE) {
            CJSON_Free(jsonBuffer.buffer);
            return NULL;
        }

        isFirst = CJSON_BOOL_FALSE;
    }

    if (CJSON_BufferAppendChar(&jsonBuffer, '}') != CJSON_BOOL_TRUE) {
        CJSON_Free(jsonBuffer.buffer);
        return NULL;
    }

    return CJSON_BufferDetach(&jsonBuffer);
}

/******************************************************************************
 * @brief      : 从JSON对象中查找指定键对应的值起始位置
 * @param[in]  : jsonObject --JSON对象字符串, key --键名
 * @param[out] : 无
 * @return     : 返回值起始位置，失败返回NULL
 * @note       : 只处理扁平对象，不支持嵌套键查找
 ******************************************************************************/
static const char* CJSON_FindValueByKey(const char* jsonObject, const char* key)
{
    const char* current = NULL;
    size_t keyLength = 0U;

    if ((jsonObject == NULL) || (key == NULL)) {
        return NULL;
    }

    current = jsonObject;
    keyLength = strlen(key);

    while (*current != '\0') {
        current = CJSON_SkipBlank(current);
        if (*current == '\"') {
            current++;
            if ((strncmp(current, key, keyLength) == 0) && (current[keyLength] == '\"')) {
                current += keyLength + 1U;
                current = CJSON_SkipBlank(current);
                if (*current == ':') {
                    current++;
                    return CJSON_SkipBlank(current);
                }
            }
        }

        current++;
    }

    return NULL;
}

/******************************************************************************
 * @brief      : 追加一个解析后的JSON转义字符
 * @param[in]  : escapeChar --转义字符
 * @param[out] : output --输出缓冲区, outputIndex --输出位置
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 不处理\uXXXX，只保留其转义字母
 ******************************************************************************/
static CJSONBool_E CJSON_WriteParsedEscapeChar(char escapeChar, char* output, size_t* outputIndex)
{
    if ((output == NULL) || (outputIndex == NULL)) {
        return CJSON_BOOL_FALSE;
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

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析JSON字符串值到缓冲区
 * @param[in]  : valueStart --JSON字符串值起始位置, outputSize --输出缓冲区大小
 * @param[out] : output --输出字符串
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 支持常见转义字符，输出字符串会自动补'\0'
 ******************************************************************************/
static CJSONBool_E CJSON_ParseJsonStringValue(const char* valueStart, char* output, size_t outputSize)
{
    const char* current = valueStart;
    size_t outputIndex = 0U;

    if ((valueStart == NULL) || (output == NULL) || (outputSize == 0U) || (*current != '\"')) {
        return CJSON_BOOL_FALSE;
    }

    current++;
    while ((*current != '\0') && (*current != '\"') && (outputIndex + 1U < outputSize)) {
        if (*current == '\\') {
            current++;
            if (CJSON_WriteParsedEscapeChar(*current, output, &outputIndex) != CJSON_BOOL_TRUE) {
                return CJSON_BOOL_FALSE;
            }
            current++;
        } else {
            output[outputIndex] = *current;
            outputIndex++;
            current++;
        }
    }

    if (*current != '\"') {
        return CJSON_BOOL_FALSE;
    }

    output[outputIndex] = '\0';

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析有符号整数字段
 * @param[in]  : valueStart --JSON值起始位置, field --字段描述
 * @param[out] : fieldAddress --字段地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 使用strtoll解析十进制整数
 ******************************************************************************/
static CJSONBool_E CJSON_ParseSignedField(const char* valueStart, void* fieldAddress, const CJSONField_S* field)
{
    int64_t signedValue = 0;

    signedValue = (int64_t)strtoll(valueStart, NULL, 10);

    return CJSON_WriteSignedInteger(fieldAddress, field->fieldSize, signedValue);
}

/******************************************************************************
 * @brief      : 解析无符号整数字段
 * @param[in]  : valueStart --JSON值起始位置, field --字段描述
 * @param[out] : fieldAddress --字段地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 使用strtoull解析十进制整数
 ******************************************************************************/
static CJSONBool_E CJSON_ParseUnsignedField(const char* valueStart, void* fieldAddress, const CJSONField_S* field)
{
    uint64_t unsignedValue = 0U;

    unsignedValue = (uint64_t)strtoull(valueStart, NULL, 10);

    return CJSON_WriteUnsignedInteger(fieldAddress, field->fieldSize, unsignedValue);
}

/******************************************************************************
 * @brief      : 解析浮点字段
 * @param[in]  : valueStart --JSON值起始位置, field --字段描述
 * @param[out] : fieldAddress --字段地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 使用strtold解析浮点数
 ******************************************************************************/
static CJSONBool_E CJSON_ParseDoubleField(const char* valueStart, void* fieldAddress, const CJSONField_S* field)
{
    long double floatingValue = 0.0L;

    floatingValue = strtold(valueStart, NULL);

    return CJSON_WriteFloatingPoint(fieldAddress, field->fieldSize, floatingValue);
}

/******************************************************************************
 * @brief      : 解析布尔字段
 * @param[in]  : valueStart --JSON值起始位置, field --字段描述
 * @param[out] : fieldAddress --字段地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 只把true解析为真，其他值解析为假
 ******************************************************************************/
static CJSONBool_E CJSON_ParseBoolField(const char* valueStart, void* fieldAddress, const CJSONField_S* field)
{
    CJSONBool_E boolValue = (strncmp(valueStart, "true", 4) == 0) ? CJSON_BOOL_TRUE : CJSON_BOOL_FALSE;

    if (field->fieldSize == sizeof(_Bool)) {
        *(_Bool*)fieldAddress = (boolValue == CJSON_BOOL_TRUE);
        return CJSON_BOOL_TRUE;
    }

    *(CJSONBool_E*)fieldAddress = boolValue;

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 根据字段类型解析一个字段值
 * @param[in]  : valueStart --JSON值起始位置, field --字段描述
 * @param[out] : fieldAddress --字段地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 根据字段类型分发到具体解析函数
 ******************************************************************************/
static CJSONBool_E CJSON_ParseFieldValue(const char* valueStart, void* fieldAddress, const CJSONField_S* field)
{
    switch (field->fieldType) {
        case CJSON_FIELD_INT:
            return CJSON_ParseSignedField(valueStart, fieldAddress, field);
        case CJSON_FIELD_UINT:
            return CJSON_ParseUnsignedField(valueStart, fieldAddress, field);
        case CJSON_FIELD_DOUBLE:
            return CJSON_ParseDoubleField(valueStart, fieldAddress, field);
        case CJSON_FIELD_BOOL:
            return CJSON_ParseBoolField(valueStart, fieldAddress, field);
        case CJSON_FIELD_STRING:
            return CJSON_ParseJsonStringValue(valueStart, (char*)fieldAddress, field->fieldSize);
        default:
            return CJSON_BOOL_FALSE;
    }
}

/******************************************************************************
 * @brief      : 根据字段描述解析一个结构体字段
 * @param[in]  : jsonObject --JSON对象字符串, field --字段描述
 * @param[out] : object --结构体地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 字段不存在时返回失败
 ******************************************************************************/
static CJSONBool_E CJSON_ParseStructField(const char* jsonObject, void* object, const CJSONField_S* field)
{
    const char* valueStart = NULL;
    void* fieldAddress = NULL;

    if ((jsonObject == NULL) || (object == NULL) || (field == NULL) || (field->jsonKey == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    valueStart = CJSON_FindValueByKey(jsonObject, field->jsonKey);
    if (valueStart == NULL) {
        return CJSON_BOOL_FALSE;
    }

    fieldAddress = (unsigned char*)object + field->fieldOffset;

    return CJSON_ParseFieldValue(valueStart, fieldAddress, field);
}

/******************************************************************************
 * @brief      : 根据字段描述把JSON对象反序列化到结构体
 * @param[in]  : jsonObject --JSON对象字符串, fields --字段描述表
 * @param[out] : object --结构体地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 只支持扁平对象，如{"key1":1,"key2":"abc"}
 ******************************************************************************/
CJSONBool_E CJSON_DeserializeStructObject(const char* jsonObject, void* object, const CJSONField_S* fields)
{
    size_t fieldIndex = 0U;

    if ((jsonObject == NULL) || (object == NULL) || (fields == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    for (fieldIndex = 0U; fields[fieldIndex].jsonKey != NULL; fieldIndex++) {
        if (CJSON_ParseStructField(jsonObject, object, &fields[fieldIndex]) != CJSON_BOOL_TRUE) {
            return CJSON_BOOL_FALSE;
        }
    }

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 追加响应JSON头部
 * @param[in]  : result --结果枚举
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 输出{"result":"pass/fail","data":
 ******************************************************************************/
static CJSONBool_E CJSON_AppendResponseHead(CJSONBuffer_S* jsonBuffer, CJSONResult_E result)
{
    if (CJSON_BufferAppendChar(jsonBuffer, '{') != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedString(jsonBuffer, CJSON_KEY_RESULT) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_BufferAppendChar(jsonBuffer, ':') != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedString(jsonBuffer, CJSON_GetResultString(result)) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_BufferAppendChar(jsonBuffer, ',') != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_BufferAppendEscapedString(jsonBuffer, CJSON_KEY_DATA) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    return CJSON_BufferAppendChar(jsonBuffer, ':');
}

/******************************************************************************
 * @brief      : 追加响应data字段
 * @param[in]  : dataType --data类型, dataText --data字符串或对象片段
 * @param[out] : jsonBuffer --动态字符串缓冲区
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 对象类型直接追加，字符串/空类型会自动加双引号
 ******************************************************************************/
static CJSONBool_E CJSON_AppendResponseData(CJSONBuffer_S* jsonBuffer, CJSONDataType_E dataType, const char* dataText)
{
    if (dataText == NULL) {
        dataText = CJSON_STRING_EMPTY;
    }

    if (dataType == CJSON_DATA_OBJECT) {
        return CJSON_BufferAppend(jsonBuffer, dataText);
    }

    return CJSON_BufferAppendEscapedString(jsonBuffer, dataText);
}

/******************************************************************************
 * @brief      : 拼接响应JSON
 * @param[in]  : result --结果枚举, dataType --data类型, dataText --data字符串或对象片段
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : dataType为对象时，dataText必须是不带引号的JSON对象
 ******************************************************************************/
static char* CJSON_BuildResponseInternal(CJSONResult_E result, CJSONDataType_E dataType, const char* dataText)
{
    CJSONBuffer_S jsonBuffer = {0};

    if (CJSON_BufferInit(&jsonBuffer) != CJSON_BOOL_TRUE) {
        return NULL;
    }

    if (CJSON_AppendResponseHead(&jsonBuffer, result) != CJSON_BOOL_TRUE) {
        CJSON_Free(jsonBuffer.buffer);
        return NULL;
    }

    if (CJSON_AppendResponseData(&jsonBuffer, dataType, dataText) != CJSON_BOOL_TRUE) {
        CJSON_Free(jsonBuffer.buffer);
        return NULL;
    }

    if (CJSON_BufferAppendChar(&jsonBuffer, '}') != CJSON_BOOL_TRUE) {
        CJSON_Free(jsonBuffer.buffer);
        return NULL;
    }

    return CJSON_BufferDetach(&jsonBuffer);
}

/******************************************************************************
 * @brief      : 构建只有结果的响应JSON
 * @param[in]  : result --结果枚举
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":""}
 ******************************************************************************/
char* CJSON_BuildEmptyResponse(CJSONResult_E result)
{
    return CJSON_BuildResponseInternal(result, CJSON_DATA_EMPTY, CJSON_STRING_EMPTY);
}

/******************************************************************************
 * @brief      : 构建data为字符串的响应JSON
 * @param[in]  : result --结果枚举, dataString --data字符串
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":"xxxx"}
 ******************************************************************************/
char* CJSON_BuildStringResponse(CJSONResult_E result, const char* dataString)
{
    return CJSON_BuildResponseInternal(result, CJSON_DATA_STRING, dataString);
}

/******************************************************************************
 * @brief      : 构建data为对象的响应JSON
 * @param[in]  : result --结果枚举, object --结构体地址, fields --字段描述表
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":{"key":value}}
 ******************************************************************************/
char* CJSON_BuildObjectResponse(CJSONResult_E result, const void* object, const CJSONField_S* fields)
{
    char* dataObjectString = NULL;
    char* responseString = NULL;

    dataObjectString = CJSON_SerializeStructObject(object, fields);
    if (dataObjectString == NULL) {
        return NULL;
    }

    responseString = CJSON_BuildResponseInternal(result, CJSON_DATA_OBJECT, dataObjectString);
    CJSON_Free(dataObjectString);

    return responseString;
}

/******************************************************************************
 * @brief      : 获取响应结果字段
 * @param[in]  : jsonString --响应JSON字符串
 * @param[out] : result --响应结果枚举
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 只接受pass/fail字符串
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResultField(const char* jsonString, CJSONResult_E* result)
{
    const char* resultStart = NULL;
    char resultText[16] = {0};

    if ((jsonString == NULL) || (result == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    resultStart = CJSON_FindValueByKey(jsonString, CJSON_KEY_RESULT);
    if (resultStart == NULL) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_ParseJsonStringValue(resultStart, resultText, sizeof(resultText)) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    if (strcmp(resultText, CJSON_RESULT_PASS_STRING) == 0) {
        *result = CJSON_RESULT_PASS;
        return CJSON_BOOL_TRUE;
    }

    if (strcmp(resultText, CJSON_RESULT_FAIL_STRING) == 0) {
        *result = CJSON_RESULT_FAIL;
        return CJSON_BOOL_TRUE;
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 解析data字符串到响应结构体
 * @param[in]  : dataStart --data字段值起始位置
 * @param[out] : response --响应结构体
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : dataString必须由调用者提供，默认最多写入256字节
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResponseStringData(const char* dataStart, CJSONResponse_S* response)
{
    if ((dataStart == NULL) || (response == NULL) || (response->dataString == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    response->dataType = CJSON_DATA_STRING;

    if ((dataStart[0] == '\"') && (dataStart[1] == '\"')) {
        response->dataType = CJSON_DATA_EMPTY;
    }

    return CJSON_ParseJsonStringValue(dataStart, response->dataString, CJSON_RESPONSE_STRING_BUFFER_SIZE);
}

/******************************************************************************
 * @brief      : 更新对象文本拷贝状态
 * @param[in]  : currentChar --当前字符
 * @param[out] : braceDepth --花括号层级, inString --是否在字符串中, escaped --是否处于转义状态
 * @return     : CJSON_BOOL_TRUE --对象已经结束，CJSON_BOOL_FALSE --对象尚未结束
 * @note       : 能避免字符串内部的花括号影响对象层级统计
 ******************************************************************************/
static CJSONBool_E CJSON_UpdateObjectCopyState(char currentChar, int* braceDepth, CJSONBool_E* inString, CJSONBool_E* escaped)
{
    if ((braceDepth == NULL) || (inString == NULL) || (escaped == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    if (*escaped == CJSON_BOOL_TRUE) {
        *escaped = CJSON_BOOL_FALSE;
        return CJSON_BOOL_FALSE;
    }

    if (currentChar == '\\') {
        *escaped = (*inString == CJSON_BOOL_TRUE) ? CJSON_BOOL_TRUE : CJSON_BOOL_FALSE;
        return CJSON_BOOL_FALSE;
    }

    if (currentChar == '\"') {
        *inString = (*inString == CJSON_BOOL_TRUE) ? CJSON_BOOL_FALSE : CJSON_BOOL_TRUE;
        return CJSON_BOOL_FALSE;
    }

    if (*inString == CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    if (currentChar == '{') {
        (*braceDepth)++;
    } else if (currentChar == '}') {
        (*braceDepth)--;
    }

    return (*braceDepth == 0) ? CJSON_BOOL_TRUE : CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 从响应JSON提取data对象片段
 * @param[in]  : dataStart --data字段值起始位置, outputSize --输出缓冲区大小
 * @param[out] : output --data对象片段
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 通过花括号层级提取完整对象文本
 ******************************************************************************/
static CJSONBool_E CJSON_CopyDataObjectText(const char* dataStart, char* output, size_t outputSize)
{
    const char* current = dataStart;
    size_t outputIndex = 0U;
    int braceDepth = 0;
    CJSONBool_E inString = CJSON_BOOL_FALSE;
    CJSONBool_E escaped = CJSON_BOOL_FALSE;

    if ((dataStart == NULL) || (output == NULL) || (outputSize == 0U) || (*current != '{')) {
        return CJSON_BOOL_FALSE;
    }

    while (*current != '\0') {
        if (outputIndex + 1U >= outputSize) {
            return CJSON_BOOL_FALSE;
        }

        output[outputIndex] = *current;
        outputIndex++;

        if (CJSON_UpdateObjectCopyState(*current, &braceDepth, &inString, &escaped) == CJSON_BOOL_TRUE) {
            output[outputIndex] = '\0';
            return CJSON_BOOL_TRUE;
        }

        current++;
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 解析响应data对象
 * @param[in]  : dataStart --data字段值起始位置, objectSize --data对象结构体大小, fields --字段描述表
 * @param[out] : response --响应结构体, object --data对象结构体地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 先提取data对象文本，再按字段描述反序列化到结构体
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResponseObjectData(const char* dataStart, CJSONResponse_S* response, void* object, size_t objectSize, const CJSONField_S* fields)
{
    char* dataObjectText = NULL;
    size_t objectTextSize = 0U;
    CJSONBool_E parseResult = CJSON_BOOL_FALSE;

    if ((dataStart == NULL) || (response == NULL) || (object == NULL) || (objectSize == 0U) || (fields == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    objectTextSize = objectSize * 16U + CJSON_INIT_BUFFER_SIZE;
    dataObjectText = (char*)malloc(objectTextSize);
    if (dataObjectText == NULL) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_CopyDataObjectText(dataStart, dataObjectText, objectTextSize) == CJSON_BOOL_TRUE) {
        parseResult = CJSON_DeserializeStructObject(dataObjectText, object, fields);
    }

    CJSON_Free(dataObjectText);

    if (parseResult != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    response->dataType = CJSON_DATA_OBJECT;
    response->dataObject = object;

    return CJSON_BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 解析响应data字段
 * @param[in]  : dataStart --data字段值起始位置, objectSize --data对象结构体大小, fields --字段描述表
 * @param[out] : response --响应结构体, object --data对象结构体地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 根据data字段首字符判断字符串或对象
 ******************************************************************************/
static CJSONBool_E CJSON_ParseResponseData(const char* dataStart, CJSONResponse_S* response, void* object, size_t objectSize, const CJSONField_S* fields)
{
    if ((dataStart == NULL) || (response == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    if (*dataStart == '\"') {
        return CJSON_ParseResponseStringData(dataStart, response);
    }

    if (*dataStart == '{') {
        return CJSON_ParseResponseObjectData(dataStart, response, object, objectSize, fields);
    }

    return CJSON_BOOL_FALSE;
}

/******************************************************************************
 * @brief      : 解析响应JSON
 * @param[in]  : jsonString --响应JSON字符串, objectSize --data对象结构体大小, fields --字段描述表
 * @param[out] : response --响应信息, object --data对象结构体地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : data是对象时使用fields解析，data是字符串或空字符串时写入response->dataString
 ******************************************************************************/
CJSONBool_E CJSON_ParseResponse(const char* jsonString, CJSONResponse_S* response, void* object, size_t objectSize, const CJSONField_S* fields)
{
    const char* dataStart = NULL;

    if ((jsonString == NULL) || (response == NULL)) {
        return CJSON_BOOL_FALSE;
    }

    if (CJSON_ParseResultField(jsonString, &response->result) != CJSON_BOOL_TRUE) {
        return CJSON_BOOL_FALSE;
    }

    dataStart = CJSON_FindValueByKey(jsonString, CJSON_KEY_DATA);
    if (dataStart == NULL) {
        return CJSON_BOOL_FALSE;
    }

    return CJSON_ParseResponseData(dataStart, response, object, objectSize, fields);
}

/******************************************************************************
 * @brief      : 填充日志data对象
 * @param[in]  : level --日志级别, module --模块名, message --日志信息
 * @param[out] : logData --日志data对象
 * @return     : 无
 * @note       : 输入字符串为NULL时按空字符串处理
 ******************************************************************************/
static void CJSON_FillLogData(CJSONLogData_S* logData, CJSONLogLevel_E level, const char* module, const char* message)
{
    if (logData == NULL) {
        return;
    }

    (void)snprintf(logData->level, sizeof(logData->level), "%s", CJSON_GetLogLevelString(level));
    (void)snprintf(logData->module, sizeof(logData->module), "%s", (module == NULL) ? CJSON_STRING_EMPTY : module);
    (void)snprintf(logData->message, sizeof(logData->message), "%s", (message == NULL) ? CJSON_STRING_EMPTY : message);
}

/******************************************************************************
 * @brief      : 构建日志JSON
 * @param[in]  : level --日志级别, module --模块名, message --日志信息
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass","data":{"level":"INFO","module":"xxx","message":"xxx"}}
 ******************************************************************************/
char* CJSON_BuildLogResponse(CJSONLogLevel_E level, const char* module, const char* message)
{
    CJSONLogData_S logData = {0};

    CJSON_FillLogData(&logData, level, module, message);

    return CJSON_BuildObjectResponse(CJSON_RESULT_PASS, &logData, g_cjsonLogFields);
}

/******************************************************************************
 * @brief      : 打印日志JSON
 * @param[in]  : level --日志级别, module --模块名, message --日志信息
 * @param[out] : 无
 * @return     : 无
 * @note       : 默认输出到stdout，嵌入式工程可替换printf输出接口
 ******************************************************************************/
void CJSON_Log(CJSONLogLevel_E level, const char* module, const char* message)
{
    char* jsonString = NULL;

    jsonString = CJSON_BuildLogResponse(level, module, message);
    if (jsonString == NULL) {
        return;
    }

    (void)printf("%s\n", jsonString);
    CJSON_Free(jsonString);
}

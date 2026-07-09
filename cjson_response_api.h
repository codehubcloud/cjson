/******************************************************************************
 * File Name : cjson_response_api.h
 * Function  : 响应JSON构造、解析和结构体字段注册接口声明。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#ifndef CJSON_RESPONSE_API_H
#define CJSON_RESPONSE_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* JSON 布尔类型枚举, 用于表示 JSON 中的 true/false 值 */
typedef enum {
    BOOL_FALSE = 0,
    BOOL_TRUE,
} CJSONBool_E;

/* JSON 响应结果类型枚举, 对应 JSON 中的 "result" 字段值 */
typedef enum {
    RESULT_FAIL = 0,
    RESULT_PASS,
} CJSONResult_E;

/* JSON 数据类型枚举, 用于标识键值对中数据的类型 */
typedef enum {
    DATA_INT = 0,
    DATA_UINT,
    DATA_BOOL,
    DATA_DOUBLE,
    DATA_STRING,
    DATA_CHAR,
} CJSONDataType_E;

/* JSON 键值对结构体, 包含 key、类型和对应类型的 union 值 */
typedef struct {
    const char *key;      /* JSON 对象中的键名 */
    CJSONDataType_E type; /* 值的数据类型 */

    union {
        int64_t intValue;        /* 有符号整数值 */
        uint64_t uintValue;      /* 无符号整数值 */
        const char *stringValue; /* 字符串值 */
        CJSONBool_E boolValue;   /* 布尔值 */
        char charValue;          /* 单字符值 */
    };
} CJSONKv_S;

/* 多item的json */
typedef struct {
    CJSONKv_S *kvs;
    size_t kvCount;
} CJSONItem_S;

/******************************************************************************
 * @brief      : 构建 JSON 响应并写入缓冲区
 * @param[in]  : dataBuf --输出缓冲区, dataLen --输出缓冲区大小, result --响应结果 (RESULT_PASS / RESULT_FAIL), kvs --键值对数组指针(无数据时传 NULL),
 *               kvCount--键值对个数(无数据时传 0)
 * @param[out] : 无
 * @return     : 无
 * @note       : 构建失败时会把 dataBuf[0] 置为 '\0'
 ******************************************************************************/
void CJSON_ResultBuildKv(char *dataBuf, size_t dataLen, CJSONResult_E result, const CJSONKv_S *kvs, size_t kvCount);

/******************************************************************************
 * @brief      : 构建多行 JSON 响应（detail数组结构扩展版）
 * @param[in]  : dataBuf --输出缓冲区, dataLen --缓冲区大小, result --调用方传入的命令执行结果, items --item数组, itemCount --item数量
 * @param[out] : dataBuf --完整 JSON 输出结果
 * @return     : 无
 * @note       : detail 内每个 item 独立包含 result + data，summary 自动统计 fail
 ******************************************************************************/
void FT_ResultPrintMultiLineImpl(char *dataBuf, size_t dataLen, CJSONResult_E result, const CJSONItem_S *items, size_t itemCount);

/******************************************************************************
 * @brief      : 创建有符号整数类型的键值对
 * @param[in]  : key --JSON 中的键名, value --有符号整数值
 * @param[out] : 无
 * @return     : CJSONKv_S --构造的键值对对象
 * @note       : 支持 int / long / long long / int32_t / int64_t 等类型
 ******************************************************************************/
static inline CJSONKv_S CJSON_MakeIntKv(const char *key, int64_t value)
{
    CJSONKv_S kv = {key, DATA_INT, {.intValue = value}};
    return kv;
}

/******************************************************************************
 * @brief      : 创建无符号整数类型的键值对
 * @param[in]  : key --JSON 中的键名, value --无符号整数值
 * @param[out] : 无
 * @return     : CJSONKv_S --构造的键值对对象
 * @note       : 支持 unsigned int / unsigned long / uint32_t / uint64_t 等类型
 ******************************************************************************/
static inline CJSONKv_S CJSON_MakeUIntKv(const char *key, uint64_t value)
{
    CJSONKv_S kv = {key, DATA_UINT, {.uintValue = value}};
    return kv;
}

/******************************************************************************
 * @brief      : 创建布尔值类型的键值对
 * @param[in]  : key --JSON 中的键名, value --布尔值 (true / false)
 * @param[out] : 无
 * @return     : CJSONKv_S --构造的键值对对象
 * @note       : 输出为 JSON 中的 true 或 false 字符串
 ******************************************************************************/
static inline CJSONKv_S CJSON_MakeBoolKv(const char *key, bool value)
{
    CJSONKv_S kv = {key, DATA_BOOL, {.boolValue = value ? BOOL_TRUE : BOOL_FALSE}};
    return kv;
}

/******************************************************************************
 * @brief      : 创建字符串类型的键值对
 * @param[in]  : key --JSON 中的键名, value --字符串指针(可以为 NULL, 会输出为空字符串)
 * @param[out] : 无
 * @return     : CJSONKv_S --构造的键值对对象
 * @note       : 支持 char * 和 const char *, 不对字符串进行转义处理
 ******************************************************************************/
static inline CJSONKv_S CJSON_MakeStringKv(const char *key, const char *value)
{
    CJSONKv_S kv = {key, DATA_STRING, {.stringValue = value}};
    return kv;
}

/******************************************************************************
 * @brief      : 创建单字符类型的键值对
 * @param[in]  : key --JSON 中的键名, value --单字符值
 * @param[out] : 无
 * @return     : CJSONKv_S --构造的键值对对象
 * @note       : 输出为 JSON 字符串格式, 例如 'A' 输出为 "A"
 ******************************************************************************/
static inline CJSONKv_S CJSON_MakeCharKv(const char *key, char value)
{
    CJSONKv_S kv = {key, DATA_CHAR, {.charValue = value}};
    return kv;
}

/******************************************************************************
 * @brief      : 自动推导类型并创建键值对宏
 * @param[in]  : key --JSON 中的键名 (字符串字面量), x --值表达式(类型会被自动推导)
 * @return     : CJSONKv_S --构造的键值对对象
 * @note       : 基于 _Generic 自动选择合适的创建函数
 *              支持类型：int, unsigned int, long, unsigned long,
 *              long long, unsigned long long, float, double,
 *              long double, char *, const char *, bool, char
 ******************************************************************************/
#define CJSON_MAKE_KV(key, x)                 \
    _Generic((x),                             \
        int: CJSON_MakeIntKv,                 \
        unsigned int: CJSON_MakeUIntKv,       \
        long: CJSON_MakeIntKv,                \
        unsigned long: CJSON_MakeUIntKv,      \
        long long: CJSON_MakeIntKv,           \
        unsigned long long: CJSON_MakeUIntKv, \
        char *: CJSON_MakeStringKv,           \
        const char *: CJSON_MakeStringKv,     \
        bool: CJSON_MakeBoolKv,               \
        char: CJSON_MakeCharKv)(key, x)

/******************************************************************************
 * @brief      : 构建 JSON 响应宏
 * @param[in]  : dataBuf --输出缓冲区指针, dataLen --输出缓冲区大小, result --响应结果 (RESULT_PASS / RESULT_FAIL), kvs --键值对数组(无数据时传 NULL),
 *               kvCount --键值对个数(无数据时传 0)
 * @return     : 无
 * @note       : 宏展开为调用 CJSON_ResultBuildKv(), 采用 do-while(0) 模式
 *              例子：CJSONKv_S kvs[] = {CJSON_MAKE_KV("vol", 220)};
 *              FT_ResultPrint(buf, sizeof(buf), RESULT_PASS, kvs, sizeof(kvs) / sizeof(kvs[0]));
 ******************************************************************************/
#define FT_ResultPrint(dataBuf, dataLen, result, kvs, kvCount)       \
    do {                                                             \
        CJSON_ResultBuildKv(dataBuf, dataLen, result, kvs, kvCount); \
    } while (0)

/******************************************************************************
 * @brief      : 构建支持 detail 嵌套 JSON 的多行响应
 * @param[in]  : dataBuf --输出缓冲区, dataLen --缓冲区大小, result --整体结果, kvsArray --每条 detail 的 KV 数组, kvCounts --每条 KV 数量, itemCount
 *               --detail数量
 * @param[out] : 无
 * @return     : 无
 * @note       : detail 内部支持 {"result","data":{...}} 结构
 ******************************************************************************/
#define FT_ResultPrintMultiLine(dataBuf, dataLen, result, items, itemCount)      \
    do {                                                                         \
        FT_ResultPrintMultiLineImpl(dataBuf, dataLen, result, items, itemCount); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif


/******************************************************************************
 * File Name : cjson_response.c
 * Function  : 响应JSON构造、解析和结构体字段注册接口实现。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#include "cjson_response_api.h"
#include <stdio.h>
#include <string.h>
#include "log_api.h"
#include "os_api.h"
#include "securec.h"

/******************************************************************************
 * @brief      : 安全追加字符串到缓冲区
 * @param[in]  : dataBuf --输出缓冲区指针, dataLen --输出缓冲区总大小, offset --当前写入偏移量指针（会被更新）, str --待追加的字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 如果缓冲区不足，则不追加，offset 不变；自动处理 '\0' 终止符
 ******************************************************************************/
static void CJSON_AppendString(char *dataBuf, size_t dataLen, size_t *offset, const char *str)
{
    if ((dataBuf == NULL) || (offset == NULL) || (str == NULL)) {
        return;
    }

    if (*offset >= dataLen) {
        LOG_Printf(SF_MID_FT, LOG_LEVEL_WARNING, "data len too small");
        return;
    }

    int ret = snprintf_s(dataBuf + *offset, dataLen - *offset, dataLen - *offset - 1, "%s", str);
    if (ret > 0) {
        *offset += (size_t)ret;
    }
}

/******************************************************************************
 * @brief      : 获取整数的字符串长度
 * @param[in]  : val --需要计算的整数
 * @param[out] : 无
 * @return     : 无
 * @note       : 无
 ******************************************************************************/
u32 CJSON_LonLongToStrSize(long long val)
{
    if (val == 0) {
        return 2; // 2表示0和\0
    }

    u32 len = (val < 0) ? 2 : 1; // 2 '-' + digit or digit
    unsigned long long uval;

    // 避免 ?: 类型冲突 + LLONG_MIN 溢出
    if (val < 0) {
        uval = (unsigned long long)(-(val + 1)) + 1ULL;
    } else {
        uval = (unsigned long long)val;
    }

    while (uval) {
        uval /= 10; // 10表示除后得到多个数
        len++;
    }

    return len;
}

/******************************************************************************
 * @brief      : 获取临时buf
 * @param[in]  : kv --键值对结构体指针
 * @param[out] : 无
 * @return     : 无
 * @note       : buf
 ******************************************************************************/
char *CJSON_GetTmpBuffer(const CJSONKv_S *kv, u32 *size)
{
    if ((kv == NULL) || (size == NULL)) {
        return NULL;
    }
    char *tmp = NULL;
    *size = 0;
    switch (kv->type) {
        case DATA_INT:
        case DATA_UINT:
            *size = (u32)strlen(kv->key) + 1 + CJSON_LonLongToStrSize(kv->intValue) + 1 + 3; // 3 "" 和 :
            tmp = (char *)OS_MemAlloc(SF_MID_FT, *size, sizeof(u32));
            break;

        case DATA_BOOL:
            u32 boolSize = (kv->boolValue == BOOL_TRUE) ? sizeof("true") : sizeof("false");
            *size = (u32)strlen(kv->key) + 1 + boolSize + 1 + 3; // 3 "" 和 :
            tmp = (char *)OS_MemAlloc(SF_MID_FT, *size, sizeof(u32));
            break;

        case DATA_STRING:
            u32 stringSize = (u32)((kv->stringValue != NULL) ? strlen(kv->stringValue) : 0) + 1;
            *size = (u32)strlen(kv->key) + 1 + stringSize + 1 + 3; // 3 "" 和 :
            tmp = (char *)OS_MemAlloc(SF_MID_FT, *size, sizeof(u32));
            break;

        case DATA_CHAR:
            *size = (u32)strlen(kv->key) + 1 + 3; // 3 "" 和 :
            tmp = (char *)OS_MemAlloc(SF_MID_FT, *size, sizeof(u32));
            break;

        default:
            break;
    }

    if (tmp == NULL) {
        LOG_Printf(SF_MID_FT, LOG_LEVEL_WARNING, "key value malloc fail");
        return NULL;
    }
    if (memset_s(tmp, *size, 0, *size) != EOK) {
        LOG_Printf(SF_MID_FT, LOG_LEVEL_WARNING, "key value memset fail");
        OS_MemFree(SF_MID_FT, tmp);
        return NULL;
    }
    return tmp;
}

/******************************************************************************
 * @brief      : 构建单个键值对的 JSON 字符串并追加到缓冲区
 * @param[in]  : dataBuf --输出缓冲区指针, dataLen --输出缓冲区总大小, offset --当前写入偏移量指针（会被更新）, kv --键值对结构体指针
 * @param[out] : 无
 * @return     : 无
 * @note       : 内部使用 0x100(256) 字节临时缓冲区，单个键值对不能超过此大小, 支持 6 种数据类型：INT, UINT, BOOL, DOUBLE, STRING, CHAR
 ******************************************************************************/
static void CJSON_AppendKv(char *dataBuf, size_t dataLen, size_t *offset, const CJSONKv_S *kv)
{
    if (kv == NULL) {
        return;
    }

    int ret;
    u32 size = 0;
    char *tmp = CJSON_GetTmpBuffer(kv, &size);
    if (tmp == NULL) {
        LOG_Printf(SF_MID_FT, LOG_LEVEL_WARNING, "Get tmp buf is null");
        return;
    }
    switch (kv->type) {
        case DATA_INT:
            ret = snprintf_s(tmp, size, size - 1, "\"%s\":%lld", kv->key, (long long)kv->intValue);
            break;

        case DATA_UINT:
            ret = snprintf_s(tmp, size, size - 1, "\"%s\":%llu", kv->key, (unsigned long long)kv->uintValue);
            break;

        case DATA_BOOL:
            ret = snprintf_s(tmp, size, size - 1, "\"%s\":%s", kv->key, (kv->boolValue == BOOL_TRUE) ? "true" : "false");
            break;

        case DATA_STRING:
            ret = snprintf_s(tmp, size, size - 1, "\"%s\":\"%s\"", kv->key, (kv->stringValue != NULL) ? kv->stringValue : "");
            break;

        case DATA_CHAR:
            ret = snprintf_s(tmp, size, size - 1, "\"%s\":\"%c\"", kv->key, kv->charValue);
            break;

        default:
            OS_MemFree(SF_MID_FT, tmp);
            return;
    }
    LOG_Printf(SF_MID_FT, LOG_LEVEL_WARNING, "ret:%d tmp:%s", ret, tmp);
    if (ret > 0) {
        CJSON_AppendString(dataBuf, dataLen, offset, tmp);
    }
    OS_MemFree(SF_MID_FT, tmp);
}

/******************************************************************************
 * @brief      : 构建完整的 JSON 响应并写入缓冲区
 * @param[in]  : dataBuf --输出缓冲区指针, dataLen --输出缓冲区大小, result --响应结果 (RESULT_PASS / RESULT_FAIL), kvs --键值对数组指针（无键值对时传 NULL）,
 *               kvCount --键值对个数（无键值对时传 0）
 * @param[out] : dataBuf --包含构造的 JSON 响应
 * @return     : 无
 * @note       : 构建失败时会把 dataBuf[0] 置为 '\0'
 ******************************************************************************/
void CJSON_ResultBuildKv(char *dataBuf, size_t dataLen, CJSONResult_E result, const CJSONKv_S *kvs, size_t kvCount)
{
    size_t offset = 0;
    size_t i;
    if ((dataBuf == NULL) || (dataLen <= 32)) { // 小于32字节开头都打不出来
        LOG_Printf(SF_MID_FT, LOG_LEVEL_WARNING, "dataLen :%zu too small", dataLen);
        return;
    }

    dataBuf[0] = '\0';

    CJSON_AppendString(dataBuf, dataLen, &offset, (result == RESULT_PASS) ? "{\"result\":\"pass\",\"data\":{" : "{\"result\":\"fail\",\"data\":{");
    for (i = 0; i < kvCount; i++) {
        if (i != 0) {
            CJSON_AppendString(dataBuf, dataLen, &offset, ",");
        }

        CJSON_AppendKv(dataBuf, dataLen, &offset, &kvs[i]);
    }

    CJSON_AppendString(dataBuf, dataLen, &offset, "}}\n");
}

/******************************************************************************
 * @brief      : 计算单个 item JSON 所需缓冲区大小
 * @param[in]  : kvs --键值对数组指针, kvCount --键值对数量
 * @param[out] : 无
 * @return     : size_t --建议分配的缓冲区大小
 * @note       : 用于动态分配 item JSON buffer，避免栈数组溢出
 ******************************************************************************/
size_t FT_CalcItemJsonSize(const CJSONKv_S *kvs, size_t kvCount)
{
    if (kvs == NULL) {
        LOG_Printf(SF_MID_FT, LOG_LEVEL_WARNING, "kvs item is null");
        return 0;
    }
    size_t size = 64; /* 64 基础JSON结构空间: { "result":"pass", "data":{} } */
    for (size_t i = 0; i < kvCount; i++) {
        if (kvs[i].key == NULL) {
            continue;
        }

        size += strlen(kvs[i].key); /* key字符串长度 */
        size += 4;                  /* 4 key格式: "key": */
        switch (kvs[i].type) {
            case DATA_STRING:
                if (kvs[i].stringValue) {
                    size += strlen(kvs[i].stringValue); /* value字符串长度 */
                }
                size += 2;                              /* 2 value两边的双引号 "" */
                break;

            case DATA_INT:
                size += 24; /* 24 int64最大长度: -9223372036854775808 + 结束余量 */
                break;

            case DATA_UINT:
                size += 24; /* 24 uint64最大长度: 18446744073709551615 + 结束余量 */
                break;

            case DATA_BOOL:
                size += 5; /* 5 false长度5，true长度4，取最大值 */
                break;

            case DATA_CHAR:
                size += 3; /* 3 单字符格式: "A" */
                break;

            default:
                break;
        }
    }

    size += 16; /* 16 = 2个大括号 {},逗号, 换行, 字符串结束符 \0,预留安全空间 */
    return size;
}

/******************************************************************************
 * @brief      : 构建单个 item JSON 字符串
 * @param[in]  : kvs --键值对数组, kvCount --键值对数量
 * @param[out] : dataBuf --输出 JSON 字符串缓冲区
 * @return     : 无
 * @note       : result 字段单独处理，其余字段走通用 KV 序列化
 ******************************************************************************/
static void FT_BuildItem(char *dataBuf, size_t dataLen, const CJSONKv_S *kvs, size_t kvCount)
{
    size_t offset = 0;
    bool firstData = TRUE;
    CJSON_AppendString(dataBuf, dataLen, &offset, "        {");

    // result 字段
    for (size_t i = 0; i < kvCount; i++) {
        if ((kvs[i].key != NULL) && (strcmp(kvs[i].key, "result") == 0)) {
            CJSON_AppendString(dataBuf, dataLen, &offset, "\"result\":\"");
            CJSON_AppendString(dataBuf, dataLen, &offset, kvs[i].stringValue ? kvs[i].stringValue : "pass");
            CJSON_AppendString(dataBuf, dataLen, &offset, "\",");
            break;
        }
    }

    // data 字段
    CJSON_AppendString(dataBuf, dataLen, &offset, "\"data\":{");
    for (size_t i = 0; i < kvCount; i++) {
        if (kvs[i].key == NULL) {
            continue;
        }

        if (strcmp(kvs[i].key, "result") == 0) {
            continue;
        }

        if (firstData != TRUE) {
            CJSON_AppendString(dataBuf, dataLen, &offset, ",");
        }

        CJSON_AppendKv(dataBuf, dataLen, &offset, &kvs[i]);
        firstData = FALSE;
    }

    CJSON_AppendString(dataBuf, dataLen, &offset, "}}");
}

/******************************************************************************
 * @brief      : 根据所有 item 中 result 字段生成 summary 结果字符串
 * @param[in]  : items --item数组指针, itemCount --item数量
 * @param[out] : 无
 * @return     : const char * --"test data pass" 或 "test data fail"
 * @note       : 任意 item result 为 fail，则整体失败
 ******************************************************************************/
static const char *FT_SummaryData(const CJSONItem_S *items, size_t itemCount)
{
    for (size_t i = 0; i < itemCount; i++) {
        const CJSONKv_S *kvs = items[i].kvs;
        for (size_t k = 0; k < items[i].kvCount; k++) {
            if (strcmp(kvs[k].key, "result") == 0 && kvs[k].stringValue && strcmp(kvs[k].stringValue, "fail") == 0) {
                return "test data fail";
            }
        }
    }
    return "test data pass";
}

/******************************************************************************
 * @brief      : 将结果枚举转换为字符串
 * @param[in]  : r --结果枚举
 * @param[out] : 无
 * @return     : const char * --"pass" 或 "fail"
 * @note       : 用于 JSON result 字段输出
 ******************************************************************************/
static const char *FT_ResultStr(CJSONResult_E result)
{
    return (result == RESULT_PASS) ? "pass" : "fail";
}

/******************************************************************************
 * @brief      : 构建多行 JSON 响应（detail数组结构扩展版）
 * @param[in]  : dataBuf --输出缓冲区, dataLen --缓冲区大小, result --调用方传入的命令执行结果, items --item数组, itemCount --item数量
 * @param[out] : dataBuf --完整 JSON 输出结果
 * @return     : 无
 * @note       : detail 内每个 item 独立包含 result + data，summary 自动统计 fail
 ******************************************************************************/
void FT_ResultPrintMultiLineImpl(char *dataBuf, size_t dataLen, CJSONResult_E result, const CJSONItem_S *items, size_t itemCount)
{
    size_t offset = 0;
    if (dataBuf == NULL || dataLen < 64) { // 小于64 还不够存 {} "" : ,
        LOG_Printf(SF_MID_FT, LOG_LEVEL_WARNING, "dataLen:%zu too small", dataLen);
        return;
    }

    dataBuf[0] = '\0';
    const char *summary = FT_SummaryData(items, itemCount);
    CJSON_AppendString(dataBuf, dataLen, &offset, "{\n    \"result\":\"");
    CJSON_AppendString(dataBuf, dataLen, &offset, FT_ResultStr(result));
    CJSON_AppendString(dataBuf, dataLen, &offset, "\",\n");
    CJSON_AppendString(dataBuf, dataLen, &offset, "    \"data\":\"");
    CJSON_AppendString(dataBuf, dataLen, &offset, summary);
    CJSON_AppendString(dataBuf, dataLen, &offset, "\",\n");
    CJSON_AppendString(dataBuf, dataLen, &offset, "    \"detail\":[\n");

    for (size_t i = 0; i < itemCount; i++) {
        u32 singleItemSize = (u32)FT_CalcItemJsonSize(items[i].kvs, items[i].kvCount);
        char *singleItemBuf = (char *)OS_MemAlloc(SF_MID_FT, singleItemSize, 1);
        if (singleItemBuf == NULL) {
            continue;
        }

        if (memset_s(singleItemBuf, singleItemSize, 0, singleItemSize) != EOK) {
            OS_MemFree(SF_MID_FT, singleItemBuf);
            continue;
        }

        FT_BuildItem(singleItemBuf, singleItemSize, items[i].kvs, items[i].kvCount);
        CJSON_AppendString(dataBuf, dataLen, &offset, singleItemBuf);
        if (i != itemCount - 1) {
            CJSON_AppendString(dataBuf, dataLen, &offset, ",");
        }
        CJSON_AppendString(dataBuf, dataLen, &offset, "\n");
        OS_MemFree(SF_MID_FT, singleItemBuf);
    }
    CJSON_AppendString(dataBuf, dataLen, &offset, "    ]\n}\n");
}

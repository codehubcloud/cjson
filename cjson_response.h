/******************************************************************************
 * File Name : cjson_response.h
 * Function  : 响应JSON构造、解析和结构体字段注册接口声明。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#ifndef CJSON_RESPONSE_H
#define CJSON_RESPONSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* 无效字段偏移量 */
#define CJSON_FIELD_INVALID_OFFSET ((size_t)(~(size_t)0U))

/* 布尔类型枚举 */
typedef enum {
    BOOL_FALSE = 0,
    BOOL_TRUE,
} CJSONBool_E;

/* 响应结果枚举 */
typedef enum {
    RESULT_FAIL = 0,
    RESULT_PASS,
} CJSONResult_E;

/* data字段类型枚举 */
typedef enum {
    DATA_EMPTY = 0,
    DATA_STRING,
    DATA_OBJECT,
    DATA_INT,
    DATA_UINT,
    DATA_BOOL,
    DATA_DOUBLE,
    DATA_CHAR,
} CJSONDataType_E;

/* 结构体字段类型枚举 */
typedef enum {
    FIELD_INT = 0,
    FIELD_UINT,
    FIELD_BOOL,
    FIELD_STRING,
    FIELD_DOUBLE,
    FIELD_CHAR,
} CJSONFieldType_E;

/* 字段描述结构体 */
typedef struct {
    const char *jsonKey;
    CJSONFieldType_E fieldType;
    size_t fieldOffset;
    size_t fieldSize;
} CJSONField_S;

/* 响应解析结果结构体 */
typedef struct {
    CJSONResult_E result;
    CJSONDataType_E dataType;
    char *dataString;
    void *dataObject;
    int64_t intValue;
    uint64_t uintValue;
    long double doubleValue;
    CJSONBool_E boolValue;
    char charValue;
} CJSONResponse_S;

/* 响应JSON构建参数结构体 */
typedef struct {
    char *dataBuf;
    size_t dataLen;
    CJSONDataType_E dataType;
    CJSONResult_E result;
    const void *data;
    const CJSONField_S *fields;
    int64_t intValue;
    uint64_t uintValue;
    long double doubleValue;
    CJSONBool_E boolValue;
    char charValue;
} CJSONBuildParam_S;

/* 响应JSON解析参数结构体 */
typedef struct {
    const char *jsonString;
    CJSONDataType_E dataType;
    CJSONResponse_S *response;
    void *data;
    const CJSONField_S *fields;
} CJSONParseParam_S;

/* 获取结构体成员偏移量 */
#define CJSON_OFFSET_OF(type, member) ((size_t)&(((type *)0)->member))

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)

/* 根据成员类型自动推导JSON字段类型 */
#define CJSON_FIELD_TYPE_OF(value)      \
    _Generic((value),                   \
        _Bool: FIELD_BOOL,              \
        char: FIELD_CHAR,               \
        signed char: FIELD_INT,         \
        unsigned char: FIELD_UINT,      \
        short: FIELD_INT,               \
        unsigned short: FIELD_UINT,     \
        int: FIELD_INT,                 \
        unsigned int: FIELD_UINT,       \
        long: FIELD_INT,                \
        unsigned long: FIELD_UINT,      \
        long long: FIELD_INT,           \
        unsigned long long: FIELD_UINT, \
        float: FIELD_DOUBLE,            \
        double: FIELD_DOUBLE,           \
        long double: FIELD_DOUBLE,      \
        char *: FIELD_STRING,           \
        const char *: FIELD_STRING)

/* 自动生成字段描述 */
#define CJSON_FIELD_AUTO_DESC(type, member, key) {(key), CJSON_FIELD_TYPE_OF(((type *)0)->member), CJSON_OFFSET_OF(type, member), sizeof(((type *)0)->member)}

/* 使用成员名作为JSON键名 */
#define CJSON_FIELD_KEY(type, member) CJSON_FIELD_AUTO_DESC(type, member, #member)

#else
#error "FT_RESULT requires C11 _Generic support. Please enable C11."
#endif

/* 字段描述表结束标记 */
#define CJSON_FIELD_END() {NULL, FIELD_STRING, CJSON_FIELD_INVALID_OFFSET, 0U}

/* 根据结构体类型生成字段描述表名称 */
#define CJSON_OBJECT_FIELDS_NAME(type) g_cjsonFields_##type

/* 根据结构体类型获取字段描述表 */
#define CJSON_OBJECT_FIELDS(type) CJSON_OBJECT_FIELDS_NAME(type)

/* 自动字段描述宏 */
#define CJSON_OBJECT_AUTO_FIELD_DESC(type, member) CJSON_FIELD_KEY(type, member),

/*
 * 说明：
 * 以下预处理器循环宏必须放在头文件中，因为结构体字段注册发生在业务代码编译阶段。
 * 它们属于字段注册支撑宏，不建议业务代码直接调用。
 */
#define CJSON_PP_EXPAND(...) __VA_ARGS__

#define CJSON_PP_FOREACH_1(macro, type, item1) macro(type, item1)
#define CJSON_PP_FOREACH_2(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_1(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_3(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_2(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_4(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_3(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_5(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_4(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_6(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_5(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_7(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_6(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_8(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_7(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_9(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_8(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_10(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_9(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_11(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_10(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_12(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_11(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_13(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_12(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_14(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_13(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_15(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_14(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_16(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_15(macro, type, __VA_ARGS__)

#define CJSON_PP_FOREACH_SELECT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, name, ...) name

#define CJSON_PP_FOREACH(macro, type, ...)                                                                                                         \
    CJSON_PP_EXPAND(CJSON_PP_FOREACH_SELECT(__VA_ARGS__, CJSON_PP_FOREACH_16, CJSON_PP_FOREACH_15, CJSON_PP_FOREACH_14, CJSON_PP_FOREACH_13,       \
                                            CJSON_PP_FOREACH_12, CJSON_PP_FOREACH_11, CJSON_PP_FOREACH_10, CJSON_PP_FOREACH_9, CJSON_PP_FOREACH_8, \
                                            CJSON_PP_FOREACH_7, CJSON_PP_FOREACH_6, CJSON_PP_FOREACH_5, CJSON_PP_FOREACH_4, CJSON_PP_FOREACH_3,    \
                                            CJSON_PP_FOREACH_2, CJSON_PP_FOREACH_1)(macro, type, __VA_ARGS__))

/*
 * 业务侧定义对象列表：
 *
 * #undef FT_RESULT_OBJECT_LIST
 * #define FT_RESULT_OBJECT_LIST(X) \
 *     X(VoltageInfo_S, vol, cap)
 *
 * FT_RESULT_REGISTER_OBJECTS()
 */
#ifndef FT_RESULT_OBJECT_LIST
#define FT_RESULT_OBJECT_LIST(X)
#endif

/* 注册单个对象类型字段描述表 */
#define FT_RESULT_REGISTER_OBJECT_ONE(type, ...) \
    static const CJSONField_S CJSON_OBJECT_FIELDS_NAME(type)[] = {CJSON_PP_FOREACH(CJSON_OBJECT_AUTO_FIELD_DESC, type, __VA_ARGS__) CJSON_FIELD_END()};

/* 批量注册对象类型字段描述表 */
#define FT_RESULT_REGISTER_OBJECTS() FT_RESULT_OBJECT_LIST(FT_RESULT_REGISTER_OBJECT_ONE)

/* 生成_Generic对象字段表映射项 */
#define FT_RESULT_OBJECT_FIELDS_CASE(type, ...) type * : CJSON_OBJECT_FIELDS(type), const type * : CJSON_OBJECT_FIELDS(type),

/* 根据data指针类型自动推导对象字段描述表；非对象类型返回NULL */
#define FT_RESULT_FIELDS_BY_DATA(dataArg) \
    _Generic((dataArg), char *: NULL, const char *: NULL, void *: NULL, const void *: NULL, FT_RESULT_OBJECT_LIST(FT_RESULT_OBJECT_FIELDS_CASE) default: NULL)

/* 根据data表达式自动填充构建参数；该宏由FT_RESULT_BUILD内部使用 */
#define FT_RESULT_SET_BUILD_DATA(buildParam, dataArg) \
    _Generic((dataArg),                               \
        _Bool: CJSON_BuildParamSetBool,               \
        char: CJSON_BuildParamSetChar,                \
        signed char: CJSON_BuildParamSetInt,          \
        unsigned char: CJSON_BuildParamSetUInt,       \
        short: CJSON_BuildParamSetInt,                \
        unsigned short: CJSON_BuildParamSetUInt,      \
        int: CJSON_BuildParamSetInt,                  \
        unsigned int: CJSON_BuildParamSetUInt,        \
        long: CJSON_BuildParamSetInt,                 \
        unsigned long: CJSON_BuildParamSetUInt,       \
        long long: CJSON_BuildParamSetInt,            \
        unsigned long long: CJSON_BuildParamSetUInt,  \
        float: CJSON_BuildParamSetDouble,             \
        double: CJSON_BuildParamSetDouble,            \
        long double: CJSON_BuildParamSetDouble,       \
        char *: CJSON_BuildParamSetPointer,           \
        const char *: CJSON_BuildParamSetPointer,     \
        void *: CJSON_BuildParamSetPointer,           \
        const void *: CJSON_BuildParamSetPointer,     \
        default: CJSON_BuildParamSetPointer)((buildParam), (dataArg))

/* 统一构建响应JSON到外部缓冲区 */
#define FT_ResultPrint(typeArg, outBufArg, outLenArg, resultArg, dataArg) \
    do {                                                                  \
        CJSONBuildParam_S cjsonBuildParam = {0};                          \
        cjsonBuildParam.dataType = (typeArg);                             \
        cjsonBuildParam.dataBuf = (outBufArg);                            \
        cjsonBuildParam.dataLen = (outLenArg);                            \
        cjsonBuildParam.result = (resultArg);                             \
        cjsonBuildParam.fields = FT_RESULT_FIELDS_BY_DATA(dataArg);       \
        FT_RESULT_SET_BUILD_DATA(&cjsonBuildParam, dataArg);              \
        CJSON_ResultBuild(&cjsonBuildParam);                              \
    } while (0)

/* 统一解析响应JSON */
#define FT_ResultParse(typeArg, jsonArg, responseArg, dataArg) \
    CJSON_ResultParse(&(const CJSONParseParam_S){              \
        (jsonArg),                                             \
        (typeArg),                                             \
        (responseArg),                                         \
        (void *)(dataArg),                                     \
        FT_RESULT_FIELDS_BY_DATA(dataArg),                     \
    })

/******************************************************************************
 * @brief      : 构建参数设置为有符号整数
 * @param[in]  : value --有符号整数值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_RESULT_BUILD宏内部使用，业务代码不要直接调用
 ******************************************************************************/
void CJSON_BuildParamSetInt(CJSONBuildParam_S *buildParam, int64_t value);

/******************************************************************************
 * @brief      : 构建参数设置为无符号整数
 * @param[in]  : value --无符号整数值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_RESULT_BUILD宏内部使用，业务代码不要直接调用
 ******************************************************************************/
void CJSON_BuildParamSetUInt(CJSONBuildParam_S *buildParam, uint64_t value);

/******************************************************************************
 * @brief      : 构建参数设置为布尔值
 * @param[in]  : value --布尔值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_RESULT_BUILD宏内部使用，业务代码不要直接调用
 ******************************************************************************/
void CJSON_BuildParamSetBool(CJSONBuildParam_S *buildParam, int value);

/******************************************************************************
 * @brief      : 构建参数设置为浮点数
 * @param[in]  : value --浮点数值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_RESULT_BUILD宏内部使用，业务代码不要直接调用
 ******************************************************************************/
void CJSON_BuildParamSetDouble(CJSONBuildParam_S *buildParam, long double value);

/******************************************************************************
 * @brief      : 构建参数设置为单字符
 * @param[in]  : value --字符值
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_RESULT_BUILD宏内部使用，业务代码不要直接调用
 ******************************************************************************/
void CJSON_BuildParamSetChar(CJSONBuildParam_S *buildParam, char value);

/******************************************************************************
 * @brief      : 构建参数设置为指针数据
 * @param[in]  : value --字符串或对象指针
 * @param[out] : buildParam --构建参数
 * @return     : 无
 * @note       : 供FT_RESULT_BUILD宏内部使用，业务代码不要直接调用
 ******************************************************************************/
void CJSON_BuildParamSetPointer(CJSONBuildParam_S *buildParam, const void *value);

/******************************************************************************
 * @brief      : 构建响应JSON到外部缓冲区
 * @param[in]  : buildParam --构建参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 构建失败时buildParam->dataBuf会被置为空字符串
 ******************************************************************************/
void CJSON_ResultBuild(const CJSONBuildParam_S *buildParam);

/******************************************************************************
 * @brief      : 解析响应JSON
 * @param[in]  : parseParam --解析参数
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 支持空字符串、字符串、对象、整数、无符号整数、布尔值、浮点数和字符
 ******************************************************************************/
CJSONBool_E CJSON_ResultParse(const CJSONParseParam_S *parseParam);

#ifdef __cplusplus
}
#endif

#endif

/******************************************************************************
 * File Name : cjson_response.h
 * Function  : 响应JSON构造、解析、结构体序列化/反序列化、批量字段宏和日志输出接口声明。
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

/* 结果键名宏 */
#define CJSON_KEY_RESULT             ("result")
/* 数据键名宏 */
#define CJSON_KEY_DATA               ("data")

/* 成功结果字符串宏 */
#define CJSON_RESULT_PASS_STRING     ("pass")
/* 失败结果字符串宏 */
#define CJSON_RESULT_FAIL_STRING     ("fail")

/* 空字符串宏 */
#define CJSON_STRING_EMPTY           ("")
/* 对象开始符号宏 */
#define CJSON_OBJECT_BEGIN           ("{")
/* 对象结束符号宏 */
#define CJSON_OBJECT_END             ("}")

/* 信息日志级别字符串宏 */
#define CJSON_LOG_LEVEL_INFO_STRING  ("INFO")
/* 警告日志级别字符串宏 */
#define CJSON_LOG_LEVEL_WARN_STRING  ("WARN")
/* 错误日志级别字符串宏 */
#define CJSON_LOG_LEVEL_ERROR_STRING ("ERROR")

/* 无效字段偏移量宏 */
#define CJSON_FIELD_INVALID_OFFSET   ((size_t)(~0U))

/* 布尔类型枚举 */
typedef enum {
    CJSON_BOOL_FALSE = 0,
    CJSON_BOOL_TRUE,
} CJSONBool_E;

/* 结果枚举 */
typedef enum {
    CJSON_RESULT_FAIL = 0,
    CJSON_RESULT_PASS,
} CJSONResult_E;

/* 数据类型枚举 */
typedef enum {
    CJSON_DATA_EMPTY = 0,
    CJSON_DATA_STRING,
    CJSON_DATA_OBJECT,
} CJSONDataType_E;

/* 字段类型枚举 */
typedef enum {
    CJSON_FIELD_INT = 0,
    CJSON_FIELD_UINT,
    CJSON_FIELD_DOUBLE,
    CJSON_FIELD_BOOL,
    CJSON_FIELD_STRING,
} CJSONFieldType_E;

/* 日志级别枚举 */
typedef enum {
    CJSON_LOG_LEVEL_INFO = 0,
    CJSON_LOG_LEVEL_WARN,
    CJSON_LOG_LEVEL_ERROR,
} CJSONLogLevel_E;

/* 字段描述结构体 */
typedef struct {
    const char* jsonKey;
    CJSONFieldType_E fieldType;
    size_t fieldOffset;
    size_t fieldSize;
} CJSONField_S;

/* 响应结构体 */
typedef struct {
    CJSONResult_E result;
    CJSONDataType_E dataType;
    char* dataString;
    void* dataObject;
} CJSONResponse_S;

#define CJSON_FIELD_BOOL_DESC(type, member, key)   {(key), CJSON_FIELD_BOOL, CJSON_OFFSET_OF(type, member), sizeof(((type*)0)->member)}

#define CJSON_OFFSET_OF(type, member)              ((size_t)&(((type*)0)->member))

/******************************************************************************
 * @brief      : 获取结构体成员偏移量
 * @param[in]  : type --结构体类型, member --成员名
 * @param[out] : 无
 * @return     : 返回成员在结构体中的偏移量
 * @note       : 用于字段描述宏中计算偏移量
 ******************************************************************************/
#define CJSON_FIELD_INT_DESC(type, member, key)    {(key), CJSON_FIELD_INT, CJSON_OFFSET_OF(type, member), sizeof(((type*)0)->member)}

/******************************************************************************
 * @brief      : 获取结构体无符号整型成员描述
 * @param[in]  : type --结构体类型, member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 返回CJSONField_S结构体描述
 * @note       : 用于构建字段描述表
 ******************************************************************************/
#define CJSON_FIELD_UINT_DESC(type, member, key)   {(key), CJSON_FIELD_UINT, CJSON_OFFSET_OF(type, member), sizeof(((type*)0)->member)}

/******************************************************************************
 * @brief      : 获取结构体浮点型成员描述
 * @param[in]  : type --结构体类型, member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 返回CJSONField_S结构体描述
 * @note       : 用于构建字段描述表
 ******************************************************************************/
#define CJSON_FIELD_DOUBLE_DESC(type, member, key) {(key), CJSON_FIELD_DOUBLE, CJSON_OFFSET_OF(type, member), sizeof(((type*)0)->member)}

/******************************************************************************
 * @brief      : 获取结构体布尔型成员描述
 * @param[in]  : type --结构体类型, member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 返回CJSONField_S结构体描述
 * @note       : 用于构建字段描述表
 ******************************************************************************/
#define CJSON_FIELD_BOOL_DESC(type, member, key)   {(key), CJSON_FIELD_BOOL, CJSON_OFFSET_OF(type, member), sizeof(((type*)0)->member)}

/******************************************************************************
 * @brief      : 获取结构体字符串成员描述
 * @param[in]  : type --结构体类型, member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 返回CJSONField_S结构体描述
 * @note       : 用于构建字段描述表
 ******************************************************************************/
#define CJSON_FIELD_STRING_DESC(type, member, key) {(key), CJSON_FIELD_STRING, CJSON_OFFSET_OF(type, member), sizeof(((type*)0)->member)}

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
/******************************************************************************
 * @brief      : 根据值类型推导字段类型
 * @param[in]  : value --待推导的值
 * @param[out] : 无
 * @return     : 返回对应的CJSONFieldType_E类型
 * @note       : 使用C11 _Generic特性进行类型推导
 ******************************************************************************/
#define CJSON_FIELD_TYPE_OF(value)            \
    _Generic((value),                         \
        _Bool: CJSON_FIELD_BOOL,              \
        char: CJSON_FIELD_INT,                \
        signed char: CJSON_FIELD_INT,         \
        unsigned char: CJSON_FIELD_UINT,      \
        short: CJSON_FIELD_INT,               \
        unsigned short: CJSON_FIELD_UINT,     \
        int: CJSON_FIELD_INT,                 \
        unsigned int: CJSON_FIELD_UINT,       \
        long: CJSON_FIELD_INT,                \
        unsigned long: CJSON_FIELD_UINT,      \
        long long: CJSON_FIELD_INT,           \
        unsigned long long: CJSON_FIELD_UINT, \
        float: CJSON_FIELD_DOUBLE,            \
        double: CJSON_FIELD_DOUBLE,           \
        long double: CJSON_FIELD_DOUBLE,      \
        char*: CJSON_FIELD_STRING,            \
        const char*: CJSON_FIELD_STRING)

/******************************************************************************
 * @brief      : 自动推导字段描述
 * @param[in]  : type --结构体类型, member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 返回CJSONField_S结构体描述
 * @note       : 结合类型推导和偏移量计算
 ******************************************************************************/
#define CJSON_FIELD_AUTO_DESC(type, member, key) {(key), CJSON_FIELD_TYPE_OF(((type*)0)->member), CJSON_OFFSET_OF(type, member), sizeof(((type*)0)->member)}

/******************************************************************************
 * @brief      : 自动字段描述宏
 * @param[in]  : type --结构体类型, member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 返回CJSONField_S结构体描述
 * @note       : 自动推导类型并生成字段描述
 ******************************************************************************/
#define CJSON_FIELD(type, member, key)           CJSON_FIELD_AUTO_DESC(type, member, key)

/******************************************************************************
 * @brief      : 根据成员名生成字段描述
 * @param[in]  : type --结构体类型, member --成员名
 * @param[out] : 无
 * @return     : 返回CJSONField_S结构体描述
 * @note       : 使用成员名作为JSON键名
 ******************************************************************************/
#define CJSON_FIELD_KEY(type, member)            CJSON_FIELD_AUTO_DESC(type, member, #member)
#endif

/******************************************************************************
 * @brief      : 字段描述表结束标记
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 返回结束标记的CJSONField_S结构体
 * @note       : 用于字段描述表的结尾
 ******************************************************************************/
#define CJSON_FIELD_END()            {NULL, CJSON_FIELD_STRING, CJSON_FIELD_INVALID_OFFSET, 0U}

/******************************************************************************
 * @brief      : 开始定义字段描述表
 * @param[in]  : name --字段描述表名称
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义静态字段描述表
 ******************************************************************************/
#define CJSON_FIELD_LIST_BEGIN(name) static const CJSONField_S name[] = {
/******************************************************************************
 * @brief      : 结束字段描述表定义
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结束字段描述表定义
 ******************************************************************************/
#define CJSON_FIELD_LIST_END() \
    CJSON_FIELD_END()          \
    }

/******************************************************************************
 * @brief      : 获取对象字段描述表名称
 * @param[in]  : type --结构体类型
 * @param[out] : 无
 * @return     : 返回字段描述表名称
 * @note       : 生成全局字段描述表名称
 ******************************************************************************/
#define CJSON_OBJECT_FIELDS_NAME(type)  g_cjsonFields_##type

/******************************************************************************
 * @brief      : 获取对象字段描述表
 * @param[in]  : type --结构体类型
 * @param[out] : 无
 * @return     : 返回字段描述表指针
 * @note       : 用于获取字段描述表
 ******************************************************************************/
#define CJSON_OBJECT_FIELDS(type)       CJSON_OBJECT_FIELDS_NAME(type)

/******************************************************************************
 * @brief      : 开始定义对象字段描述表
 * @param[in]  : type --结构体类型
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义静态字段描述表
 ******************************************************************************/
#define CJSON_OBJECT_FIELDS_BEGIN(type) static const CJSONField_S CJSON_OBJECT_FIELDS_NAME(type)[] = {
/******************************************************************************
 * @brief      : 结束对象字段描述表定义
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结束字段描述表定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELDS_END() \
    CJSON_FIELD_END()             \
    }

/******************************************************************************
 * @brief      : 定义对象字段描述表
 * @param[in]  : type --结构体类型, ... --字段列表
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义静态字段描述表
 ******************************************************************************/
#define CJSON_DEFINE_OBJECT_FIELDS(type, ...)      static const CJSONField_S CJSON_OBJECT_FIELDS_NAME(type)[] = {__VA_ARGS__, CJSON_FIELD_END()}

/******************************************************************************
 * @brief      : 自动字段描述宏
 * @param[in]  : type --结构体类型, member --成员名
 * @param[out] : 无
 * @return     : 返回字段描述
 * @note       : 用于自动构建字段描述
 ******************************************************************************/
#define CJSON_OBJECT_AUTO_FIELD_DESC(type, member) CJSON_FIELD_KEY(type, member),

/******************************************************************************
 * @brief      : 注册对象类型
 * @param[in]  : type --结构体类型, ... --字段列表
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于注册结构体类型和字段描述
 ******************************************************************************/
#define CJSON_REGISTER_OBJECT_TYPE(type, ...) \
    static const CJSONField_S CJSON_OBJECT_FIELDS_NAME(type)[] = {CJSON_PP_FOREACH(CJSON_OBJECT_AUTO_FIELD_DESC, type, __VA_ARGS__) CJSON_FIELD_END()}

/******************************************************************************
 * @brief      : 定义对象字段类型 - 整型
 * @param[in]  : member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义结构体字段类型
 ******************************************************************************/
#define CJSON_OBJECT_INT(member, key) (INT, member, key)

/******************************************************************************
 * @brief      : 定义对象字段类型 - 无符号整型
 * @param[in]  : member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义结构体字段类型
 ******************************************************************************/
#define CJSON_OBJECT_UINT(member, key) (UINT, member, key)

/******************************************************************************
 * @brief      : 定义对象字段类型 - 浮点型
 * @param[in]  : member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义结构体字段类型
 ******************************************************************************/
#define CJSON_OBJECT_DOUBLE(member, key) (DOUBLE, member, key)

/******************************************************************************
 * @brief      : 定义对象字段类型 - 布尔型
 * @param[in]  : member --成员名, key --JSON键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义结构体字段类型
 ******************************************************************************/
#define CJSON_OBJECT_BOOL(member, key) (BOOL, member, key)

/******************************************************************************
 * @brief      : 定义对象字段类型 - 字符串
 * @param[in]  : member --成员名, size --字符串大小, key --JSON键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义结构体字段类型
 ******************************************************************************/
#define CJSON_OBJECT_STRING(member, size, key) (STRING, member, size, key)

/******************************************************************************
 * @brief      : 展开宏参数
 * @param[in]  : ... --参数列表
 * @param[out] : 无
 * @return     : 展开后的参数
 * @note       : 用于预处理器参数展开
 ******************************************************************************/
#define CJSON_PP_EXPAND(...) __VA_ARGS__

/******************************************************************************
 * @brief      : 解包宏参数
 * @param[in]  : ... --参数列表
 * @param[out] : 无
 * @return     : 解包后的参数
 * @note       : 用于预处理器参数解包
 ******************************************************************************/
#define CJSON_PP_UNPACK(...) __VA_ARGS__

/******************************************************************************
 * @brief      : 预处理器循环宏 - 1个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_1(macro, type, item1) macro(type, item1)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 2个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_2(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_1(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 3个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_3(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_2(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 4个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_4(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_3(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 5个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_5(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_4(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 6个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_6(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_5(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 7个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_7(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_6(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 8个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_8(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_7(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 9个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_9(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_8(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 10个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_10(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_9(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 11个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_11(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_10(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 12个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_12(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_11(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 13个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_13(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_12(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 14个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_14(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_13(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 15个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_15(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_14(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏 - 16个参数
 * @param[in]  : macro --宏名, type --类型, item1 --参数1, ... --其他参数
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环
 ******************************************************************************/
#define CJSON_PP_FOREACH_16(macro, type, item1, ...) macro(type, item1) CJSON_PP_FOREACH_15(macro, type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 预处理器循环宏选择器
 * @param[in]  : _1, _2, ... --参数列表
 * @param[out] : 无
 * @return     : 选择的宏
 * @note       : 用于选择合适的循环宏
 ******************************************************************************/
#define CJSON_PP_FOREACH_SELECT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, name, ...) name

/******************************************************************************
 * @brief      : 预处理器循环宏
 * @param[in]  : macro --宏名, type --类型, ... --参数列表
 * @param[out] : 无
 * @return     : 宏调用结果
 * @note       : 用于预处理器循环调用
 ******************************************************************************/
#define CJSON_PP_FOREACH(macro, type, ...)                                                                                                         \
    CJSON_PP_EXPAND(CJSON_PP_FOREACH_SELECT(__VA_ARGS__, CJSON_PP_FOREACH_16, CJSON_PP_FOREACH_15, CJSON_PP_FOREACH_14, CJSON_PP_FOREACH_13,       \
                                            CJSON_PP_FOREACH_12, CJSON_PP_FOREACH_11, CJSON_PP_FOREACH_10, CJSON_PP_FOREACH_9, CJSON_PP_FOREACH_8, \
                                            CJSON_PP_FOREACH_7, CJSON_PP_FOREACH_6, CJSON_PP_FOREACH_5, CJSON_PP_FOREACH_4, CJSON_PP_FOREACH_3,    \
                                            CJSON_PP_FOREACH_2, CJSON_PP_FOREACH_1)(macro, type, __VA_ARGS__))

/******************************************************************************
 * @brief      : 结构体字段定义 - 评估
 * @param[in]  : type --结构体类型, field --字段
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT(type, field)                  CJSON_OBJECT_FIELD_STRUCT_EVAL(type, field)

/******************************************************************************
 * @brief      : 结构体字段定义 - 评估2
 * @param[in]  : type --结构体类型, field --字段
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_EVAL(type, field)             CJSON_OBJECT_FIELD_STRUCT_EVAL2(type, CJSON_PP_UNPACK field)

/******************************************************************************
 * @brief      : 结构体字段定义 - 评估3
 * @param[in]  : type --结构体类型, ... --参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_EVAL2(type, ...)              CJSON_OBJECT_FIELD_STRUCT_IMPL(type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 结构体字段定义 - 实现
 * @param[in]  : type --结构体类型, kind --类型, ... --参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_IMPL(type, kind, ...)         CJSON_OBJECT_FIELD_STRUCT_IMPL2(type, kind, __VA_ARGS__)

/******************************************************************************
 * @brief      : 结构体字段定义 - 实现2
 * @param[in]  : type --结构体类型, kind --类型, ... --参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_IMPL2(type, kind, ...)        CJSON_OBJECT_FIELD_STRUCT_##kind(__VA_ARGS__)

/******************************************************************************
 * @brief      : 结构体字段定义 - 整型
 * @param[in]  : member --成员名, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_INT(member, key)              int member;

/******************************************************************************
 * @brief      : 结构体字段定义 - 无符号整型
 * @param[in]  : member --成员名, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_UINT(member, key)             unsigned int member;

/******************************************************************************
 * @brief      : 结构体字段定义 - 浮点型
 * @param[in]  : member --成员名, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_DOUBLE(member, key)           double member;

/******************************************************************************
 * @brief      : 结构体字段定义 - 布尔型
 * @param[in]  : member --成员名, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_BOOL(member, key)             CJSONBool_E member;

/******************************************************************************
 * @brief      : 结构体字段定义 - 字符串
 * @param[in]  : member --成员名, size --大小, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段定义
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_STRUCT_STRING(member, size, key)     char member[size];

/******************************************************************************
 * @brief      : 结构体字段描述 - 评估
 * @param[in]  : type --结构体类型, field --字段
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC(type, field)                    CJSON_OBJECT_FIELD_DESC_EVAL(type, field)

/******************************************************************************
 * @brief      : 结构体字段描述 - 评估2
 * @param[in]  : type --结构体类型, field --字段
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_EVAL(type, field)               CJSON_OBJECT_FIELD_DESC_EVAL2(type, CJSON_PP_UNPACK field)

/******************************************************************************
 * @brief      : 结构体字段描述 - 评估3
 * @param[in]  : type --结构体类型, ... --参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_EVAL2(type, ...)                CJSON_OBJECT_FIELD_DESC_IMPL(type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 结构体字段描述 - 实现
 * @param[in]  : type --结构体类型, kind --类型, ... --参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_IMPL(type, kind, ...)           CJSON_OBJECT_FIELD_DESC_IMPL2(type, kind, __VA_ARGS__)

/******************************************************************************
 * @brief      : 结构体字段描述 - 实现2
 * @param[in]  : type --结构体类型, kind --类型, ... --参数
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_IMPL2(type, kind, ...)          CJSON_OBJECT_FIELD_DESC_##kind(type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 结构体字段描述 - 整型
 * @param[in]  : type --结构体类型, member --成员名, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_INT(type, member, key)          CJSON_FIELD_INT_DESC(type, member, key),

/******************************************************************************
 * @brief      : 结构体字段描述 - 无符号整型
 * @param[in]  : type --结构体类型, member --成员名, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_UINT(type, member, key)         CJSON_FIELD_UINT_DESC(type, member, key),

/******************************************************************************
 * @brief      : 结构体字段描述 - 浮点型
 * @param[in]  : type --结构体类型, member --成员名, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_DOUBLE(type, member, key)       CJSON_FIELD_DOUBLE_DESC(type, member, key),

/******************************************************************************
 * @brief      : 结构体字段描述 - 布尔型
 * @param[in]  : type --结构体类型, member --成员名, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_BOOL(type, member, key)         CJSON_FIELD_BOOL_DESC(type, member, key),

/******************************************************************************
 * @brief      : 结构体字段描述 - 字符串
 * @param[in]  : type --结构体类型, member --成员名, size --大小, key --键名
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于结构体字段描述
 ******************************************************************************/
#define CJSON_OBJECT_FIELD_DESC_STRING(type, member, size, key) CJSON_FIELD_STRING_DESC(type, member, key),

/******************************************************************************
 * @brief      : 注册对象类型字段
 * @param[in]  : type --结构体类型, ... --字段列表
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于注册对象类型和字段描述
 ******************************************************************************/
#define CJSON_REGISTER_OBJECT_TYPE_WITH_FIELDS(type, ...) \
    static const CJSONField_S CJSON_OBJECT_FIELDS_NAME(type)[] = {CJSON_PP_FOREACH(CJSON_OBJECT_FIELD_DESC, type, __VA_ARGS__) CJSON_FIELD_END()}

/******************************************************************************
 * @brief      : 定义对象类型
 * @param[in]  : type --结构体类型, ... --字段列表
 * @param[out] : 无
 * @return     : 无
 * @note       : 用于定义结构体类型和字段描述
 ******************************************************************************/
#define CJSON_DEFINE_OBJECT_TYPE(type, ...)                            \
    typedef struct {                                                   \
        CJSON_PP_FOREACH(CJSON_OBJECT_FIELD_STRUCT, type, __VA_ARGS__) \
    } type;                                                            \
    CJSON_REGISTER_OBJECT_TYPE_WITH_FIELDS(type, __VA_ARGS__)

/******************************************************************************
 * @brief      : 构建空响应
 * @param[in]  : result --结果枚举
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":""}
 ******************************************************************************/
#define CJSON_RESPONSE_BUILD_EMPTY(result)                              CJSON_BuildEmptyResponse((result))

/******************************************************************************
 * @brief      : 构建字符串响应
 * @param[in]  : result --结果枚举, dataString --data字符串
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":"xxxx"}
 ******************************************************************************/
#define CJSON_RESPONSE_BUILD_STRING(result, dataString)                 CJSON_BuildStringResponse((result), (dataString))

/******************************************************************************
 * @brief      : 构建字段描述响应
 * @param[in]  : result --结果枚举, object --结构体地址, fields --字段描述表
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":{"key":value}}
 ******************************************************************************/
#define CJSON_RESPONSE_BUILD_OBJECT_WITH_FIELDS(result, object, fields) CJSON_BuildObjectResponse((result), (object), (fields))

/******************************************************************************
 * @brief      : 构建对象响应
 * @param[in]  : result --结果枚举, object --结构体地址, type --结构体类型
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":{"key":value}}
 ******************************************************************************/
#define CJSON_RESPONSE_BUILD_OBJECT(result, object, type)               CJSON_BuildObjectResponse((result), (object), CJSON_OBJECT_FIELDS(type))

/******************************************************************************
 * @brief      : 构建自动对象响应
 * @param[in]  : result --结果枚举, object --结构体地址, type --结构体类型, ... --字段列表
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":{"key":value}}
 ******************************************************************************/
#define CJSON_RESPONSE_BUILD_OBJECT_AUTO(result, object, type, ...) \
    CJSON_BuildObjectResponse((result), (object), (const CJSONField_S[]){CJSON_PP_FOREACH(CJSON_OBJECT_AUTO_FIELD_DESC, type, __VA_ARGS__) CJSON_FIELD_END()})

/******************************************************************************
 * @brief      : 解析对象响应
 * @param[in]  : jsonString --响应JSON字符串, response --响应结构体, object --结构体地址, objectSize --结构体大小, type --结构体类型
 * @param[out] : 无
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : data是对象时使用fields解析
 ******************************************************************************/
#define CJSON_PARSE_RESPONSE_OBJECT(jsonString, response, object, objectSize, type) \
    CJSON_ParseResponse((jsonString), (response), (object), (objectSize), CJSON_OBJECT_FIELDS(type))

/******************************************************************************
 * @brief      : 解析自动对象响应
 * @param[in]  : jsonString --响应JSON字符串, response --响应结构体, object --结构体地址, objectSize --结构体大小, type --结构体类型, ... --字段列表
 * @param[out] : 无
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : data是对象时使用fields解析
 ******************************************************************************/
#define CJSON_PARSE_RESPONSE_OBJECT_AUTO(jsonString, response, object, objectSize, type, ...) \
    CJSON_ParseResponse((jsonString), (response), (object), (objectSize),                     \
                        (const CJSONField_S[]){CJSON_PP_FOREACH(CJSON_OBJECT_AUTO_FIELD_DESC, type, __VA_ARGS__) CJSON_FIELD_END()})
/******************************************************************************
 * @brief      : 释放JSON字符串
 * @param[in]  : jsonString --JSON字符串
 * @param[out] : 无
 * @return     : 无
 * @note       : 只释放本模块返回的堆内存字符串
 ******************************************************************************/
void CJSON_Free(char* jsonString);

/******************************************************************************
 * @brief      : 根据结构体字段描述序列化为JSON对象字符串
 * @param[in]  : object --结构体地址, fields --字段描述表
 * @param[out] : 无
 * @return     : 返回JSON对象字符串，失败返回NULL
 * @note       : 只生成{"key":value,...}对象片段，返回值需要CJSON_Free释放
 ******************************************************************************/
char* CJSON_SerializeStructObject(const void* object, const CJSONField_S* fields);

/******************************************************************************
 * @brief      : 根据字段描述把JSON对象反序列化到结构体
 * @param[in]  : jsonObject --JSON对象字符串, fields --字段描述表
 * @param[out] : object --结构体地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : 只支持扁平对象，如{"key1":1,"key2":"abc"}
 ******************************************************************************/
CJSONBool_E CJSON_DeserializeStructObject(const char* jsonObject, void* object, const CJSONField_S* fields);

/******************************************************************************
 * @brief      : 构建只有结果的响应JSON
 * @param[in]  : result --结果枚举
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":""}
 ******************************************************************************/
char* CJSON_BuildEmptyResponse(CJSONResult_E result);

/******************************************************************************
 * @brief      : 构建data为字符串的响应JSON
 * @param[in]  : result --结果枚举, dataString --data字符串
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":"xxxx"}
 ******************************************************************************/
char* CJSON_BuildStringResponse(CJSONResult_E result, const char* dataString);

/******************************************************************************
 * @brief      : 构建data为对象的响应JSON
 * @param[in]  : result --结果枚举, object --结构体地址, fields --字段描述表
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass/fail","data":{"key":value}}
 ******************************************************************************/
char* CJSON_BuildObjectResponse(CJSONResult_E result, const void* object, const CJSONField_S* fields);

/******************************************************************************
 * @brief      : 解析响应JSON
 * @param[in]  : jsonString --响应JSON字符串, objectSize --data对象结构体大小
 * @param[out] : response --响应信息, object --data对象结构体地址
 * @return     : CJSON_BOOL_TRUE --成功，CJSON_BOOL_FALSE --失败
 * @note       : data是对象时使用fields解析，data是字符串或空字符串时写入response->dataString
 ******************************************************************************/
CJSONBool_E CJSON_ParseResponse(const char* jsonString, CJSONResponse_S* response, void* object, size_t objectSize, const CJSONField_S* fields);

/******************************************************************************
 * @brief      : 构建日志JSON
 * @param[in]  : level --日志级别, module --模块名, message --日志信息
 * @param[out] : 无
 * @return     : 返回JSON字符串，失败返回NULL
 * @note       : 输出格式{"result":"pass","data":{"level":"INFO","module":"xxx","message":"xxx"}}
 ******************************************************************************/
char* CJSON_BuildLogResponse(CJSONLogLevel_E level, const char* module, const char* message);

/******************************************************************************
 * @brief      : 打印日志JSON
 * @param[in]  : level --日志级别, module --模块名, message --日志信息
 * @param[out] : 无
 * @return     : 无
 * @note       : 默认输出到stdout，嵌入式工程可替换printf输出接口
 ******************************************************************************/
void CJSON_Log(CJSONLogLevel_E level, const char* module, const char* message);

#ifdef __cplusplus
}
#endif

#endif

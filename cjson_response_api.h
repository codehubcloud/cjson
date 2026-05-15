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

#include <stddef.h>
#include <stdint.h>

#define CJSON_FIELD_INVALID_OFFSET ((size_t)(~(size_t)0U))

typedef enum {
    BOOL_FALSE = 0,
    BOOL_TRUE,
} CJSONBool_E;

typedef enum {
    RESULT_FAIL = 0,
    RESULT_PASS,
} CJSONResult_E;

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

typedef enum {
    FIELD_INT = 0,
    FIELD_UINT,
    FIELD_BOOL,
    FIELD_STRING,
    FIELD_DOUBLE,
    FIELD_CHAR,
} CJSONFieldType_E;

typedef struct {
    const char* jsonKey;
    CJSONFieldType_E fieldType;
    size_t fieldOffset;
    size_t fieldSize;
} CJSONField_S;

typedef struct {
    CJSONResult_E result;
    CJSONDataType_E dataType;
    char* dataString;
    void* dataObject;
    int64_t intValue;
    uint64_t uintValue;
    long double doubleValue;
    CJSONBool_E boolValue;
    char charValue;
} CJSONResponse_S;

typedef struct {
    char* dataBuf;
    size_t dataLen;
    CJSONDataType_E dataType;
    CJSONResult_E result;
    const void* data;
    const CJSONField_S* fields;
    int64_t intValue;
    uint64_t uintValue;
    long double doubleValue;
    CJSONBool_E boolValue;
    char charValue;
} CJSONBuildParam_S;

typedef struct {
    const char* jsonString;
    CJSONDataType_E dataType;
    CJSONResponse_S* response;
    void* data;
    const CJSONField_S* fields;
} CJSONParseParam_S;

typedef struct {
    const char* typeName;
    const CJSONField_S* fields;
} CJSONObjectType_S;

#define CJSON_OFFSET_OF(type, member) ((size_t)&(((type*)0)->member))

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
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
        char*: FIELD_STRING,            \
        const char*: FIELD_STRING)
#else
#error "FT_Result requires C11 _Generic support. Please enable C11."
#endif

#define CJSON_FIELD_AUTO_DESC(type, member, key)     {(key), CJSON_FIELD_TYPE_OF(((type*)0)->member), CJSON_OFFSET_OF(type, member), sizeof(((type*)0)->member)}
#define CJSON_FIELD_KEY(type, member)                CJSON_FIELD_AUTO_DESC(type, member, #member)
#define CJSON_FIELD_END()                            {NULL, FIELD_STRING, CJSON_FIELD_INVALID_OFFSET, 0U}
#define CJSON_OBJECT_AUTO_FIELD_DESC(type, member)   CJSON_FIELD_KEY(type, member),

#define CJSON_PP_EXPAND(...)                         __VA_ARGS__
#define CJSON_PP_FOREACH_1(macro, type, item1)       macro(type, item1)
#define CJSON_PP_FOREACH_2(macro, type, item1, ...)  macro(type, item1) CJSON_PP_FOREACH_1(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_3(macro, type, item1, ...)  macro(type, item1) CJSON_PP_FOREACH_2(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_4(macro, type, item1, ...)  macro(type, item1) CJSON_PP_FOREACH_3(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_5(macro, type, item1, ...)  macro(type, item1) CJSON_PP_FOREACH_4(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_6(macro, type, item1, ...)  macro(type, item1) CJSON_PP_FOREACH_5(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_7(macro, type, item1, ...)  macro(type, item1) CJSON_PP_FOREACH_6(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_8(macro, type, item1, ...)  macro(type, item1) CJSON_PP_FOREACH_7(macro, type, __VA_ARGS__)
#define CJSON_PP_FOREACH_9(macro, type, item1, ...)  macro(type, item1) CJSON_PP_FOREACH_8(macro, type, __VA_ARGS__)
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

#define FT_RESULT_CAT_INNER(left, right)          left##right
#define FT_RESULT_CAT(left, right)                FT_RESULT_CAT_INNER(left, right)
#define FT_RESULT_OBJECT_FIELDS_FUNC_NAME(type)   FT_ResultFields_##type
#define FT_RESULT_OBJECT_REGISTER_FUNC_NAME(type) FT_ResultRegister_##type
#define FT_RESULT_OBJECT_PRINT_FUNC_NAME(type)    FT_ResultPrint_##type
#define FT_RESULT_OBJECT_PARSE_FUNC_NAME(type)    FT_ResultParse_##type

#define FT_RESULT_DECLARE_OBJECT_TYPE(type)                                                                             \
    CJSONBool_E FT_RESULT_OBJECT_REGISTER_FUNC_NAME(type)(void);                                                        \
    void FT_RESULT_OBJECT_PRINT_FUNC_NAME(type)(char* outBuf, size_t outLen, CJSONResult_E result, const type* object); \
    CJSONBool_E FT_RESULT_OBJECT_PARSE_FUNC_NAME(type)(const char* jsonString, CJSONResponse_S* response, type* object)

#define FT_RESULT_DEFINE_OBJECT_TYPE(type, ...)                                                                                          \
    static const CJSONField_S* FT_RESULT_OBJECT_FIELDS_FUNC_NAME(type)(void)                                                             \
    {                                                                                                                                    \
        static const CJSONField_S cjsonFields[] = {CJSON_PP_FOREACH(CJSON_OBJECT_AUTO_FIELD_DESC, type, __VA_ARGS__) CJSON_FIELD_END()}; \
        return cjsonFields;                                                                                                              \
    }                                                                                                                                    \
    CJSONBool_E FT_RESULT_OBJECT_REGISTER_FUNC_NAME(type)(void)                                                                          \
    {                                                                                                                                    \
        return CJSON_RegisterObjectType(#type, FT_RESULT_OBJECT_FIELDS_FUNC_NAME(type)());                                               \
    }                                                                                                                                    \
    void FT_RESULT_OBJECT_PRINT_FUNC_NAME(type)(char* outBuf, size_t outLen, CJSONResult_E result, const type* object)                   \
    {                                                                                                                                    \
        const CJSONField_S* fields = CJSON_FindObjectFields(#type);                                                                      \
        CJSONBuildParam_S cjsonBuildParam = {0};                                                                                         \
        cjsonBuildParam.dataType = DATA_OBJECT;                                                                                          \
        cjsonBuildParam.dataBuf = outBuf;                                                                                                \
        cjsonBuildParam.dataLen = outLen;                                                                                                \
        cjsonBuildParam.result = result;                                                                                                 \
        cjsonBuildParam.data = (const void*)object;                                                                                      \
        cjsonBuildParam.fields = (fields != NULL) ? fields : FT_RESULT_OBJECT_FIELDS_FUNC_NAME(type)();                                  \
        CJSON_ResultBuild(&cjsonBuildParam);                                                                                             \
    }                                                                                                                                    \
    CJSONBool_E FT_RESULT_OBJECT_PARSE_FUNC_NAME(type)(const char* jsonString, CJSONResponse_S* response, type* object)                  \
    {                                                                                                                                    \
        const CJSONField_S* fields = CJSON_FindObjectFields(#type);                                                                      \
        fields = (fields != NULL) ? fields : FT_RESULT_OBJECT_FIELDS_FUNC_NAME(type)();                                                  \
        return CJSON_ResultParse(&(const CJSONParseParam_S){jsonString, DATA_OBJECT, response, (void*)object, fields});                  \
    }

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
        char*: CJSON_BuildParamSetPointer,            \
        const char*: CJSON_BuildParamSetPointer,      \
        void*: CJSON_BuildParamSetPointer,            \
        const void*: CJSON_BuildParamSetPointer,      \
        default: CJSON_BuildParamSetPointer)((buildParam), (dataArg))

#define FT_RESULT_BUILD_COMMON(dataTypeArg, outBufArg, outLenArg, resultArg, dataArg) \
    do {                                                                              \
        CJSONBuildParam_S cjsonBuildParam = {0};                                      \
        cjsonBuildParam.dataType = (dataTypeArg);                                     \
        cjsonBuildParam.dataBuf = (outBufArg);                                        \
        cjsonBuildParam.dataLen = (outLenArg);                                        \
        cjsonBuildParam.result = (resultArg);                                         \
        cjsonBuildParam.fields = NULL;                                                \
        FT_RESULT_SET_BUILD_DATA(&cjsonBuildParam, dataArg);                          \
        CJSON_ResultBuild(&cjsonBuildParam);                                          \
    } while (0)

#define FT_RESULT_PARSE_COMMON(dataTypeArg, jsonArg, responseArg, dataArg) \
    CJSON_ResultParse(&(const CJSONParseParam_S){(jsonArg), (dataTypeArg), (responseArg), (void*)(dataArg), NULL})

#define FT_ResultPrint(typeArg, outBufArg, outLenArg, resultArg, dataArg) \
    FT_RESULT_CAT(FT_ResultPrint_, typeArg)((outBufArg), (outLenArg), (resultArg), (dataArg))

#define FT_ResultParse(typeArg, jsonArg, responseArg, dataArg)               FT_RESULT_CAT(FT_ResultParse_, typeArg)((jsonArg), (responseArg), (dataArg))

#define FT_ResultPrint_DATA_EMPTY(outBufArg, outLenArg, resultArg, dataArg)  FT_RESULT_BUILD_COMMON(DATA_EMPTY, outBufArg, outLenArg, resultArg, dataArg)
#define FT_ResultPrint_DATA_STRING(outBufArg, outLenArg, resultArg, dataArg) FT_RESULT_BUILD_COMMON(DATA_STRING, outBufArg, outLenArg, resultArg, dataArg)
#define FT_ResultPrint_DATA_INT(outBufArg, outLenArg, resultArg, dataArg)    FT_RESULT_BUILD_COMMON(DATA_INT, outBufArg, outLenArg, resultArg, dataArg)
#define FT_ResultPrint_DATA_UINT(outBufArg, outLenArg, resultArg, dataArg)   FT_RESULT_BUILD_COMMON(DATA_UINT, outBufArg, outLenArg, resultArg, dataArg)
#define FT_ResultPrint_DATA_BOOL(outBufArg, outLenArg, resultArg, dataArg)   FT_RESULT_BUILD_COMMON(DATA_BOOL, outBufArg, outLenArg, resultArg, dataArg)
#define FT_ResultPrint_DATA_DOUBLE(outBufArg, outLenArg, resultArg, dataArg) FT_RESULT_BUILD_COMMON(DATA_DOUBLE, outBufArg, outLenArg, resultArg, dataArg)
#define FT_ResultPrint_DATA_CHAR(outBufArg, outLenArg, resultArg, dataArg)   FT_RESULT_BUILD_COMMON(DATA_CHAR, outBufArg, outLenArg, resultArg, dataArg)

#define FT_ResultParse_DATA_EMPTY(jsonArg, responseArg, dataArg)             FT_RESULT_PARSE_COMMON(DATA_EMPTY, jsonArg, responseArg, dataArg)
#define FT_ResultParse_DATA_STRING(jsonArg, responseArg, dataArg)            FT_RESULT_PARSE_COMMON(DATA_STRING, jsonArg, responseArg, dataArg)
#define FT_ResultParse_DATA_INT(jsonArg, responseArg, dataArg)               FT_RESULT_PARSE_COMMON(DATA_INT, jsonArg, responseArg, dataArg)
#define FT_ResultParse_DATA_UINT(jsonArg, responseArg, dataArg)              FT_RESULT_PARSE_COMMON(DATA_UINT, jsonArg, responseArg, dataArg)
#define FT_ResultParse_DATA_BOOL(jsonArg, responseArg, dataArg)              FT_RESULT_PARSE_COMMON(DATA_BOOL, jsonArg, responseArg, dataArg)
#define FT_ResultParse_DATA_DOUBLE(jsonArg, responseArg, dataArg)            FT_RESULT_PARSE_COMMON(DATA_DOUBLE, jsonArg, responseArg, dataArg)
#define FT_ResultParse_DATA_CHAR(jsonArg, responseArg, dataArg)              FT_RESULT_PARSE_COMMON(DATA_CHAR, jsonArg, responseArg, dataArg)

void CJSON_BuildParamSetInt(CJSONBuildParam_S* buildParam, int64_t value);
void CJSON_BuildParamSetUInt(CJSONBuildParam_S* buildParam, uint64_t value);
void CJSON_BuildParamSetBool(CJSONBuildParam_S* buildParam, int value);
void CJSON_BuildParamSetDouble(CJSONBuildParam_S* buildParam, long double value);
void CJSON_BuildParamSetChar(CJSONBuildParam_S* buildParam, char value);
void CJSON_BuildParamSetPointer(CJSONBuildParam_S* buildParam, const void* value);

CJSONBool_E CJSON_RegisterObjectType(const char* typeName, const CJSONField_S* fields);
const CJSONField_S* CJSON_FindObjectFields(const char* typeName);
void CJSON_ResultBuild(const CJSONBuildParam_S* buildParam);
CJSONBool_E CJSON_ResultParse(const CJSONParseParam_S* parseParam);

#ifdef __cplusplus
}
#endif

#endif

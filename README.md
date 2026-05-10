# cjson_response

`cjson_response` 是一个轻量级 C 语言 JSON 响应构造与解析模块，主要用于嵌入式测试、工厂测试、命令响应、日志输出等场景。

模块不依赖第三方 JSON 库，内部通过字符串拼接和简单解析完成以下三类响应 JSON 的构造与解析：

```json
{"result":"pass","data":{"key1":123,"key2":"value"}}
```

```json
{"result":"pass","data":"SN123456"}
```

```json
{"result":"fail","data":""}
```

---

## 1. 功能特性

* 支持构建只有结果的响应 JSON。
* 支持构建 `data` 为字符串的响应 JSON。
* 支持构建 `data` 为对象的响应 JSON。
* 支持结构体自动序列化为 JSON 对象。
* 支持 JSON 对象反序列化到结构体。
* 支持字段描述表注册结构体字段。
* 支持批量字段宏，减少手写字段描述。
* 支持日志 JSON 统一输出。
* 纯 C 实现，不依赖第三方库。
* 支持 CMake 编译。

---

## 2. 工程目录

```text
cjson_response/
├── CMakeLists.txt
├── cjson_response.h
├── cjson_response.c
└── main_response_only_example.c
```

文件说明：

| 文件                             | 说明                         |
| ------------------------------ | -------------------------- |
| `cjson_response.h`             | 对外接口声明、宏定义、枚举和结构体定义        |
| `cjson_response.c`             | JSON 构造、解析、序列化、反序列化和日志输出实现 |
| `main_response_only_example.c` | 使用示例                       |
| `CMakeLists.txt`               | CMake 编译脚本                 |

---

## 3. 编译方法

### 3.1 Linux / Ubuntu 使用 CMake 编译

```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```

运行示例程序：

```bash
./build/main_response_only_example
```

---

### 3.2 Linux / Ubuntu 使用 Ninja 编译

如果系统已经安装 Ninja，可以使用：

```bash
mkdir -p build
cmake -S . -B build -G Ninja
cmake --build build
```

运行：

```bash
./build/main_response_only_example
```

如果没有安装 Ninja：

```bash
sudo apt install ninja-build
```

---

### 3.3 Windows 使用 MinGW 编译

```bash
mkdir build
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

运行：

```bash
build\main_response_only_example.exe
```

---

### 3.4 Windows 使用 Visual Studio 编译

```powershell
mkdir build
cmake -S . -B build
cmake --build build --config Debug
```

运行：

```powershell
.\build\Debug\main_response_only_example.exe
```

---

### 3.5 不使用 CMake，直接使用 gcc 编译

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic cjson_response.c main_response_only_example.c -o main_response_only_example
```

Linux 运行：

```bash
./main_response_only_example
```

Windows MinGW 运行：

```bash
main_response_only_example.exe
```

---

## 4. 快速开始

### 4.1 定义结构体

```c
typedef struct {
    int vol;
    int cap;
} VoltageInfo_S;
```

---

### 4.2 注册结构体字段

```c
CJSON_REGISTER_OBJECT_TYPE(VoltageInfo_S, vol, cap);
```

注册后可以通过 `VoltageInfo_S` 类型直接构建对象响应。

---

### 4.3 构建只有结果的响应

```c
char *jsonString = CJSON_RESPONSE_BUILD_EMPTY(CJSON_RESULT_FAIL);

printf("%s\n", jsonString);

CJSON_Free(jsonString);
```

输出示例：

```json
{"result":"fail","data":""}
```

---

### 4.4 构建字符串响应

```c
char *jsonString = CJSON_RESPONSE_BUILD_STRING(CJSON_RESULT_PASS, "SN123456");

printf("%s\n", jsonString);

CJSON_Free(jsonString);
```

输出示例：

```json
{"result":"pass","data":"SN123456"}
```

---

### 4.5 构建对象响应

```c
VoltageInfo_S voltageInfo = {300, 1200};

char *jsonString = CJSON_RESPONSE_BUILD_OBJECT(CJSON_RESULT_PASS, &voltageInfo, VoltageInfo_S);

printf("%s\n", jsonString);

CJSON_Free(jsonString);
```

输出示例：

```json
{"result":"pass","data":{"vol":300,"cap":1200}}
```

---

## 5. 完整示例

```c
#include "cjson_response.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    int vol;
    int cap;
} VoltageInfo_S;

typedef struct {
    int num1;
    int num2;
    char version[64];
    bool flags;
} Test_S;

CJSON_REGISTER_OBJECT_TYPE(VoltageInfo_S, vol, cap);
CJSON_REGISTER_OBJECT_TYPE(Test_S, num1, num2, version, flags);

int main(void)
{
    VoltageInfo_S voltageInfo = {300, 1200};
    VoltageInfo_S parsedVoltageInfo = {0};
    Test_S testInfo = {42, 84, "v1.0.0", true};
    Test_S parsedTestInfo = {0};
    CJSONResponse_S response = {0};
    char dataString[256] = {0};
    char *emptyResponse = NULL;
    char *stringResponse = NULL;
    char *objectResponse = NULL;
    char *testResponse = NULL;

    emptyResponse = CJSON_RESPONSE_BUILD_EMPTY(CJSON_RESULT_FAIL);
    stringResponse = CJSON_RESPONSE_BUILD_STRING(CJSON_RESULT_PASS, "SN123456");
    objectResponse = CJSON_RESPONSE_BUILD_OBJECT(CJSON_RESULT_PASS, &voltageInfo, VoltageInfo_S);
    testResponse = CJSON_RESPONSE_BUILD_OBJECT(CJSON_RESULT_PASS, &testInfo, Test_S);

    printf("%s\n", emptyResponse);
    printf("%s\n", stringResponse);
    printf("%s\n", objectResponse);
    printf("%s\n", testResponse);

    response.dataString = dataString;
    if (CJSON_ParseResponse(stringResponse, &response, NULL, 0U, NULL) == CJSON_BOOL_TRUE) {
        printf("parsed result=%d data=%s\n", response.result, response.dataString);
    }

    if (CJSON_PARSE_RESPONSE_OBJECT(objectResponse, &response, &parsedVoltageInfo, sizeof(parsedVoltageInfo), VoltageInfo_S) == CJSON_BOOL_TRUE) {
        printf("parsed vol=%d cap=%d\n", parsedVoltageInfo.vol, parsedVoltageInfo.cap);
    }

    if (CJSON_PARSE_RESPONSE_OBJECT(testResponse, &response, &parsedTestInfo, sizeof(parsedTestInfo), Test_S) == CJSON_BOOL_TRUE) {
        printf("parsed num1=%d num2=%d version=%s flags=%d\n",
               parsedTestInfo.num1,
               parsedTestInfo.num2,
               parsedTestInfo.version,
               parsedTestInfo.flags);
    }

    CJSON_Log(CJSON_LOG_LEVEL_INFO, "factory_test", "read voltage success");

    CJSON_Free(emptyResponse);
    CJSON_Free(stringResponse);
    CJSON_Free(objectResponse);
    CJSON_Free(testResponse);

    return 0;
}
```

---

## 6. 运行结果示例

```text
{"result":"fail","data":""}
{"result":"pass","data":"SN123456"}
{"result":"pass","data":{"vol":300,"cap":1200}}
{"result":"pass","data":{"num1":42,"num2":84,"version":"v1.0.0","flags":true}}
parsed result=1 data=SN123456
parsed vol=300 cap=1200
parsed num1=42 num2=84 version=v1.0.0 flags=1
{"result":"pass","data":{"level":"INFO","module":"factory_test","message":"read voltage success"}}
```

---

## 7. 主要接口说明

### 7.1 释放 JSON 字符串

```c
void CJSON_Free(char *jsonString);
```

说明：

本模块构建出来的 JSON 字符串由堆内存分配，使用完必须调用 `CJSON_Free()` 释放。

---

### 7.2 构建空响应

```c
char *CJSON_BuildEmptyResponse(CJSONResult_E result);
```

宏封装：

```c
CJSON_RESPONSE_BUILD_EMPTY(result)
```

输出格式：

```json
{"result":"pass","data":""}
```

或者：

```json
{"result":"fail","data":""}
```

---

### 7.3 构建字符串响应

```c
char *CJSON_BuildStringResponse(CJSONResult_E result, const char *dataString);
```

宏封装：

```c
CJSON_RESPONSE_BUILD_STRING(result, dataString)
```

输出格式：

```json
{"result":"pass","data":"xxxx"}
```

---

### 7.4 构建对象响应

```c
char *CJSON_BuildObjectResponse(CJSONResult_E result, const void *object, const CJSONField_S *fields);
```

宏封装：

```c
CJSON_RESPONSE_BUILD_OBJECT(result, object, type)
```

输出格式：

```json
{"result":"pass","data":{"key":value}}
```

---

### 7.5 解析响应 JSON

```c
CJSONBool_E CJSON_ParseResponse(const char *jsonString,
                                CJSONResponse_S *response,
                                void *object,
                                size_t objectSize,
                                const CJSONField_S *fields);
```

说明：

* 当 `data` 是字符串时，需要提前给 `response.dataString` 设置外部缓冲区。
* 当 `data` 是对象时，需要传入对象地址、对象大小和字段描述表。

解析字符串响应：

```c
CJSONResponse_S response = {0};
char dataString[256] = {0};

response.dataString = dataString;

if (CJSON_ParseResponse(jsonString, &response, NULL, 0U, NULL) == CJSON_BOOL_TRUE) {
    printf("data=%s\n", response.dataString);
}
```

解析对象响应：

```c
VoltageInfo_S voltageInfo = {0};
CJSONResponse_S response = {0};

if (CJSON_PARSE_RESPONSE_OBJECT(jsonString, &response, &voltageInfo, sizeof(voltageInfo), VoltageInfo_S) == CJSON_BOOL_TRUE) {
    printf("vol=%d cap=%d\n", voltageInfo.vol, voltageInfo.cap);
}
```

---

### 7.6 构建日志响应

```c
char *CJSON_BuildLogResponse(CJSONLogLevel_E level, const char *module, const char *message);
```

示例：

```c
char *logJson = CJSON_BuildLogResponse(CJSON_LOG_LEVEL_INFO, "factory_test", "read voltage success");
printf("%s\n", logJson);
CJSON_Free(logJson);
```

输出示例：

```json
{"result":"pass","data":{"level":"INFO","module":"factory_test","message":"read voltage success"}}
```

---

### 7.7 直接打印日志

```c
void CJSON_Log(CJSONLogLevel_E level, const char *module, const char *message);
```

示例：

```c
CJSON_Log(CJSON_LOG_LEVEL_INFO, "factory_test", "read voltage success");
```

输出示例：

```json
{"result":"pass","data":{"level":"INFO","module":"factory_test","message":"read voltage success"}}
```

---

## 8. 字段类型说明

| 字段类型                 | 说明    |
| -------------------- | ----- |
| `CJSON_FIELD_INT`    | 有符号整数 |
| `CJSON_FIELD_UINT`   | 无符号整数 |
| `CJSON_FIELD_DOUBLE` | 浮点数   |
| `CJSON_FIELD_BOOL`   | 布尔值   |
| `CJSON_FIELD_STRING` | 字符串   |

---

## 9. 结果类型说明

```c
typedef enum {
    CJSON_RESULT_FAIL = 0,
    CJSON_RESULT_PASS,
} CJSONResult_E;
```

对应 JSON 字符串：

| 枚举                  | JSON值    |
| ------------------- | -------- |
| `CJSON_RESULT_PASS` | `"pass"` |
| `CJSON_RESULT_FAIL` | `"fail"` |

---

## 10. data 类型说明

```c
typedef enum {
    CJSON_DATA_EMPTY = 0,
    CJSON_DATA_STRING,
    CJSON_DATA_OBJECT,
} CJSONDataType_E;
```

| 类型                  | 说明     | 示例                   |
| ------------------- | ------ | -------------------- |
| `CJSON_DATA_EMPTY`  | 空字符串   | `"data":""`          |
| `CJSON_DATA_STRING` | 普通字符串  | `"data":"SN123456"`  |
| `CJSON_DATA_OBJECT` | JSON对象 | `"data":{"vol":300}` |

---

## 11. 使用注意事项

1. 本模块返回的 JSON 字符串均为堆内存，需要调用 `CJSON_Free()` 释放。
2. 当前解析器只适用于本模块生成的简单 JSON 响应。
3. 结构体对象解析只支持扁平对象，不支持嵌套对象和数组。
4. 字符串字段建议使用固定长度字符数组，例如 `char version[64]`。
5. 字符串响应解析时，`response.dataString` 必须提前指向有效缓冲区。
6. `CJSON_PARSE_RESPONSE_OBJECT()` 需要先注册对应结构体字段。
7. 如果字段描述和结构体成员类型不匹配，可能导致解析失败或数据异常。
8. 嵌入式环境中可以替换 `printf()`，实现自定义日志输出。

---

## 12. 常见问题

### 12.1 编译时提示 `CJSON_FIELD_BOOL_DESC` 未定义

需要在 `cjson_response.h` 中补充布尔字段描述宏：

```c
#define CJSON_FIELD_BOOL_DESC(type, member, key) \
    {(key), CJSON_FIELD_BOOL, CJSON_OFFSET_OF(type, member), sizeof(((type *)0)->member)}
```

---

### 12.2 include guard 是否需要修改？

建议把头文件保护宏改为和文件名一致：

```c
#ifndef CJSON_RESPONSE_H_
#define CJSON_RESPONSE_H_
```

文件最后保持：

```c
#endif
```

---

### 12.3 为什么解析字符串响应前要设置 `response.dataString`？

因为 `CJSONResponse_S` 中的 `dataString` 是指针，模块不会自动分配字符串缓冲区。

正确写法：

```c
CJSONResponse_S response = {0};
char dataString[256] = {0};

response.dataString = dataString;

CJSON_ParseResponse(jsonString, &response, NULL, 0U, NULL);
```

---

### 12.4 为什么对象响应需要注册字段？

C 语言没有运行时反射能力，模块无法自动知道结构体有哪些成员、成员偏移和成员类型。

因此需要通过字段描述表告诉模块如何访问结构体成员：

```c
CJSON_REGISTER_OBJECT_TYPE(VoltageInfo_S, vol, cap);
```

---

## 13. 适用场景

* 工厂测试命令返回结果。
* SSD、MCU、设备端接口响应。
* 简单 RPC 响应。
* 串口命令响应。
* CLI 工具输出 JSON。
* 测试日志统一输出。
* 不方便引入第三方 JSON 库的嵌入式工程。

---

## 14. 不适用场景

本模块不是完整 JSON 解析库，不适合以下场景：

* 复杂嵌套 JSON。
* JSON 数组解析。
* 动态字段解析。
* 大型 JSON 文档解析。
* 严格 JSON 标准兼容场景。
* 需要完整 Unicode `\uXXXX` 解析的场景。

如果需要完整 JSON 功能，建议使用成熟 JSON 库，例如 cJSON、Jansson 或 yyjson。

---

## 15. CMake 配置说明

当前 `CMakeLists.txt` 会生成一个静态库和一个示例程序：

```cmake
add_library(cjson_response STATIC
    cjson_response.c
)

add_executable(main_response_only_example
    main_response_only_example.c
)

target_link_libraries(main_response_only_example
    PRIVATE
        cjson_response
)
```

编译后会生成：

```text
build/
├── libcjson_response.a
└── main_response_only_example
```

Windows 下可能生成：

```text
build/
├── cjson_response.lib
└── main_response_only_example.exe
```

---

## 16. 版本信息

| 项目   | 内容             |
| ---- | -------------- |
| 模块名  | cjson_response |
| 语言   | C              |
| C标准  | C11            |
| 当前版本 | V1.0           |
| 日期   | 2026/05/09     |

---

## 17. License

当前未指定开源协议。若用于开源项目，建议补充 `LICENSE` 文件。

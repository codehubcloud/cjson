/******************************************************************************
 * File Name : ft_result_test_voltage.h
 * Function  : Voltage相关测试类型和测试接口声明。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/15
 ******************************************************************************/

#ifndef FT_RESULT_TEST_VOLTAGE_H
#define FT_RESULT_TEST_VOLTAGE_H

#include "cjson_response_api.h"

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
    int vol;
    int cap;
    u8 channel;
    u16 temperature;
    u32 count;
    u64 timestamp;
    char version[64];
    bool flag;
    double ratio;
    char grade;
} VoltageInfo_S;

typedef struct {
    char objectResponse[1024];
} VoltageResponseBuffer_S;

typedef struct {
    VoltageInfo_S voltageInfo;
    VoltageInfo_S parsedVoltageInfo;
} VoltageTestData_S;

FT_RESULT_DECLARE_OBJECT_TYPE(VoltageInfo_S);

/******************************************************************************
 * @brief      : 注册Voltage测试对象类型
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 测试程序启动时调用一次
 ******************************************************************************/
CJSONBool_E FT_ResultRegisterVoltageObjects(void);

/******************************************************************************
 * @brief      : 初始化Voltage测试数据
 * @param[in]  : 无
 * @param[out] : voltageData --测试数据
 * @return     : 无
 * @note       : 初始化Voltage测试对象
 ******************************************************************************/
void FT_ResultInitVoltageData(VoltageTestData_S* voltageData);

/******************************************************************************
 * @brief      : 构建Voltage响应
 * @param[in]  : voltageData --测试数据
 * @param[out] : responseBuf --响应缓冲区
 * @return     : 无
 * @note       : 构建Voltage对象响应
 ******************************************************************************/
void FT_ResultBuildVoltageResponses(const VoltageTestData_S* voltageData, VoltageResponseBuffer_S* responseBuf);

/******************************************************************************
 * @brief      : 打印Voltage构建结果
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : 无
 * @return     : 无
 * @note       : 打印Voltage构建出的JSON
 ******************************************************************************/
void FT_ResultPrintVoltageBuildResult(const VoltageResponseBuffer_S* responseBuf);

/******************************************************************************
 * @brief      : 解析Voltage响应
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : voltageData --测试数据
 * @return     : 无
 * @note       : 解析Voltage对象
 ******************************************************************************/
void FT_ResultParseVoltageResponses(const VoltageResponseBuffer_S* responseBuf, VoltageTestData_S* voltageData);

/******************************************************************************
 * @brief      : 运行Voltage测试
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 0 --成功，非0 --失败
 * @note       : 完成注册、构建、打印、解析
 ******************************************************************************/
int FT_ResultRunVoltageTests(void);

#endif

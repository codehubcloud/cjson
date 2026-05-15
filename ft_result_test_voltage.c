/******************************************************************************
 * File Name : ft_result_test_voltage.c
 * Function  : Voltage相关测试代码实现。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/15
 ******************************************************************************/

#include "ft_result_test_voltage.h"

#include <stdio.h>

FT_RESULT_DEFINE_OBJECT_TYPE(VoltageInfo_S, vol, cap, channel, temperature, count, timestamp, version, flag, ratio, grade)

/******************************************************************************
 * @brief      : 注册Voltage测试对象类型
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 测试程序启动时调用一次
 ******************************************************************************/
CJSONBool_E FT_ResultRegisterVoltageObjects(void)
{
    if (FT_ResultRegister_VoltageInfo_S() != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 初始化Voltage测试数据
 * @param[in]  : 无
 * @param[out] : voltageData --测试数据
 * @return     : 无
 * @note       : 初始化Voltage测试对象
 ******************************************************************************/
void FT_ResultInitVoltageData(VoltageTestData_S* voltageData)
{
    if (voltageData == NULL) {
        return;
    }

    *voltageData = (VoltageTestData_S){0};
    voltageData->voltageInfo = (VoltageInfo_S){300, 1200, 2U, 35U, 99U, 123456789ULL, "v1.0.0", true, 0.75, 'A'};
}

/******************************************************************************
 * @brief      : 构建Voltage响应
 * @param[in]  : voltageData --测试数据
 * @param[out] : responseBuf --响应缓冲区
 * @return     : 无
 * @note       : 构建Voltage对象响应
 ******************************************************************************/
void FT_ResultBuildVoltageResponses(const VoltageTestData_S* voltageData, VoltageResponseBuffer_S* responseBuf)
{
    if (voltageData == NULL || responseBuf == NULL) {
        return;
    }

    FT_ResultPrint(VoltageInfo_S, responseBuf->objectResponse, sizeof(responseBuf->objectResponse), RESULT_PASS, &voltageData->voltageInfo);
}

/******************************************************************************
 * @brief      : 打印Voltage构建结果
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : 无
 * @return     : 无
 * @note       : 打印Voltage构建出的JSON
 ******************************************************************************/
void FT_ResultPrintVoltageBuildResult(const VoltageResponseBuffer_S* responseBuf)
{
    if (responseBuf == NULL) {
        return;
    }

    printf("========== build voltage object ==========\n");
    printf("%-28s: %s\n", "build voltage response", responseBuf->objectResponse);
}

/******************************************************************************
 * @brief      : 解析Voltage响应
 * @param[in]  : responseBuf --响应缓冲区
 * @param[out] : voltageData --测试数据
 * @return     : 无
 * @note       : 解析Voltage对象
 ******************************************************************************/
void FT_ResultParseVoltageResponses(const VoltageResponseBuffer_S* responseBuf, VoltageTestData_S* voltageData)
{
    CJSONResponse_S response = {0};

    if (responseBuf == NULL || voltageData == NULL) {
        return;
    }

    printf("\n========== parse voltage object ==========\n");

    if (FT_ResultParse(VoltageInfo_S, responseBuf->objectResponse, &response, &voltageData->parsedVoltageInfo) == BOOL_TRUE) {
        printf("%-28s: result=%d dataType=%d vol=%d cap=%d channel=%u temperature=%u "
               "count=%u timestamp=%llu version=%s flag=%d ratio=%Lf grade=%c\n",
               "parse voltage response", response.result, response.dataType, voltageData->parsedVoltageInfo.vol, voltageData->parsedVoltageInfo.cap,
               voltageData->parsedVoltageInfo.channel, voltageData->parsedVoltageInfo.temperature, voltageData->parsedVoltageInfo.count,
               (unsigned long long)voltageData->parsedVoltageInfo.timestamp, voltageData->parsedVoltageInfo.version, voltageData->parsedVoltageInfo.flag,
               (long double)voltageData->parsedVoltageInfo.ratio, voltageData->parsedVoltageInfo.grade);
    }
}

/******************************************************************************
 * @brief      : 运行Voltage测试
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 0 --成功，非0 --失败
 * @note       : 完成注册、构建、打印、解析
 ******************************************************************************/
int FT_ResultRunVoltageTests(void)
{
    VoltageTestData_S voltageData = {0};
    VoltageResponseBuffer_S responseBuf = {0};

    if (FT_ResultRegisterVoltageObjects() != BOOL_TRUE) {
        printf("register voltage object failed\n");
        return -1;
    }

    FT_ResultInitVoltageData(&voltageData);
    FT_ResultBuildVoltageResponses(&voltageData, &responseBuf);
    FT_ResultPrintVoltageBuildResult(&responseBuf);
    FT_ResultParseVoltageResponses(&responseBuf, &voltageData);

    return 0;
}

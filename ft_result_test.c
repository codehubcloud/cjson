/******************************************************************************
 * File Name : ft_result_test.c
 * Function  : 响应JSON测试代码实现。
 * Author    : xxx
 * Version   : V1.0
 * Date      : 2026/05/09
 ******************************************************************************/

#include "ft_result_test.h"
#include "ft_result_test_basictype.h"
#include "ft_result_test_voltage.h"

#include <stdio.h>

/******************************************************************************
 * @brief      : 注册测试对象类型
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : BOOL_TRUE --成功，BOOL_FALSE --失败
 * @note       : 测试程序启动时调用一次
 ******************************************************************************/
CJSONBool_E FT_ResultRegisterTestObjects(void)
{
    if (FT_ResultRegisterVoltageObjects() != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    if (FT_ResultRegisterBasicTypeObjects() != BOOL_TRUE) {
        return BOOL_FALSE;
    }

    return BOOL_TRUE;
}

/******************************************************************************
 * @brief      : 运行全部测试
 * @param[in]  : 无
 * @param[out] : 无
 * @return     : 0 --成功，非0 --失败
 * @note       : 完成注册、构建、打印、解析和结果打印
 ******************************************************************************/
int FT_ResultRunAllTests(void)
{
    if (FT_ResultRegisterTestObjects() != BOOL_TRUE) {
        printf("register object failed\n");
        return -1;
    }

    // Run Voltage tests
    if (FT_ResultRunVoltageTests() != 0) {
        printf("voltage tests failed\n");
        return -1;
    }

    // Run BasicType tests
    if (FT_ResultRunBasicTypeTests() != 0) {
        printf("basictype tests failed\n");
        return -1;
    }

    return 0;
}

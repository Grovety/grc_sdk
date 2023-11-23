#ifndef _GRC_LL_PROTOCOL_COMMANDS_H_
#define _GRC_LL_PROTOCOL_COMMANDS_H_

#include <stdint.h>
#include "grc/i2c/protocol_structures.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*!
 * \brief init protocol
 */
int initProtocolCommands(void* ll_dev);

/*!
 * \brief get the function currently running on the device
 */
int getCurFunction(void* ll_dev);

/*!
 * \brief send function argument of different types
 */
int sendIntArguments(void* ll_dev, int arg);
int sendFloatArguments(void* ll_dev, float arg);
int sendFloatArrayArguments(void* ll_dev, unsigned len, const float* vals, uint8_t* blockCnt);
int sendParamArguments(void* ll_dev, struct Param* arg);

/*!
 * \brief get status of send arguments
 */
int getStreamResult(void* ll_dev, uint8_t* status);

/*!
 * \brief call remote function
 */
int callFunction(void* ll_dev, uint8_t functionCmd);

/*!
 * \brief get function executing status and retcode
 */
int getFunctionStatus(void* ll_dev, uint8_t functionCmd, struct FunctionExecutionStatus* status);

/*!
 * \brief get function result values
 */
int getFunctionResult(void* ll_dev, uint8_t functionCmd, int* result);

int getCurGRCVersion(void* ll_dev);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GRC_LL_PROTOCOL_COMMANDS_H_

#ifndef _GRC_LL_PROTOCOL_COMMANDS_H_
#define _GRC_LL_PROTOCOL_COMMANDS_H_

#include <stdint.h>
#include "grc/drivers/grc_ll_i2c.h"
#include "grc/protocol_layer/protocol_structures.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*!
 * \brief init protocol
 */
int initProtocolCommands(struct grc_ll_i2c_dev* grc);

/*!
 * \brief get the function currently running on the device
 */
int getCurFunction(struct grc_ll_i2c_dev* grc);

/*!
 * \brief send function argument of different types
 */
int sendIntArguments(struct grc_ll_i2c_dev* grc, int arg);
int sendFloatArguments(struct grc_ll_i2c_dev* grc, float arg);
int sendFloatArrayArguments(struct grc_ll_i2c_dev* grc, unsigned len, const float* vals, uint8_t* blockCnt);
int sendParamArguments(struct grc_ll_i2c_dev* grc, struct Param* arg);

/*!
 * \brief get status of send arguments
 */
int getStreamResult(struct grc_ll_i2c_dev* grc, uint8_t* status);

/*!
 * \brief call remote function
 */
int callFunction(struct grc_ll_i2c_dev* grc, uint8_t functionCmd);

/*!
 * \brief get function executing status and retcode
 */
int getFunctionStatus(struct grc_ll_i2c_dev* grc, uint8_t functionCmd, struct FunctionExecutionStatus* status);

/*!
 * \brief get function result values
 */
int getFunctionResult(struct grc_ll_i2c_dev* grc, uint8_t functionCmd, int* result);

int getCurGRCVersion(struct grc_ll_i2c_dev* grc);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GRC_LL_PROTOCOL_COMMANDS_H_

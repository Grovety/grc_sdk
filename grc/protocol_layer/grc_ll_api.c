#include <stdio.h>
#include "grc/grc_error_codes.h"
#include "grc/protocol_layer/grc_ll_api.h"
#include "grc/protocol_layer/grc_ll_protocol_commands.h"

#define FUNCTION_START_TRAINING_CMD 0x07
#define FUNCTION_STOP_TRAINING_CMD 0x08
#define FUNCTION_START_INFERENCE_CMD 0x09
#define FUNCTION_STOP_INFERENCE_CMD 0x0a
#define FUNCTION_FEED_DATA_FLOAT_CMD 0x0b
#define FUNCTION_FEED_DATA_FLOAT_ARRAY_CMD 0x0c
#define FUNCTION_GET_STATUS_CMD 0x0d
#define FUNCTION_CLEAR_CMD 0x0e
#define FUNCTION_SET_NEEDED_PARAMS_CMD 0x0f

#define FUNCTION_MIN FUNCTION_START_TRAINING_CMD
#define FUNCTION_MAX FUNCTION_SET_NEEDED_PARAMS_CMD

#define STATUS_BYTE_CNT 32

static uint8_t streamingResult[STATUS_BYTE_CNT];

#define CHECK_TRANSPORT_RESULT(func, res) \
    res = func;                           \
    if (res < 0) {                        \
        return res;                       \
    }

int __isExecutingAllowed(struct grc_ll_i2c_dev* grc)
{
    int curFunction;
    CHECK_TRANSPORT_RESULT(getCurFunction(grc), curFunction)
    if (((curFunction > 0) && (curFunction < FUNCTION_MIN)) || (curFunction > FUNCTION_MAX)) {
        return WRONG_GRC_ANSWER;
    }
    if (curFunction > 0) {
        return GRC_IS_BUSY;
    }

    return I2C_OK;
}

int __checkSingleStatus(const uint8_t* status)
{
    uint8_t packetStatus = status[31] & 1;
    return packetStatus == 1 ? I2C_OK : DATA_NOT_DELIVERED;
}

int __checkFloatArrayStatus(const uint8_t* status, uint8_t blockCnt)
{

    uint8_t packageNumber = 0;
    for (uint8_t j = 0; j < STATUS_BYTE_CNT; j++) {
        for (uint8_t i = 0; i < 8; i++) {

            if (blockCnt <= packageNumber++) {
                return I2C_OK;
            }
            // if block was not delivered
            if (((status[STATUS_BYTE_CNT - j - 1] & 1 << i) >> i) == 0) {
                return DATA_NOT_DELIVERED;
            }
        }
    }
    return I2C_OK;
}

int __callStartTrainingFunction(struct grc_ll_i2c_dev* grc, int category)
{
    int res;
    CHECK_TRANSPORT_RESULT(__isExecutingAllowed(grc), res)
    CHECK_TRANSPORT_RESULT(sendIntArguments(grc, category), res)
    CHECK_TRANSPORT_RESULT(getStreamResult(grc, streamingResult), res)
    // It was assumed that if some packages did not reach, they could be forwarded. but so far this possibility has not been realized
    CHECK_TRANSPORT_RESULT(__checkSingleStatus(streamingResult), res)
    return callFunction(grc, FUNCTION_START_TRAINING_CMD);
}

int __callFeedDataSingleFunction(struct grc_ll_i2c_dev* grc, float arg)
{
    int res;
    CHECK_TRANSPORT_RESULT(__isExecutingAllowed(grc), res)
    CHECK_TRANSPORT_RESULT(sendFloatArguments(grc, arg), res)
    CHECK_TRANSPORT_RESULT(getStreamResult(grc, streamingResult), res)
    CHECK_TRANSPORT_RESULT(__checkSingleStatus(streamingResult), res)
    return callFunction(grc, FUNCTION_FEED_DATA_FLOAT_CMD);
}

int __callFeedDataFunction(struct grc_ll_i2c_dev* grc, unsigned len, const float* vals)
{
    int res;
    CHECK_TRANSPORT_RESULT(__isExecutingAllowed(grc), res)

    uint8_t blockCnt = 0;
    CHECK_TRANSPORT_RESULT(sendFloatArrayArguments(grc, len, vals, &blockCnt), res)
    CHECK_TRANSPORT_RESULT(getStreamResult(grc, streamingResult), res)
    CHECK_TRANSPORT_RESULT(__checkFloatArrayStatus(streamingResult, blockCnt), res)
    return callFunction(grc, FUNCTION_FEED_DATA_FLOAT_ARRAY_CMD);
}

int __callFunctionWithoutArguments(struct grc_ll_i2c_dev* grc, uint8_t functionCmd)
{
    int res;
    CHECK_TRANSPORT_RESULT(__isExecutingAllowed(grc), res)
    return callFunction(grc, functionCmd);
}

int __callSetNeededParamsFunction(struct grc_ll_i2c_dev* grc, struct Param* param)
{
    int res;
    CHECK_TRANSPORT_RESULT(__isExecutingAllowed(grc), res)
    CHECK_TRANSPORT_RESULT(sendParamArguments(grc, param), res)
    CHECK_TRANSPORT_RESULT(getStreamResult(grc, streamingResult), res)
    CHECK_TRANSPORT_RESULT(__checkSingleStatus(streamingResult), res)
    return callFunction(grc, FUNCTION_SET_NEEDED_PARAMS_CMD);
}

int __waitResultActive(struct grc_ll_i2c_dev* grc, uint8_t functionCmd, Retcode* retcode)
{
    struct FunctionExecutionStatus status;
    *retcode = NotCalled;
    while (1) {
        int res;
        CHECK_TRANSPORT_RESULT(getFunctionStatus(grc, functionCmd, &status), res)

        if (status.isRunning || status.isCalled) {
            grc_ll_sleep(2);
        } else {
            *retcode = status.retcode;
            break;
        }
    }
    return I2C_OK;
}

int initProtocolLayer(struct grc_ll_i2c_dev* grc)
{
    int res = grc_ll_i2c_init(grc);
    if (res < 0) {
        return res;
    }
    res = getCurGRCVersion(grc);
    return res;
}

int setNeededParameters(struct grc_ll_i2c_dev* grc, struct Param* param, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callSetNeededParamsFunction(grc, param), res)
    return __waitResultActive(grc, FUNCTION_SET_NEEDED_PARAMS_CMD, retcode);
}

int startTraining(struct grc_ll_i2c_dev* grc, int category, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callStartTrainingFunction(grc, category), res)
    return __waitResultActive(grc, FUNCTION_START_TRAINING_CMD, retcode);
}

int stopTraining(struct grc_ll_i2c_dev* grc, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callFunctionWithoutArguments(grc, FUNCTION_STOP_TRAINING_CMD), res)
    return __waitResultActive(grc, FUNCTION_STOP_TRAINING_CMD, retcode);
}

int startInference(struct grc_ll_i2c_dev* grc, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callFunctionWithoutArguments(grc, FUNCTION_START_INFERENCE_CMD), res)
    return __waitResultActive(grc, FUNCTION_START_INFERENCE_CMD, retcode);
}

int stopInference(struct grc_ll_i2c_dev* grc, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callFunctionWithoutArguments(grc, FUNCTION_STOP_INFERENCE_CMD), res)
    return __waitResultActive(grc, FUNCTION_STOP_INFERENCE_CMD, retcode);
}

int feedDataSingle(struct grc_ll_i2c_dev* grc, float val, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callFeedDataSingleFunction(grc, val), res)
    return __waitResultActive(grc, FUNCTION_FEED_DATA_FLOAT_CMD, retcode);
}

int feedData(struct grc_ll_i2c_dev* grc, unsigned len, const float* vals, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callFeedDataFunction(grc, len, vals), res)
    return __waitResultActive(grc, FUNCTION_FEED_DATA_FLOAT_ARRAY_CMD, retcode);
}

int getStatus(struct grc_ll_i2c_dev* grc, int* pstat, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callFunctionWithoutArguments(grc, FUNCTION_GET_STATUS_CMD), res)
    CHECK_TRANSPORT_RESULT(__waitResultActive(grc, FUNCTION_GET_STATUS_CMD, retcode), res)
    return getFunctionResult(grc, FUNCTION_GET_STATUS_CMD, pstat);
}

int clear(struct grc_ll_i2c_dev* grc, Retcode* retcode)
{
    *retcode = NotCalled;
    int res;
    CHECK_TRANSPORT_RESULT(__callFunctionWithoutArguments(grc, FUNCTION_CLEAR_CMD), res)
    return __waitResultActive(grc, FUNCTION_CLEAR_CMD, retcode);
}

int releaseProtocolLayer(struct grc_ll_i2c_dev* grc)
{
    return grc_ll_i2c_release(grc);
}

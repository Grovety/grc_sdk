#include <math.h>
#include <stdio.h>

#include "grc/grc_error_codes.h"
#include "grc/protocol_layer/crc_calculation.h"
#include "grc/protocol_layer/grc_ll_protocol_commands.h"


#define BUFFER_SIZE 256
#define SIMPLE_COMMAND_RESULT_SIZE 1
#define STREAMING_RESULT_SIZE 32
#define ACTIVATE_STREAMING_COMMAND_SIZE 3

#define PACKAGE_HEADER_BYTE 3
#define MAX_VALUE_CNT_FOR_PACKAGE 62

#define NO_COMMAND 0x00

#define GET_CUR_FUNCTION_CMD 0x01
#define ACTIVATE_STREAMING_CMD 0x02
#define GET_STREAMING_RESULT_CMD 0x03
#define CALL_FUNCTION_CMD 0x04
#define GET_FUNCTION_STATUS_CMD 0x05
#define GET_FUNCTION_RESULT_CMD 0x06
#define GET_SDK_VERSION_CMD 0x07

static uint8_t inBuff[BUFFER_SIZE];
static uint8_t outBuff[BUFFER_SIZE];
static uint16_t outBuffLen = 0;

#define IS_LITTLE_ENDIAN 1
#define INT_SIZE 4
#define FLOAT_SIZE 4

// =============== PUT SIMPLE VALUES =====================
void __putValue(uint32_t value)
{
    uint32_t intTemplate = 0xff;
    for (uint8_t i = 0; i < INT_SIZE; i++) {
        uint8_t idx = IS_LITTLE_ENDIAN == 1 ? i : INT_SIZE - i - 1;
        outBuff[outBuffLen + idx] = (value & (intTemplate << (i * 8))) >> (i * 8);
    }
    outBuffLen = outBuffLen + INT_SIZE;
}
void __putByte(uint8_t value)
{
    outBuff[outBuffLen++] = value;
}

void __putInt(int value)
{
    uint32_t val = *(uint32_t*)&value;
    __putValue(val);
}

void __putFloat(float value)
{
    uint32_t val = *(uint32_t*)((void*)&value);
    __putValue(val);
}

uint32_t getValue(const uint8_t* source)
{
    uint32_t val = 0;
    for (uint8_t i = 0; i < INT_SIZE; i++) {
        uint8_t idx = IS_LITTLE_ENDIAN ? i : INT_SIZE - i - 1;
        val = (source[idx] << 8 * i) ^ val;
    }
    return val;
}

int getInt(uint8_t* source)
{
    uint32_t val = getValue(source);
    int result = *((int*)&val);
    return result;
}

// ============== PUT COMMANDS ===========================

// fills the buffer to send a simple command
// simple command do not need to pass an array of arguments
// (all except ACTIVATE_STREAMING_CMD)
// cmd - command code
// func -  function code, required for CALL_FUNCTION_CMD, GET_FUNCTION_STATUS_CMD, GET_FUNCTION_RESULT_CMD. in other case should be 0
int __putSimpleCommand(uint8_t cmd, uint8_t func)
{
    outBuffLen = 0;
    outBuff[outBuffLen++] = cmd;
    if ((cmd == CALL_FUNCTION_CMD) || (cmd == GET_FUNCTION_STATUS_CMD) || (cmd == GET_FUNCTION_RESULT_CMD)) {
        if (func > 0) {
            outBuff[outBuffLen++] = func;
        } else {
            return ARGUMENT_ERROR;
        }
    } else {
        if (func > 0) {
            return ARGUMENT_ERROR;
        }
    }
    return I2C_OK;
}

void __putActivateStreamingCommand(uint8_t blockSize, uint8_t blockCnt)
{
    outBuffLen = 0;
    outBuff[outBuffLen++] = ACTIVATE_STREAMING_CMD; // streaming activatiin command
    outBuff[outBuffLen++] = blockSize; // including: 0xff 0xfe <data bytes> crc8.
    outBuff[outBuffLen++] = blockCnt;
}

int __putIntAsBlock(int arg)
{
    if (outBuffLen + 8 < 256) {
        outBuff[outBuffLen++] = 0xff;
        outBuff[outBuffLen++] = 0xfe;
        uint8_t dataStart = outBuffLen;
        outBuff[outBuffLen++] = 1;
        __putInt(arg);
        outBuff[outBuffLen++] = Crc8(&outBuff[dataStart], INT_SIZE + 1);
        return I2C_OK;
    }
    return ARGUMENT_ERROR;
}

int __putParamAsBlock(struct Param* arg)
{
    if (outBuffLen + INT_SIZE + 4 < 256) {
        outBuff[outBuffLen++] = 0xff;
        outBuff[outBuffLen++] = 0xfe;
        uint8_t dataStart = outBuffLen;
        outBuff[outBuffLen++] = 1;
        __putByte(arg->kind);
        __putInt(arg->ival);
        outBuff[outBuffLen++] = Crc8(&outBuff[dataStart], INT_SIZE + 1 + 1);
        return I2C_OK;
    }
    return ARGUMENT_ERROR;
}

int __putFloatAsBlock(float arg)
{
    if (outBuffLen + 8 < 256) {
        outBuff[outBuffLen++] = 0xff;
        outBuff[outBuffLen++] = 0xfe;
        uint8_t dataStart = outBuffLen;
        outBuff[outBuffLen++] = 1;
        __putFloat(arg);
        outBuff[outBuffLen++] = Crc8(&outBuff[dataStart], INT_SIZE + 1);
        return I2C_OK;
    }
    return ARGUMENT_ERROR;
}

int __putFloatArrayAsBlock(unsigned len, const float* vals, uint8_t blockNumber, uint8_t blockSize)
{
    if (outBuffLen + blockSize >= 256) {
        return ARGUMENT_ERROR;
    }
    uint8_t valuesSavedInBlock = 0;
    uint32_t totalValuesSaved = 0; // how many values from the array were added to the previous blocks

    uint8_t valuesInBlock = (blockSize - 4) / FLOAT_SIZE;
    outBuff[outBuffLen++] = 0xff;
    outBuff[outBuffLen++] = 0xfe;
    uint8_t dataStart = outBuffLen;
    outBuff[outBuffLen++] = blockNumber;
    if (blockNumber == 1) {
        __putInt(len);
        valuesSavedInBlock++;
    } else {
        totalValuesSaved = (blockNumber - 1) * valuesInBlock - 1;
    }
    // while the block is not filled and there are not added values in the array
    while ((valuesSavedInBlock < valuesInBlock) && (totalValuesSaved < len)) {
        __putFloat(vals[totalValuesSaved++]);
        valuesSavedInBlock++;
    }
    // fill remaining with zeros
    while (valuesSavedInBlock < valuesInBlock) {
        __putFloat(0);
        valuesSavedInBlock++;
    }
    outBuff[outBuffLen++] = Crc8(&outBuff[dataStart], FLOAT_SIZE * valuesSavedInBlock + 1);
    return I2C_OK;
}

void __resetBuffer()
{
    outBuffLen = 0;
}
// =============== COMMUNICATION HELPERS ===========================
int __writeSimpleCommand(struct grc_ll_i2c_dev* grc, uint8_t cmd, uint8_t func)
{
    int res = __putSimpleCommand(cmd, func);
    if (res < 0) {
        return res;
    }
    res = grc_ll_i2c_write(grc, outBuff, outBuffLen);
    if (res < 0) {
        return res;
    }
    return I2C_OK;
}

// =============== INTERFACE ===========================
int initProtocolCommands(struct grc_ll_i2c_dev* grc)
{
    return grc_ll_i2c_init(grc);
}

/* cur executing command if > 0,
0 -  no command executing,
error < 0
*/
int getCurFunction(struct grc_ll_i2c_dev* grc)
{
    int res = __writeSimpleCommand(grc, GET_CUR_FUNCTION_CMD, 0);
    if (res < 0) {
        return res;
    }
    // TODO for some reason sometimes takes more than 2ms (2098 microseconds), although on average 50 microseconds
    // TODO to deal with the delay,
    // TODO to make a repeat request if got garbage
    grc_ll_sleep(10);
    res = grc_ll_i2c_read(grc, inBuff, SIMPLE_COMMAND_RESULT_SIZE);
    if (res < 0) {
        return res;
    }
    return inBuff[0];
}

int sendIntArguments(struct grc_ll_i2c_dev* grc, int arg)
{
    uint8_t blockCnt = 1;
    uint8_t blockSize = 8; // 4 for int arg and 4 for protocol wrap
    __putActivateStreamingCommand(blockSize, blockCnt);
    int res = __putIntAsBlock(arg);
    if (res < 0) {
        return res;
    }
    return grc_ll_i2c_write(grc, outBuff, outBuffLen);
}

int sendFloatArguments(struct grc_ll_i2c_dev* grc, float arg)
{
    uint8_t blockCnt = 1;
    uint8_t blockSize = 8; // 4 for int arg and 4 for protocol wrap
    __putActivateStreamingCommand(blockSize, blockCnt);
    int res = __putFloatAsBlock(arg);
    if (res < 0) {
        return res;
    }
    return grc_ll_i2c_write(grc, outBuff, outBuffLen);
}

int sendFloatArrayArguments(struct grc_ll_i2c_dev* grc, unsigned len, const float* vals, uint8_t* blockCnt)
{
    *blockCnt = 252;
    uint8_t blockSize = 255;
    float b = (ceil((len + 1) / (float)MAX_VALUE_CNT_FOR_PACKAGE));
    if (b <= 255) // max allowed package count 255
    {
        *blockCnt = (uint8_t)(b);
        blockSize = (uint8_t)(ceil((float)(len + 1) / (float)(*blockCnt))) * FLOAT_SIZE + 4;
    }
    __putActivateStreamingCommand(blockSize, *blockCnt);
    if ((BUFFER_SIZE - blockSize) < ACTIVATE_STREAMING_COMMAND_SIZE) {
        int res = grc_ll_i2c_write(grc, outBuff, outBuffLen);
        if (res < 0) {
            return res;
        }
        __resetBuffer();
    }
    for (int i = 0; i < *blockCnt; i++) {
        int res = __putFloatArrayAsBlock(len, vals, i + 1, blockSize);
        if (res < 0) {
            return res;
        }
        if ((BUFFER_SIZE - outBuffLen) < blockSize) {
            res = grc_ll_i2c_write(grc, outBuff, outBuffLen);
            if (res < 0) {
                return res;
            }
            __resetBuffer();
        }
    }
    if (outBuffLen > 0) {
        int res = grc_ll_i2c_write(grc, outBuff, outBuffLen);
        if (res < 0) {
            return res;
        }
        __resetBuffer();
    }
    return I2C_OK;
}

int sendParamArguments(struct grc_ll_i2c_dev* grc, struct Param* arg)
{
    uint8_t blockCnt = 1;
    uint8_t blockSize = INT_SIZE + 1 + 4; // 4 for int arg and 4 for protocol wrap
    __putActivateStreamingCommand(blockSize, blockCnt);
    int res = __putParamAsBlock(arg);
    if (res < 0) {
        return res;
    }
    return grc_ll_i2c_write(grc, outBuff, outBuffLen);
}

int getStreamResult(struct grc_ll_i2c_dev* grc, uint8_t* status)
{

    int res = __writeSimpleCommand(grc, GET_STREAMING_RESULT_CMD, 0);
    if (res < 0) {
        return res;
    }
    grc_ll_sleep(1);
    res = grc_ll_i2c_read(grc, status, STREAMING_RESULT_SIZE);
    if (res < 0) {
        return res;
    }
    return I2C_OK;
}

int callFunction(struct grc_ll_i2c_dev* grc, uint8_t functionCmd)
{
    int res = __writeSimpleCommand(grc, CALL_FUNCTION_CMD, functionCmd);
    return res;
}

int getFunctionStatus(struct grc_ll_i2c_dev* grc, uint8_t functionCmd, struct FunctionExecutionStatus* status)
{
    int res = __writeSimpleCommand(grc, GET_FUNCTION_STATUS_CMD, functionCmd);
    if (res < 0) {
        return res;
    }
    grc_ll_sleep(1);
    res = grc_ll_i2c_read(grc, inBuff, SIMPLE_COMMAND_RESULT_SIZE);
    if (res < 0) {
        return res;
    }
    status->isRunning = ((inBuff[0] >> 6) & 0x01);
    status->isCalled = ((inBuff[0] >> 7) & 0x01);
    status->retcode = inBuff[0] & 0x3f;

    return I2C_OK;
}

int getFunctionResult(struct grc_ll_i2c_dev* grc, uint8_t functionCmd, int* result)
{
    int res = __writeSimpleCommand(grc, GET_FUNCTION_RESULT_CMD, functionCmd);
    if (res < 0) {
        return res;
    }
    grc_ll_sleep(1);
    res = grc_ll_i2c_read(grc, inBuff, GET_FUNCTION_RESULT_CMD);
    if (res < 0) {
        return res;
    }
    *result = getInt(inBuff);
    return I2C_OK;
}

int getCurGRCVersion(struct grc_ll_i2c_dev* grc)
{
    int res = __writeSimpleCommand(grc, GET_SDK_VERSION_CMD, 0);
    if (res < 0) {
        return res;
    }
    grc_ll_sleep(10);
    res = grc_ll_i2c_read(grc, inBuff, INT_SIZE);
    if (res < 0) {
        return res;
    }
    return getInt(inBuff);
}

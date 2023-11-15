#ifndef _PROTOCOL_STRUCTURES_H_
#define _PROTOCOL_STRUCTURES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct FunctionExecutionStatus {
    uint8_t isCalled;
    uint8_t isRunning;
    uint8_t retcode;
};

typedef enum {
    Ok = 0,
    Error = 1,

    InvalState = 10,
    InvalParm,
    InvalDataLen,

    NotCalled = 20,

    NotImplemented = 30
} Retcode;

typedef enum {
    PredictSignal = 1,
    SeparateInaccuracies,
    Noise,
    InputScaling,
    FeedbackScaling,
    ThresholdFactor,

    ArchType = 10,

    AskExtStatus = 20,
    LoadTrainData,
    ReqCategory
} ParamKind;

typedef enum {
    None = 0,
    CatsQty,
    SaveDataLen,
    NextDataElm
} ExtStatusReq;

struct Param {
    ParamKind kind;
    union {
        int ival;
        float fval;
    };
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROTOCOL_STRUCTURES_H_

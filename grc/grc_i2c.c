#include <stdio.h>
#include <string.h>

#include "grc/grc.h"
#include "grc/grc_error_codes.h"
#include "grc/drivers/grc_ll_i2c.h"
#include "grc/protocol_layer/grc_ll_api.h"
#include "grc/protocol_layer/protocol_structures.h"

#define CUR_SDK_VERSION 1

int retcode_to_result(Retcode* retcode)
{
    int res = I2C_ERROR;
    switch (*retcode) {
    case Ok:
        res = 0;
        break;
    case Error:
        res = REMOTE_FUNCTION_ERROR;
        break;
    case InvalState:
        res = REMOTE_FUNCTION_INVAL_STATE;
        break;
    case InvalParm:
        res = REMOTE_FUNCTION_INVAL_PARAM;
        break;
    case InvalDataLen:
        res = REMOTE_FUNCTION_INVAL_DATA_LEN;
        break;
    case NotCalled:
        res = REMOTE_FUNCTION_NOT_CALLED;
        break;
    case NotImplemented:
        res = REMOTE_FUNCTION_NOT_IMPLEMENTED;
        break;
    }
    return res;
}

#define CHECK_REMOTE_CALL(func, res, retcode) \
    res = func;                               \
    if (res < 0) {                            \
        return res;                           \
    }                                         \
    res = retcode_to_result(&retcode);       \
    if (res < 0) {                            \
        return res;                           \
    }

#define MAX_TAG_CNT 5
static int tags_trained[MAX_TAG_CNT] = { -1 };
static int tags_trained_len = 0;

static int get_tag_idx(grc_class_tag_t tag, uint32_t flags)
{
    int class_idx = NOT_CLASSIFIED;
    if (flags & GRC_PARAMS_ADD_NEW_TAG) {
        return class_idx;
    }
    for (int i = 0; i < tags_trained_len; i++) {
        if (tags_trained[i] == tag) {
            class_idx = i;
            break;
        }
    }
    return class_idx;
}

int __get_reservoir_neurons_count(struct grc_config* cfg)
{
    switch (cfg->arch) {
    case I3_N10_1:
        return 10;
    case I3_N10_2:
        return 10;
    case I6_N15_0:
        return 15;
    case CUSTOM:
        return cfg->ReservoirNeurons;
    }
    return -1;
}

int __get_input_components_count(struct grc_config* cfg)
{
    switch (cfg->arch) {
    case I3_N10_1:
        return 3;
    case I3_N10_2:
        return 3;
    case I6_N15_0:
        return 6;
    case CUSTOM:
        return cfg->InputComponents;
    }
    return -1;
}

int __get_reservoir_kind(struct grc_config* cfg)
{
    int reservoir_neurons = __get_reservoir_neurons_count(cfg);
    int input_components = __get_input_components_count(cfg);
    if ((reservoir_neurons < 0) || (input_components < 0)) {
        return ARGUMENT_ERROR;
    }
    int reservoir_kind = NOT_IMPLEMENTED;
    if (input_components == 1) {
        if (reservoir_neurons == 10) {
            reservoir_kind = Ch1_Nn10;
        } else if (reservoir_neurons == 18) {
            reservoir_kind = Ch1_Nn18;
        } else if (reservoir_neurons == 30) {
            reservoir_kind = Ch1_Nn30;
        } else if (reservoir_neurons == 100) {
            reservoir_kind = Ch1_Nn100;
        }
    } else if (input_components == 3) {
        if (reservoir_neurons == 10) {
            reservoir_kind = Ch3_Nn10;
        } else if (reservoir_neurons == 30) {
            reservoir_kind = Ch3_Nn30;
        } else if (reservoir_neurons == 100) {
            reservoir_kind = Ch3_Nn100;
        }
    } else if (input_components == 6) {
        if (reservoir_neurons == 17) {
            reservoir_kind = Ch6_Nn17;
        }
    }
    return reservoir_kind;
}

int __set_params(struct hp_setup* hp, struct Param* param)
{
    int res = 0;
    switch (hp->type) {
    case PREDICT_SIGNAL:
        param->m_kind = PredictSignal;
        param->m_ival = (hp->value > 0) ? 1 : 0;
        break;
    case SEPARATE_INACCURACIES:
        param->m_kind = SeparateInaccuracies;
        param->m_ival = (hp->value > 0) ? 1 : 0;
        break;
    case NOISE:
        param->m_kind = Noise;
        param->m_fval = hp->value;
        break;
    case INPUT_SCALING:
        param->m_kind = InputScaling;
        param->m_fval = hp->value;
        break;
    case FEEDBACK_SCALING:
        param->m_kind = FeedbackScaling;
        param->m_fval = hp->value;
        break;
    case THRESHOLD_FACTOR:
        param->m_kind = ThresholdFactor;
        param->m_fval = hp->value;
        break;

    default:
        res = ARGUMENT_ERROR;
        break;
    }
    return res;
}

int grc_init(struct grc_device* dev, struct grc_config* cfg)
{
    int grc_sdk_version = initProtocolLayer(dev->ll_dev);
    if (grc_sdk_version < 0) {
        // initialization failed
        return grc_sdk_version;
    }
    dev->version = grc_sdk_version;
    if (grc_sdk_version != CUR_SDK_VERSION) {
        return SDK_VERSION_MISMATCH;
    }
    int reservoir_kind = __get_reservoir_kind(cfg);
    if (reservoir_kind < 0) {
        return reservoir_kind;
    }
    int res;
    Retcode retcode;
    struct Param param = { .m_kind = ReservoirKind, .m_ival = reservoir_kind };
    CHECK_REMOTE_CALL(setNeededParameters(dev->ll_dev, &param, &retcode), res, retcode)
    return 0;
}

int grc_release(struct grc_device* dev)
{
    return releaseProtocolLayer(dev->ll_dev);
}

int grc_set_config(struct grc_device* dev, struct hp_setup* hp, int len)
{
    Retcode retcode;
    struct Param param = {};
    for (int i = 0; i < len; i++) {
        int res = __set_params(&hp[i], &param);
        if (res < 0) {
            return res;
        }
        CHECK_REMOTE_CALL(setNeededParameters(dev->ll_dev, &param, &retcode), res, retcode)
    }
    return 0;
}

int grc_clear_state(struct grc_device* dev)
{
    int res;
    Retcode retcode;
    CHECK_REMOTE_CALL(clear(dev->ll_dev, &retcode), res, retcode)
    tags_trained_len = 0;
    return 0;
}

int grc_train(
    struct grc_device* dev,
    struct grc_training_params* params,
    const float* vals,
    uint32_t len)
{
    int class_idx = get_tag_idx(params->tag, params->flags);
    if (!(params->flags & GRC_PARAMS_OVERWRITE) && (class_idx >= 0)) {
        return ARGUMENT_ERROR;
    }
    if (params->flags & GRC_PARAMS_ASYNC) {
        return NOT_IMPLEMENTED;
    } else {
        int res;
        Retcode retcode;
        CHECK_REMOTE_CALL(startTraining(dev->ll_dev, class_idx, &retcode), res, retcode)
        CHECK_REMOTE_CALL(feedData(dev->ll_dev, len, vals, &retcode), res, retcode)
        CHECK_REMOTE_CALL(stopTraining(dev->ll_dev, &retcode), res, retcode)
        if (class_idx < 0) {
            class_idx = tags_trained_len;
            tags_trained[tags_trained_len++] = (params->flags & GRC_PARAMS_ADD_NEW_TAG) ? class_idx : params->tag;
        }
    }
    return class_idx;
}

int grc_inference(
    struct grc_device* dev,
    struct grc_inference_params* params,
    const float* vals,
    uint32_t len)
{
    int res;
    Retcode retcode;
    if (params->flags & GRC_PARAMS_SINGLE_CLASS) {
        int class_idx = get_tag_idx(params->tag, 0);
        if (class_idx < 0) {
            return ARGUMENT_ERROR;
        }
        struct Param param = { .m_kind = ReqCategory, .m_ival = class_idx };
        CHECK_REMOTE_CALL(setNeededParameters(dev->ll_dev, &param, &retcode), res, retcode)
    }
    CHECK_REMOTE_CALL(startInference(dev->ll_dev, &retcode), res, retcode)
    CHECK_REMOTE_CALL(feedData(dev->ll_dev, len, vals, &retcode), res, retcode)
    CHECK_REMOTE_CALL(stopInference(dev->ll_dev, &retcode), res, retcode)
    int class_idx;
    CHECK_REMOTE_CALL(getStatus(dev->ll_dev, &class_idx, &retcode), res, retcode)
    if (class_idx >= tags_trained_len) {
        return WRONG_GRC_ANSWER;
    }
    if (class_idx < 0) {
        return class_idx;
    }
    return tags_trained[class_idx];
}

int grc_wait(struct grc_device* dev)
{
    return NOT_IMPLEMENTED;
}

int grc_get_classes_number(struct grc_device* dev)
{
    int res;
    Retcode retcode;
    struct Param param = { .m_kind = AskExtStatus, .m_ival = CatsQty };
    CHECK_REMOTE_CALL(setNeededParameters(dev->ll_dev, &param, &retcode), res, retcode)

    int class_numbers;
    CHECK_REMOTE_CALL(getStatus(dev->ll_dev, &class_numbers, &retcode), res, retcode)
    return class_numbers;
}

int grc_get_class_info_by_index(
    struct grc_device* dev,
    uint32_t index,
    struct grc_class_info* info)
{
    return NOT_IMPLEMENTED;
}

int grc_get_class_info_by_tag(
    struct grc_device* dev,
    grc_class_tag_t tag,
    struct grc_class_info* info)
{
    return NOT_IMPLEMENTED;
}

int grc_clear_class_by_tag(
    struct grc_device* dev,
    grc_class_tag_t tag)
{
    return NOT_IMPLEMENTED;
}

int grc_download(struct grc_device* dev, struct grc_internal_state* states, uint32_t* len)
{
    int res;
    Retcode retcode;
    int i = 0;
    struct Param param = { .m_kind = AskExtStatus, .m_ival = SaveDataLen };

    CHECK_REMOTE_CALL(setNeededParameters(dev->ll_dev, &param, &retcode), res, retcode)
    int download_len = -1;
    CHECK_REMOTE_CALL(getStatus(dev->ll_dev, &download_len, &retcode), res, retcode)
    if (download_len < 0) {
        return -1;
    }
    typedef float dtype;
    states[i].len = download_len;
    states[i].values = (dtype*)malloc(states[i].len * sizeof(dtype));

    param.m_kind = AskExtStatus;
    param.m_ival = NextDataElm;
    CHECK_REMOTE_CALL(setNeededParameters(dev->ll_dev, &param, &retcode), res, retcode)

    int elm;
    for (unsigned cnt = 0; cnt < download_len; ++cnt) {
        CHECK_REMOTE_CALL(getStatus(dev->ll_dev, &(elm), &retcode), res, retcode)
        memcpy(&(states[i].values[cnt]), &elm, sizeof(dtype));
    }
    *len = 1;

    return grc_get_classes_number(dev);
}

int grc_upload(struct grc_device* dev, struct grc_internal_state* states, uint32_t len)
{
    int res;
    Retcode retcode;
    int j = 0;
    for (unsigned i = 0; i < states[j].len; ++i) {
        CHECK_REMOTE_CALL(feedDataSingle(dev->ll_dev, states[j].values[i], &retcode), res, retcode);
    }
    struct Param param = { .m_kind = LoadTrainData, .m_ival = len };
    CHECK_REMOTE_CALL(setNeededParameters(dev->ll_dev, &param, &retcode), res, retcode)
    tags_trained_len = 0;
    for (int i = 0; i < len; i++) {
        tags_trained[tags_trained_len++] = i;
    }
    return 0;
}

int grc_store(struct grc_device* dev)
{
    return NOT_IMPLEMENTED;
}

int grc_restore(struct grc_device* dev)
{
    return NOT_IMPLEMENTED;
}

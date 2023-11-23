#include "grc/Grc.hpp"
#include <cstdlib>

Grc::Grc(void* ll_dev)
{
    dev_ = grc_device { .ll_dev = ll_dev, .version = 1 };
}

Grc::~Grc()
{
    grc_release(&dev_);
}

int Grc::init(const HP& hp) const
{
    struct grc_config conf = { .arch = uint32_t(ARCH_CONSTRUCTOR(0, hp.InputComponents, hp.Neurons, 0)) };
    int res = grc_init(&dev_, &conf);
    if (res < 0) {
        return res;
    }

    int config_len = 6;
    struct hp_setup config[config_len] = {
        hp_setup { .type = PREDICT_SIGNAL,
            .value = (hp.PredictSignal) ? 1.0f : 0.0f },
        hp_setup { .type = SEPARATE_INACCURACIES,
            .value = (hp.SeparateInaccuracies) ? 1.0f : 0.0f },
        hp_setup { .type = NOISE, .value = (float)hp.Noise },
        hp_setup { .type = INPUT_SCALING, .value = (float)hp.InputScaling },
        hp_setup { .type = FEEDBACK_SCALING, .value = (float)hp.FeedbackScaling },
        hp_setup { .type = THRESHOLD_FACTOR, .value = (float)hp.ThresholdFactor }
    };
    res = grc_set_config(&dev_, config, config_len);
    return res;
}

int Grc::clearState() const
{
    return grc_clear_state(&dev_);
}

int Grc::train(uint32_t len, const float* vals, int category) const
{
    struct grc_training_params training_params = {};
    if (category >= 0) {
        training_params.flags = GRC_PARAMS_OVERWRITE;
        training_params.tag = category;
    } else {
        training_params.flags = GRC_PARAMS_ADD_NEW_TAG;
    }
    int train_category = grc_train(&dev_, &training_params, vals, len);
    return train_category;
}

int Grc::inference(uint32_t len, const float* vals, int category) const
{
    struct grc_inference_params inf_params = {};
    if (category >= 0) {
        inf_params.flags = GRC_PARAMS_SINGLE_CLASS;
        inf_params.tag = category;
    }
    int inf_category = grc_inference(&dev_, &inf_params, vals, len);
    return inf_category;
}

int Grc::wait() const
{
    return GRC_OK;
}

int Grc::getQty() const
{
    return grc_get_classes_number(&dev_);
}

int Grc::getCategoryInfo(uint32_t index, struct grc_class_info* info) const
{
    return GRC_OK;
}

int Grc::clearCategory(uint32_t tag) const
{
    return GRC_OK;
}

int Grc::save(std::vector<float>& data) const
{
    grc_internal_state state = {};
    struct grc_internal_state states[] = { state };
    uint32_t len;
    int res = grc_download(&dev_, states, &len);
    if (res > 0) {
        data.clear();
        uint32_t total_data = 0;
        for (uint32_t j = 0; j < len; j++) {
            total_data += states[j].len;
        }
        if (total_data > 0 && total_data < (1 << 15)) {
            data.reserve(total_data);

            for (uint32_t j = 0; j < len; j++) {
                for (uint32_t i = 0; i < states[j].len; i++) {
                    data.push_back(states[j].values[i]);
                }
            }
        }
        for (uint32_t j = 0; j < len; j++) {
            if (states[j].values != nullptr) {
                free(states[j].values);
            }
        }
    }
    return res;
}

int Grc::load(uint32_t qty, uint32_t len, const float* vals) const
{
    struct grc_internal_state states[1] = {
        grc_internal_state { .tag = 0, .len = len, .values = const_cast<float*>(vals) }
    };
    return grc_upload(&dev_, states, qty);
}

int Grc::store() const
{
    return GRC_OK;
}

int Grc::restore() const
{
    return GRC_OK;
}

int Grc::reset() const
{
    return grc_device_reset(&dev_);
}
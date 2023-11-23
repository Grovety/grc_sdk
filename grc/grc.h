#ifndef _GRC_H_
#define _GRC_H_

#include <stdint.h>
#include "grc_error_codes.h"

/*!
 * \brief flags for grc_training_params and grc_inference_params
 */

#define GRC_PARAMS_ASYNC 0x00000001 // if set then run function in asynchronous mode
#define GRC_PARAMS_OVERWRITE 0x00000002 // allows to train the tag again
#define GRC_PARAMS_SINGLE_CLASS 0x00000004 // run inference in anomaly detection mode (defines belonging to a specific class)
#define GRC_PARAMS_ADD_NEW_TAG 0x00000008 // allows not specify tag for train and just create new one

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef uint32_t grc_class_tag_t;
typedef void (*grc_callback_t)(int status, void* user_data);

/*!
 * \brief types of GRC architecture
 */

#define ARCH_CONSTRUCTOR(reserved, c, n, i) ((uint8_t)(c) << 16 | (uint8_t)(n) << 8 | (uint8_t)(i))

typedef enum {
    I1_N10 = ARCH_CONSTRUCTOR(0, 1, 10, 0),
    I1_N18 = ARCH_CONSTRUCTOR(0, 1, 18, 0),
    I1_N30 = ARCH_CONSTRUCTOR(0, 1, 30, 0),
    I1_N100 = ARCH_CONSTRUCTOR(0, 1, 100, 0),

    I3_N10 = ARCH_CONSTRUCTOR(0, 3, 10, 0),
    I3_N19 = ARCH_CONSTRUCTOR(0, 3, 19, 0),
    I3_N30 = ARCH_CONSTRUCTOR(0, 3, 30, 0),
    I3_N100 = ARCH_CONSTRUCTOR(0, 3, 100, 0),

    I6_N17 = ARCH_CONSTRUCTOR(0, 6, 17, 0),
} ARCH_TYPE;

/*!
 * \brief here are the architectural features of the GRC (the number of neurons, the size of the input)
 *        initialized by the user for the initial configuration of the chip
 */
struct grc_config {
    uint32_t arch;
};

/*!
 * \brief types of hyperparams that can be set
 */
typedef enum {
    PREDICT_SIGNAL,
    SEPARATE_INACCURACIES,
    NOISE,
    INPUT_SCALING,
    FEEDBACK_SCALING,
    THRESHOLD_FACTOR
} hyperparam_types;

/*!
 * \brief structure for setting parameters.
 * \param type Type of configurable parameter
 * \param value Value of configurable parameter (for PREDICT_SIGNAL and SEPARATE_INACCURACIES allowed only 1 and 0)
 */
struct hp_setup {
    hyperparam_types type;
    float value;
};

/*!
 * \brief structure for training parameters.
 * \param flags GRC_PARAMS_ASYNC, GRC_PARAMS_OVERWRITE, GRC_PARAMS_ADD_NEW_TAG
 * \param tag Name of class. can be missed if used GRC_PARAMS_ADD_NEW_TAG
 * \param callback callback for async mode(GRC_PARAMS_ASYNC)
 * \param user_data callback arguments
 */
struct grc_training_params {
    uint32_t flags;
    grc_class_tag_t tag;
    grc_callback_t callback;
    void* user_data;
};

/*!
 * \brief structure for inference parameters.
 * \param flags GRC_PARAMS_ASYNC, GRC_PARAMS_SINGLE_CLASS
 * \param tag Name of class. required for GRC_PARAMS_SINGLE_CLASS
 * \param callback callback for async mode(GRC_PARAMS_ASYNC)
 * \param user_data callback arguments
 */
struct grc_inference_params {
    uint32_t flags;
    grc_class_tag_t tag;
    grc_callback_t callback;
    void* user_data;
};

/*!
 * \brief info for trained class.
 * \param tag Name of class.
 * \param responce_len  len of class info
 * \param responce info
 */
struct grc_class_info {
    grc_class_tag_t tag;
    uint32_t responce_len;
    float* responce;
};

/*!
 * \brief internal states of trained class(weights)
 * \param tag Name of class.
 * \param len  len of values array
 * \param values internal state(weights)
 */
struct grc_internal_state {
    grc_class_tag_t tag;
    uint32_t len;
    float* values;
};

/*!
 * \brief structure for grc device setup
 * \param ll_dev structure with specified transport layer parameters
 * \param version  GRC SDK version
 */
struct grc_device {
    void* ll_dev;
    uint32_t version;
};

/*!
 * \brief interface and grc initialising
 * \param dev structure for grc device setup
 * \param cfg  configuration of GRC architecture
 * \return Ok(=0) or error code (<0)
 */
int grc_init(struct grc_device* dev, struct grc_config* cfg);

/*!
 * \brief release interface
 * \param dev structure for grc device
 * \return Ok(=0) or error code (<0)
 */
int grc_release(struct grc_device* dev);

/*!
 * \brief setup GRC AI parameters
 * \param dev structure for grc device
 * \param hp array of parameter types and values
 * \param len length of hp array
 * \return Ok(=0) or error code (<0)
 */
int grc_set_config(struct grc_device* dev, struct hp_setup* hp, int len);

/*!
 * \brief clear GRC trained state
 * \param dev structure for grc device
 * \return Ok(=0) or error code (<0)
 */
int grc_clear_state(struct grc_device* dev);

/*!
 * \brief Train GRC on raw data
 * \param dev structure for grc device
 * \param params train parameters
 * \param vals Pointer to train data
 * \param len Train data len
 * \return trained class id(>= 0) or error code (<0)
 */
int grc_train(
    struct grc_device* dev,
    struct grc_training_params* params,
    const float* vals,
    uint32_t len);

/*!
 * \brief Inference on raw data.
 * \param dev structure for grc device
 * \param params inference parameters
 * \param vals Pointer to inference data.
 * \param len Inference data len.
 * \return trained class id(>= 0) or error code (<0). error_code -1 for NOT_CLASSIFIED
 */
int grc_inference(
    struct grc_device* dev,
    struct grc_inference_params* params,
    const float* vals,
    uint32_t len);

/*!
 * \brief (NOT IMPLEMENTED) wait for train or inference execution end
 * \param dev structure for grc device
 * \return Ok(=0) or error code (<0)
 */
int grc_wait(struct grc_device* dev);

/*!
 * \brief get number of trained classes
 * \return number of trained classes (>= 0) or error code (<0).
 */
int grc_get_classes_number(struct grc_device* dev);

/*!
 * \brief (NOT IMPLEMENTED)get class info by class index
 * \param dev structure for grc device
 * \param index index of requested class
 * \param info structure where will be put class info
 * \return class id(>= 0) or error code (<0).
 */
int grc_get_class_info_by_index(
    struct grc_device* dev,
    uint32_t index,
    struct grc_class_info* info);

/*!
 * \brief (NOT IMPLEMENTED)get class info by class tag
 * \param dev structure for grc device
 * \param tag tag of requested class
 * \param info structure where will be put class info
 * \return class id(>= 0) or error code (<0).
 */
int grc_get_class_info_by_tag(
    struct grc_device* dev,
    grc_class_tag_t tag,
    struct grc_class_info* info);

/*!
 * \brief (NOT IMPLEMENTED)clear trained class with tag
 * \param dev structure for grc device
 * \param tag tag of requested class
 * \return class id(>= 0) or error code (<0).
 */
int grc_clear_class_by_tag(
    struct grc_device* dev,
    grc_class_tag_t tag);

/*!
 * \brief download trained model
 * \param dev structure for grc device
 * \param states array of states for each of the classes(in version 1 states of all classes downloaded into single grc_internal_state)
 * \param len states array length (in version 1 always len = 1)
 * \return class numbers(>= 0) or error code (<0).
 */
int grc_download(struct grc_device* dev, struct grc_internal_state* states, uint32_t* len);

/*!
 * \brief upload pre-trained model to GRC
 * \param dev structure for grc device
 * \param states array of states for each of the classes(in version 1 states of all classes should be placed into single grc_internal_state)
 * \param len class numbers(in version 1 not equal to states length)
 * \return Ok(=0) or error code (<0).
 */
int grc_upload(struct grc_device* dev, struct grc_internal_state* states, uint32_t len);

/*!
 * \brief (NOT IMPLEMENTED) store trained model into GRC internal memory
 * \param dev structure for grc device
 * \return Ok(=0) or error code (<0).
 */
int grc_store(struct grc_device* dev);

/*!
 * \brief (NOT IMPLEMENTED) restore trained model from GRC internal memory
 * \param dev structure for grc device
 * \return Ok(=0) or error code (<0).
 */
int grc_restore(struct grc_device* dev);

/*!
 * \brief reset GRC device
 * \param dev structure for grc device
 * \return Ok(=0) or error code (<0).
 */
int grc_device_reset(struct grc_device* dev);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GRC_H_

#ifndef _GRC_HPP_
#define _GRC_HPP_

#include "grc/grc.h"

#include <vector>

/*!
 * \brief Hyper parameters to GRC AI SW.
 */
struct HP {
    uint32_t PredictSignal;
    uint32_t SeparateInaccuracies;
    uint32_t InputComponents;
    uint32_t OutputComponents;
    uint32_t Neurons;
    float SpectralRadius;
    float Sparsity;
    float Noise;
    float InputScaling;
    float InputSparsity;
    float FeedbackScaling;
    float FeedbackSparsity;
    float ThresholdFactor;
};

/*!
 * \brief GRC device.
 */
class Grc {
public:
    /*!
    * \brief Constructor.
    * \param ll_dev Pointer to grc ll device
    */
    Grc(void* ll_dev);

    /*!
    * \brief Destructor.
    */
    ~Grc();

    /*!
    * \brief Initialize GRC device and GRC AI SW.
    * \param hp Hyper parameters to GRC AI SW.
    * \return Error code.
    */
    int init(const HP &hp) const;

    /*!
    * \brief Clear GRC AI SW state.
    * \return Error code.
    */
    int clearState() const;

    /*!
    * \brief Train GRC AI SW on raw data.
    * \param len Train data len.
    * \param vals Pointer to train data.
    * \param category Overwrite specific category in GRC AI SW.
    * \return Trained category.
    */
    int train(uint32_t len, const float *vals, int category) const;
    /*!
    * \brief Inference on raw data.
    * \param len Inference data len.
    * \param vals Pointer to inference data.
    * \param category Hint category.
    * \return Inferenced category.
    */
    int inference(uint32_t len, const float *vals, int category = -1) const;
    /*!
    * \brief (NOT IMPLEMENTED) Wait for train or inference execution end
    * \return Ok(=0) or error code (<0)
    */
    int wait() const;
    /*!
    * \brief Get the number of trained categories.
    * \return Number of trained categories.
    */
    int getQty() const;
    /*!
    * \brief (NOT IMPLEMENTED) Get category info by category index
    * \param index index of requested category
    * \param info structure where will be put category info
    * \return category id(>= 0) or error code (<0).
    */
    int getCategoryInfo(uint32_t index, struct grc_class_info* info) const;
    /*!
    * \brief (NOT IMPLEMENTED) Clear trained category with tag
    * \param index index of requested category
    * \return category id(>= 0) or error code (<0).
    */
    int clearCategory(uint32_t index) const;
    /*!
    * \brief Retrieve train metadata from GRC .
    * \param data Where to save.
    * \return Number of trained categories.
    */
    int save(std::vector<float> &data) const;
    /*!
    * \brief Load train metadata.
    * \param qty Number of trained categories.
    * \param len Buffer size.
    * \param val Pointer to data.
    * \return Whether the data was sent successfully to GRC.
    */
    int load(uint32_t qty, uint32_t len, const float *vals) const;
    /*!
    * \brief (NOT IMPLEMENTED) Store trained model into GRC internal memory
    * \return Ok(=0) or error code (<0).
    */
    int store() const;
    /*!
    * \brief (NOT IMPLEMENTED) Restore trained model from GRC internal memory
    * \return Ok(=0) or error code (<0).
    */
    int restore() const;
    /*!
    * \brief Reset GRC device
    * \return Ok(=0) or error code (<0).
    */
    int reset() const;

protected:
    /*! \brief Device structure. */
    mutable grc_device dev_;
};

#endif //_GRC_HPP_
#ifndef _GRC_LL_API_H_
#define _GRC_LL_API_H_

#include <stdint.h>
#include "grc/drivers/grc_ll_i2c.h"
#include "grc/protocol_layer/protocol_structures.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int initProtocolLayer(struct grc_ll_i2c_dev* grc);

int setNeededParameters(struct grc_ll_i2c_dev* grc, struct Param* param, Retcode* retcode);

int startTraining(struct grc_ll_i2c_dev* grc, int category, Retcode* retcode);

int stopTraining(struct grc_ll_i2c_dev* grc, Retcode* retcode);

int startInference(struct grc_ll_i2c_dev* grc, Retcode* retcode);

int stopInference(struct grc_ll_i2c_dev* grc, Retcode* retcode);

int feedDataSingle(struct grc_ll_i2c_dev* grc, float val, Retcode* retcode);
int feedData(struct grc_ll_i2c_dev* grc, unsigned len, const float* vals, Retcode* retcode);

int getStatus(struct grc_ll_i2c_dev* grc, int* pstat, Retcode* retcode);

int clear(struct grc_ll_i2c_dev* grc, Retcode* retcode);

int releaseProtocolLayer(struct grc_ll_i2c_dev* grc);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GRC_LL_API_H_

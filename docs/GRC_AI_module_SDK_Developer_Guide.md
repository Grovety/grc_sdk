# GRC AI Module SDK Developer Guide

## Overview

<img src="media/grc_sdk.png" width=400px>

GRC AI module – is a software designed to perform classification and class recognition tasks.
Functionality includes:

* AI SW on-device learning
* parameters configuration
* class recognition
* classification
* saving and loading a pre-trained model
GRC_SDK serves as a communication interface for GRC AI module over I2C bus.

<img src="media/grc_sdk_arch.png" width=400px>

GRC_SDK has several abstraction layers:

* **Transport layer** – a platform-dependent interface to work with I2C bus. It provides basic functions: configuring, reading and writing (see drivers);
* **Protocol layer** – a protocol for function remote call on GRC implemented over transport layer (see protocol_layer);
* **Application layer** – high-layer interface for calling GRC function.

## Use Cases

The basic GRC AI SW scenario includes parameter configuration, learning, and category classification based on data.

```cpp
// === 1. Driver’s Initialization ===
struct grc_ll_i2c_dev ll_dev = {
    //constant for selecting communication type
    .type = PROTOCOL_INTERFACE_I2C,
    .sda_io_num = <SDA PIN>,
    .scl_io_num = <SCL PIN>,
    .data_ready_io_num = <DATA READY PIN>,
    .i2c_num = <I2C PORT>,
    .clk_speed = 400000,
    .slave_addr = 0x36,
    .timeout_us = 1000
  };
struct grc_device dev = {.ll_dev = &ll_dev};

// GRC AI SW architecture parameters
static struct grc_config arch_conf = {.arch = I3_N10_1};

int result = grc_init(&dev, &arch_conf);

// === 2. Configuring GRC AI SW Parameters ===
int ai_params_len = 6;

struct hp_setup ai_params[ai_params_len];
  ai_params[0].type = PREDICT_SIGNAL;
  ai_params[0].value = 1;
  ai_params[1].type = SEPARATE_INACCURACIES;
  ai_params[1].value = 0;
  ai_params[2].type = NOISE;
  ai_params[2].value = 0.001;
  ai_params[3].type = INPUT_SCALING;
  ai_params[3].value = 0.5;
  ai_params[4].type = FEEDBACK_SCALING;
  ai_params[4].value = 0.001;
  ai_params[5].type = THRESHOLD_FACTOR;
  ai_params[5].value = 1;
result = grc_set_config(&dev, ai_params, ai_params_len);

// === 3. Preparing for Classification ===
//--- 3.a Training ---.
struct grc_training_params train_params;
train_params.flags = GRC_PARAMS_ADD_NEW_TAG;
float *train_data;
uint32_t train_data_len;

result = grc_train(&dev, &train_params, train_data, train_data_len);

// === 4. Classification ===
struct grc_inference_params inference_params;
float *inference_data;
unsigned inference_data_len;
int result_category = grc_inference(&dev, &inference_params, train_data, train_data_len);

// === 5. Clearing AI SW State===
result = grc_clear_state(&dev);

// === 6.  Driver’s De-initialization ===
result = grc_release(&dev);
```

To avoid training every time, a model can be pre-trained, and the result will be saved for further usage.

```cpp
// === 3b.1. Training and Saving a Model ===
struct grc_training_params train_params;
float *train_data;
uint32_t train_data_len;

result = grc_train(&dev, &train_params, train_data, train_data_len);

// ---------- Save in memory of the Host device ------
struct grc_internal_state *internal_states;
uint32_t internal_states_len;
result = grc_download(&dev, &internal_state, &internal_states_len);

// ---------- or in the GRC internal memory----------
result = grc_store(&dev);
// Afterwards, the model can be loaded to GRC

// === 3b.2 Loading a pre-trained model ===
// ------------- from Host device memory ----------
result = grc_upload(&dev, internal_states, internal_states_len);
// ------------- or from GRC internal memory --------
result = grc_restore(&dev);
```

## Methods

### Device Configuration

Initialization of interface **dev** (grc_device) and configuration of AI SW (grc_config) architecture.    Returns 0 in case of success or an error code (<0)

```cpp
int grc_init(
    struct grc_device* dev,
    struct grc_config* cfg);
```

Releasing resources occupied by driver.    Returns 0 in case of success or an error code (<0)

```cpp
int grc_release(struct grc_device* dev);
```

### AI SW Configuration

Configuration of AI SW work via parameters array **hp** (hp_setup) of **len** length    Returns 0 in case of success or an error code (<0)

```cpp
int grc_set_config(
    struct grc_device* dev,
    struct hp_setup* hp,
    int len);
```

### Training GRC on raw data

* **params** – training parameters (grc_training_params )
* **vals**  – pointer to training data
* **len**  – array length
Returns id of the trained class (>=0) in case of success or an error code (<0)

```cpp
int grc_train(
    struct grc_device* dev,
    struct grc_training_params* params,
    float* vals,
    uint32_t len);
```

(NOT IMPLEMENTED)
Deletion of a trained class with the tag defined in info (grc_class_info).
Returns id of the deleted class (>=0) in case of success or an error code (<0)

```cpp
 int grc_clear_class_by_tag(
    struct grc_device* dev,
    grc_class_tag_t tag);
```

Resetting to AI SW default settings.
Returns performance status: 0 in case of success, or <0 in case of error

```cpp
int grc_clear_state(struct grc_device* dev)
```

### Detection / classification

Inference on raw data:

* **params** – inference parameters (grc_inference_params )
* **vals**  – pointer to training data
* **len**  – array length

Returns id of the class (>=0), to which data belong or an error code (<0)
error code -1 – failed to determine a class

```cpp
int grc_inference(
    struct grc_device* dev,
    struct grc_inference_params* params,
    float* vals,
    uint32_t len);
```

(NOT IMPLEMENTED)
Wait till inference or training ends (for asynchronous mode)

```cpp
int grc_wait(struct grc_device* dev)
```

### Information about AI SW

Returns the number of trained classes (>=0) or an error code (<0)

```cpp
int grc_get_classes_number(struct grc_device* dev);
```

(NOT IMPLEMENTED)
Forming class information with **index**  in structure  **info**
Returns id (>=0) in case of success or an error code (<0)

```cpp
int grc_get_class_info_by_index(
    struct grc_device * dev,
    uint32_t index,
    struct grc_class_info * info);
```

(NOT IMPLEMENTED)
Forming class information with **tag**  in structure  **info**
Returns id (>=0) in case of success or an error code (<0)

```cpp
int grc_get_class_info_by_tag(
    struct grc_device* dev,
    grc_class_tag_t tag,
    struct grc_class_info* info);
```

### Saving / Loading AI SW

Placing information (grc_internal_state) about each **len** class into **states** array.
Returns the number of classes trained on GRC (>= 0) in case of success or an error code (<0).

_NOTE:_ in current implementation, information about all classes is stored in one array, therefore, length of array states (**len**) always equals to 1.

```cpp
int grc_download(
    struct grc_device* dev,
    grc_internal_state* states,
    uint32_t* len)
```

Loading the information (grc_internal_state) about each **len** class into **states** array.
Returns 0 in case of success or an error code (<0).
_NOTE_:  in current implementation, information about all classes is stored in one array, therefore, length of array states ( **len** ) always equals to 1, though the value **len** shall correspond the number of classes.

```cpp
int grc_upload(
    struct grc_device* dev,
    grc_internal_state* states,
    uint32_t len);
```

(NOT IMPLEMENTED)
Saving current state of AI SW into GRC internal memory.
Returns 0 in case of success or an error code (<0).

```cpp
int grc_store(struct grc_device* dev);
```

(NOT IMPLEMENTED)
Restoring AI SW state from GRC internal memory.
Returns 0 in case of success or an error code (<0).

```cpp
int grc_restore(struct grc_device* dev);
```

## Error Codes

### Error codes at protocol layer

| **Error** | **Error code** | **Meaning** |
| --- | --- | --- |
| I2C_OK | 0 | Function works correctly |
| NOT_CLASSIFIED | -1 | Data belong to none of the classes |
| I2C_ERROR | -2 | Transport layer error |
| ARGUMENT_ERROR | -3 | Argument layer error. E.g., negative values in configuration of GRC AI SW architecture, incorrect flags, an attempt to set up a non-existent **hp** (hp_setup) parameter, an attempt to classify data as a non-existent class |
| WRONG_GRC_ANSWER | -4 | Fails to interpret GRC response |
| GRC_IS_BUSY | -5 | GRC cannot start performing a new function while the previous one is still running |
| DATA_NOT_DELIVERED | -6 | Data have not been delivered to GRC |
| NOT_IMPLEMENTED | -7 | The functionality is yet to be implemented |
| SDK_VERSION_MISMATCH | -8 | The GRC_SDK version does not match the GRC firmware version |

### Error code, which are returned by remote functions

| **Error** | **Error code** | **Meaning** |
| --- | --- | --- |
| REMOTE_FUNCTION_ERROR | -20 |   |
| REMOTE_FUNCTION_INVAL_STATE | -21 |   |
| REMOTE_FUNCTION_INVAL_PARAM | -22 |   |
| REMOTE_FUNCTION_INVAL_DATA_LEN | -23 |   |
| REMOTE_FUNCTION_NOT_CALLED | -24 |   |
| REMOTE_FUNCTION_NOT_IMPLEMENTED | -25 |   |

## Structures

### grc_device

Description of the remote GRC AI SW device.

| **Field** | **Description** |
| --- | --- |
| void* ll_dev | Information about used driver (grc_ll_i2c_dev for I2C) |
| uint32_t version | GRC firmware version. It is set up during interface initialization call |

### grc_ll_i2c_dev

GRC AI SW driver parameters

| **Field** | **Description** |
| --- | --- |
| uint32_t type | Remote connection type (I2C/SPI/UART). For I2C PROTOCOL_INTERFACE_I2C value is used |
| int sda_io_num | SDA pin |
| int scl_io_num | SCL pin |
| int data_ready_io_num | Date readiness interrupt pin |
| int i2c_num | I2C port |
| uint32_t clk_speed | Clock frequency for I2C master (not greater than 400KHz) |
| uint16_t slave_addr | GRC device address |
| uint32_t timeout_us | GRC response waiting time in milliseconds |

### grc_config

AI SW Architecture Details

Initialized by user during the first chip configuration

| **Field** | **Description** |
| --- | --- |
| uint32_t arch | Configuration type: configuration can be selected from the ARCH_TYPE list. OR set arch into CUSTOM value and configure each of the parameters described below |
| uint32_t InputComponents | Input size |
| uint32_t OutputComponents | Output size |
| uint32_t ReservoirNeurons | Neurons number |

### hp_setup

Structure for configuring classification parameters

| **Field** | **Description** |
| --- | --- |
| hyperparam_types type | Parameter configuration |
| float value | Value of the configured parameter |

### hyperparam_types

| **Field** | **Description** |
| --- | --- |
| PREDICT_SIGNAL | Takes value 1 or 0 |
| SEPARATE_INACCURACIES | Takes value 1 or 0 |
| NOISE |   |
| INPUT_SCALING |   |
| FEEDBACK_SCALING |   |
| THRESHOLD_FACTOR |   |

### grc_training_params

| **Field** | **Description** |
| --- | --- |
| uint32_t flags | Allow/Forbid overwriting the existing class with this tag (GRC_PARAMS_OVERWRITE). Perform synchronously or asynchronously (GRC_PARAMS_ASYNC). Train a class with the tag or create a new tag(GRC_PARAMS_ADD_NEW_TAG) |
| grc_class_tag_t tag | Class name |
| grc_callback_t callback | Performance processing for asynchronous variant (NOT IMPLEMENTED) |
| void* user_data | Callback arguments (NOT IMPLEMENTED) |

### grc_inference_params

| **Field** | **Description** |
| --- | --- |
| uint32_t flags | Switch on/off class classification (GRC_PARAMS_SINGLE_CLASS). Perform synchronously or asynchronously (GRC_PARAMS_ASYNC) |
| grc_class_tag_t tag | Name of the class for which classification is done. (In case flag GRC_PARAMS_SINGLE_CLASS is set) |
| grc_callback_t callback | Performance processing for asynchronous variant (NOT IMPLEMENTED) |
| void* user_data | Callback arguments (NOT IMPLEMENTED) |

### grc_class_info

| **Field** | **Description** |
| --- | --- |
| grc_class_tag_t tag | Class name |
| uint32_t responce_len | Length of response array |
| float* responce | Class parameters |

### grc_internal_state

Structure that contains the internal AI SW device for a particular tag

| **Field** | **Description** |
| --- | --- |
| grc_class_tag_t tag | Class name |
| uint32_t len | Length of values array |
| double* values | Internal model value for recognizing tag |

### Data Types

`typedef uint32_t grc_class_tag_t;`

`typedef void(*grc_callback_t)(int status, void* user_data);`

## GRC_SDK Structure

### examples

Contains examples of work with sdk

* **async_excange.c** – file includes examples on synchronous and asynchronous classification function call (grc_inference)

### grc

SDK Code:

* **drivers** - [Transport Layer] – grc remote protocols. Includes interaction interface over I2C **grc_ll_i2c.h** and implementation for different platforms (MCU Specific code):
* **grc_ll_i2c_esp32.c**
* **grc_ll_i2c_stm32.c**
* **protocol_layer** – [Protocol Layer] – protocol of remote function calls on GRC
* **crc_calculation.h/crc_calculation.c** – calculation of checksum to check integrity of the sent and received data
* **grc_ll_api.h/grc_ll_api.c** – deleted GRC functions
* **grc_ll_protocol_commands.h/grc_ll_protocol_commands.c** – protocol layers which implements various function call steps: GRC status check, argument transfer, function call, waiting till function is over, receiving finished function code, receiving returned values
* **protocol_structures.h** – data structures required for remote call of deleted functions (grc_ll_api)
* **grc.h** – [Application Layer] – API for communicating with GRC (High Level API)
* **grc_i2c.с** - [Application Layer] –interface implementation grc.h for I2C protocol

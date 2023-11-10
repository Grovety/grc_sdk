# GRC AI Module SDK Developer Guide 
/GRC_SDK/

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
    .type = PROTOCOL_INTERFACE_I2C, //constant for selecting communication type
    .sda_io_num = <SDA PIN>,
    .scl_io_num = <SCL PIN>,
    .data_ready_io_num = <DATA READY PIN>,
    .i2c_num = <I2C PORT>,
    .clk_speed = 400000,
    .slave_addr = 0x36,
    .timeout_us = 1000
  };
struct grc_device dev = {.ll_dev = &ll_dev};
static struct grc_config arch_conf = {.arch = I3_N10_1}; // GRC AI SW architecture parameters

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
result = grc_store(&dev)
Afterwards, the model can be loaded to GRC
// === 3b.2 Loading a pre-trained model ===
// ------------- from Host device memory ----------
result = grc_upload(&dev, internal_states, internal_states_len);
// ------------- or from GRC internal memory --------
result = grc_restore(&dev)
```

## Methods
### Device Configuration
```
int grc\_init( struct grc\_device\* dev,
            struct grc\_config\* cfg)
```
Initialization of interface **dev** (grc\_device) and configuration of AI SW (grc\_config) architecture.    Returns 0 in case of success or an error code (\<0)


```
int grc\_release(struct grc\_device\* dev)
```
Releasing resources occupied by driver.    Returns 0 in case of success or an error code (\<0)

### AI SW Configuration
```
int grc\_set\_config(struct grc\_device\* dev,
                                   struct hp\_setup \*hp, int len)
```
Configuration of AI SW work via parameters array **hp** (hp\_setup) of **len** length    Returns 0 in case of success or an error code (\<0)


```
int grc\_train(struct grc\_device\* dev,
                           struct grc\_training\_params\* params,
                           float\* vals,
                           uint32\_t len);
```
Training GRC on raw data   
**vals**  – pointer to training data   
**len**  – array length   
**params** – training parameters (grc\_training\_params )   
Returns id of the trained class (\>=0) in case of success or an error code (\<0) 


 ```
 int grc\_clear\_class\_by\_tag( struct grc\_device\* dev,
                                                   grc\_class\_tag\_t tag)
```
(NOT IMPLEMENTED)    
Deletion of a trained class with the tag defined in info (grc\_class\_info)    
Returns id of the deleted class (\>=0) in case of success or an error code (\<0)    


```
int grc\_clear\_state(struct grc\_device\* dev)
```
Resetting to AI SW default settings    
Returns performance status: 0 in case of success, or \<0 in case of error    

### Detection / classification
```
int grc\_inference(struct grc\_device\* dev,
                                   struct grc\_inference\_params\* params,
                                   float\* vals,
                                   uint32\_t len);
```
Inference on raw data    
**len –** inference data **len**    
**vals** – pointer to inference **dat**    
Returns id class (\>=0), to which data belong or an error code (\<0)    
error code -1 – failed to determine a class
```
int grc\_wait(struct grc\_device\* dev)
```
(NOT IMPLEMENTED)    
Wait till inference or training ends (for asynchronous mode)
### Information about AI SW
```int grc\_get\_classes\_number(struct grc\_device\* dev);
```
Returns the number of trained classes (\>=0) or an error code (\<0) 
```
int grc\_get\_class\_info\_by\_index(struct grc\_device \* dev,
                                                          uint32\_t index,
                                                           struct
 grc\_class\_info \* info);
```
(NOT IMPLEMENTED)    
Forming class information with **index**  in structure  **info**    
Returns id (\>=0) in case of success or an error code (\<0)
```
int grc\_get\_class\_info\_by\_tag(struct grc\_device \* dev,
                                                      grc\_class\_tag\_t tag,
                                                      struct grc\_class\_info \* info);
```
(NOT IMPLEMENTED)    
Forming class information with **tag**  in structure  **info**    
Returns id (\>=0) in case of success or an error code (\<0)
### Saving / Loading AI SW
```
int grc\_download(struct grc\_device\* dev,
                                    grc\_internal\_state\* states,
                                     uint32\_t \*len)
```
Placing information (grc\_internal\_state) about each
**len** class into **states** array    Returns the number of classes trained on GRC (\>= 0) in case of success or an error code (\<0)    
 NOTE:  in current implementation, information about all classes is stored in one array, therefore, length of array states ( **len** ) always equals to 1 
```
int grc\_upload(struct grc\_device\* dev,
                                    grc\_internal\_state\* states,
                                     uint32\_t len)
```
Loading information (grc\_internal\_state) about each    
**len** class into **states** array    
Returns 0 in case of success or an error code (\<0)    
NOTE:  in current implementation, information about all classes is stored in one array, therefore, length of array states ( **len** ) always equals to 1, though the value **len** shall correspond the number of classes
```
int grc\_store(struct grc\_device\* dev)
```
(NOT IMPLEMENTED)    
Saving current state of AI SW into GRC internal memory    
Returns 0 in case of success or an error code (\<0)
```
int grc\_restore(struct grc\_device\* dev)
```
(NOT IMPLEMENTED)    
Restoring AI SW state from GRC internal memory    
Returns 0 in case of success or an error code (\<0)

## Error Codes

### Error codes at protocol layer

| **Error** | **Error code** | **Meaning** |
| --- | --- | --- |
| I2C\_OK | 0 | Function works correctly |
| --- | --- | --- |
| NOT\_CLASSIFIED | -1 | Data belong to none of the classes |
| I2C\_ERROR | -2 | Transport layer error |
| ARGUMENT\_ERROR | -3 | Argument layer error. E.g., negative values in configuration of GRC AI SW architecture, incorrect flags, an attempt to set up a non-existent **hp** (hp\_setup) parameter, an attempt to classify data as a non-existent class |
| WRONG\_GRC\_ANSWER | -4 | Fails to interpret GRC response |
| GRC\_IS\_BUSY | -5 | GRC cannot start performing a new function while the previous one is still running |
| DATA\_NOT\_DELIVERED | -6 | Data have not been delivered to GRC |
| NOT\_IMPLEMENTED | -7 | The functionality is yet to be implemented |
| SDK\_VERSION\_MISMATCH | -8 | The GRC\_SDK version does not match the GRC firmware version |

### Error code, which are returned by deleted functions

| **Error** | **Error code** | **Meaning** |
| --- | --- | --- |
| REMOTE\_FUNCTION\_ERROR | -20 |   |
| REMOTE\_FUNCTION\_INVAL\_STATE | -21 |   |
| REMOTE\_FUNCTION\_INVAL\_PARAM | -22 |   |
| REMOTE\_FUNCTION\_INVAL\_DATA\_LEN | -23 |   |
| REMOTE\_FUNCTION\_NOT\_CALLED | -24 |   |
| REMOTE\_FUNCTION\_NOT\_IMPLEMENTED | -25 |   |

## Structures

### grc\_device

Description of the remote GRC AI SW device.

| **Field** | **Description** |
| --- | --- |
| void\* ll\_dev | Information about used driver (grc\_ll\_i2c\_dev for I2C) |
| uint32\_t version | GRC firmware version. It is set up during interface initialization call |

### grc\_ll\_i2c\_dev

GRC AI SW driver parameters

| **Field** | **Description** |
| --- | --- |
| uint32\_t type | Remote connection type (I2C/SPI/UART). For I2C PROTOCOL\_INTERFACE\_I2C value is used |
| int sda\_io\_num | SDA pin |
| int scl\_io\_num | SCL pin |
| int data\_ready\_io\_num | Date readiness interrupt pin |
| int i2c\_num | I2C port |
| uint32\_t clk\_speed | Clock frequency for I2C master (not greater than 400KHz) |
| uint16\_t slave\_addr | GRC device address |
| uint32\_t timeout\_us | GRC response waiting time in milliseconds |

### grc\_config

AI SW Architecture Peculiarities

Initialized by user during the first chip configuration

| **Field** | **Description** |
| --- | --- |
| uint32\_t arch | Configuration type: configuration can be selected from the ARCH\_TYPE list. OR set arch into CUSTOM value and configure each of the parameters described below |
| uint32\_t InputComponents | Input size |
| uint32\_t OutputComponents | Output size |
| uint32\_t ReservoirNeurons | Neurons number |

**hp\_setup**

Structure for configuring classification parameters

| **Field** | **Description** |
| --- | --- |
| hyperparam\_types type | Parameter configuration |
| float value | Value of the configured parameter |

hyperparam\_types

| **Field** | **Description** |
| --- | --- |
| PREDICT\_SIGNAL | Takes value 1 or 0 |
| SEPARATE\_INACCURACIES | Takes value 1 or 0 |
| NOISE |   |
| INPUT\_SCALING |   |
| FEEDBACK\_SCALING |   |
| THRESHOLD\_FACTOR |   |

### grc\_training\_params

| **Field** | **Description** |
| --- | --- |
| uint32\_t flags | Allow/Forbid overwriting the existing class with this tag (GRC\_PARAMS\_OVERWRITE). Perform synchronously or asynchronously (GRC\_PARAMS\_ASYNC). Train a class with the tag or create a new tag(GRC\_PARAMS\_ADD\_NEW\_TAG) |
| grc\_class\_tag\_t tag | Class name |
| grc\_callback\_t callback | Performance processing for asynchronous variant (NOT IMPLEMENTED) |
| void\* user\_data | Callback arguments (NOT IMPLEMENTED) |

### grc\_inference\_params

| **Field** | **Description** |
| --- | --- |
| uint32\_t flags | Switch on/off class classification (GRC\_PARAMS\_SINGLE\_CLASS). Perform synchronously or asynchronously (GRC\_PARAMS\_ASYNC) |
| grc\_class\_tag\_t tag | Name of the class for which classification is done. (In case flag GRC\_PARAMS\_SINGLE\_CLASS is set) |
| grc\_callback\_t callback | Performance processing for asynchronous variant (NOT IMPLEMENTED) |
| void\* user\_data | Callback arguments (NOT IMPLEMENTED) |

### grc\_class\_info

| **Field** | **Description** |
| --- | --- |
| grc\_class\_tag\_t tag | Class name |
| uint32\_t responce\_len | Length of response array |
| float\* responce | Class parameters |

### grc\_internal\_state

Structure that contains the internal AI SW device for a particular tag

| **Field** | **Description** |
| --- | --- |
| grc\_class\_tag\_t tag | Class name |
| uint32\_t len | Length of values array |
| double\* values | Internal model value for recognizing tag |

### Data Types

`typedef double RT;`

`typedef uint32\_t grc\_class\_tag\_t;`

`typedef void(\*grc\_callback\_t)(int status, void\* user\_data);`

## GRC_SDK Structure
**examples**
Contains examples of work with sdk
* **async_excange.c** – file includes examples on synchronous and asynchronous classification function call (grc_inference)

**grc**

SDK Code
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



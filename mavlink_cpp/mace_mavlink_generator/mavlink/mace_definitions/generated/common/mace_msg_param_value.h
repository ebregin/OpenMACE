#pragma once
// MESSAGE PARAM_VALUE PACKING

#define MACE_MSG_ID_PARAM_VALUE 11

MACEPACKED(
typedef struct __mace_param_value_t {
 float param_value; /*< Onboard parameter value*/
 uint16_t param_count; /*< Total number of onboard parameters*/
 uint16_t param_index; /*< Index of this onboard parameter*/
 char param_id[16]; /*< Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string*/
 uint8_t param_type; /*< Onboard parameter type: see the UXV_PARAM_TYPE enum for supported data types.*/
}) mace_param_value_t;

#define MACE_MSG_ID_PARAM_VALUE_LEN 25
#define MACE_MSG_ID_PARAM_VALUE_MIN_LEN 25
#define MACE_MSG_ID_11_LEN 25
#define MACE_MSG_ID_11_MIN_LEN 25

#define MACE_MSG_ID_PARAM_VALUE_CRC 220
#define MACE_MSG_ID_11_CRC 220

#define MACE_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN 16

#if MACE_COMMAND_24BIT
#define MACE_MESSAGE_INFO_PARAM_VALUE { \
    11, \
    "PARAM_VALUE", \
    5, \
    {  { "param_value", NULL, MACE_TYPE_FLOAT, 0, 0, offsetof(mace_param_value_t, param_value) }, \
         { "param_count", NULL, MACE_TYPE_UINT16_T, 0, 4, offsetof(mace_param_value_t, param_count) }, \
         { "param_index", NULL, MACE_TYPE_UINT16_T, 0, 6, offsetof(mace_param_value_t, param_index) }, \
         { "param_id", NULL, MACE_TYPE_CHAR, 16, 8, offsetof(mace_param_value_t, param_id) }, \
         { "param_type", NULL, MACE_TYPE_UINT8_T, 0, 24, offsetof(mace_param_value_t, param_type) }, \
         } \
}
#else
#define MACE_MESSAGE_INFO_PARAM_VALUE { \
    "PARAM_VALUE", \
    5, \
    {  { "param_value", NULL, MACE_TYPE_FLOAT, 0, 0, offsetof(mace_param_value_t, param_value) }, \
         { "param_count", NULL, MACE_TYPE_UINT16_T, 0, 4, offsetof(mace_param_value_t, param_count) }, \
         { "param_index", NULL, MACE_TYPE_UINT16_T, 0, 6, offsetof(mace_param_value_t, param_index) }, \
         { "param_id", NULL, MACE_TYPE_CHAR, 16, 8, offsetof(mace_param_value_t, param_id) }, \
         { "param_type", NULL, MACE_TYPE_UINT8_T, 0, 24, offsetof(mace_param_value_t, param_type) }, \
         } \
}
#endif

/**
 * @brief Pack a param_value message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param param_id Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
 * @param param_value Onboard parameter value
 * @param param_type Onboard parameter type: see the UXV_PARAM_TYPE enum for supported data types.
 * @param param_count Total number of onboard parameters
 * @param param_index Index of this onboard parameter
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mace_msg_param_value_pack(uint8_t system_id, uint8_t component_id, mace_message_t* msg,
                               const char *param_id, float param_value, uint8_t param_type, uint16_t param_count, uint16_t param_index)
{
#if MACE_NEED_BYTE_SWAP || !MACE_ALIGNED_FIELDS
    char buf[MACE_MSG_ID_PARAM_VALUE_LEN];
    _mace_put_float(buf, 0, param_value);
    _mace_put_uint16_t(buf, 4, param_count);
    _mace_put_uint16_t(buf, 6, param_index);
    _mace_put_uint8_t(buf, 24, param_type);
    _mace_put_char_array(buf, 8, param_id, 16);
        memcpy(_MACE_PAYLOAD_NON_CONST(msg), buf, MACE_MSG_ID_PARAM_VALUE_LEN);
#else
    mace_param_value_t packet;
    packet.param_value = param_value;
    packet.param_count = param_count;
    packet.param_index = param_index;
    packet.param_type = param_type;
    mace_array_memcpy(packet.param_id, param_id, sizeof(char)*16);
        memcpy(_MACE_PAYLOAD_NON_CONST(msg), &packet, MACE_MSG_ID_PARAM_VALUE_LEN);
#endif

    msg->msgid = MACE_MSG_ID_PARAM_VALUE;
    return mace_finalize_message(msg, system_id, component_id, MACE_MSG_ID_PARAM_VALUE_MIN_LEN, MACE_MSG_ID_PARAM_VALUE_LEN, MACE_MSG_ID_PARAM_VALUE_CRC);
}

/**
 * @brief Pack a param_value message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param param_id Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
 * @param param_value Onboard parameter value
 * @param param_type Onboard parameter type: see the UXV_PARAM_TYPE enum for supported data types.
 * @param param_count Total number of onboard parameters
 * @param param_index Index of this onboard parameter
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mace_msg_param_value_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mace_message_t* msg,
                                   const char *param_id,float param_value,uint8_t param_type,uint16_t param_count,uint16_t param_index)
{
#if MACE_NEED_BYTE_SWAP || !MACE_ALIGNED_FIELDS
    char buf[MACE_MSG_ID_PARAM_VALUE_LEN];
    _mace_put_float(buf, 0, param_value);
    _mace_put_uint16_t(buf, 4, param_count);
    _mace_put_uint16_t(buf, 6, param_index);
    _mace_put_uint8_t(buf, 24, param_type);
    _mace_put_char_array(buf, 8, param_id, 16);
        memcpy(_MACE_PAYLOAD_NON_CONST(msg), buf, MACE_MSG_ID_PARAM_VALUE_LEN);
#else
    mace_param_value_t packet;
    packet.param_value = param_value;
    packet.param_count = param_count;
    packet.param_index = param_index;
    packet.param_type = param_type;
    mace_array_memcpy(packet.param_id, param_id, sizeof(char)*16);
        memcpy(_MACE_PAYLOAD_NON_CONST(msg), &packet, MACE_MSG_ID_PARAM_VALUE_LEN);
#endif

    msg->msgid = MACE_MSG_ID_PARAM_VALUE;
    return mace_finalize_message_chan(msg, system_id, component_id, chan, MACE_MSG_ID_PARAM_VALUE_MIN_LEN, MACE_MSG_ID_PARAM_VALUE_LEN, MACE_MSG_ID_PARAM_VALUE_CRC);
}

/**
 * @brief Encode a param_value struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param param_value C-struct to read the message contents from
 */
static inline uint16_t mace_msg_param_value_encode(uint8_t system_id, uint8_t component_id, mace_message_t* msg, const mace_param_value_t* param_value)
{
    return mace_msg_param_value_pack(system_id, component_id, msg, param_value->param_id, param_value->param_value, param_value->param_type, param_value->param_count, param_value->param_index);
}

/**
 * @brief Encode a param_value struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param param_value C-struct to read the message contents from
 */
static inline uint16_t mace_msg_param_value_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mace_message_t* msg, const mace_param_value_t* param_value)
{
    return mace_msg_param_value_pack_chan(system_id, component_id, chan, msg, param_value->param_id, param_value->param_value, param_value->param_type, param_value->param_count, param_value->param_index);
}

/**
 * @brief Send a param_value message
 * @param chan MAVLink channel to send the message
 *
 * @param param_id Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
 * @param param_value Onboard parameter value
 * @param param_type Onboard parameter type: see the UXV_PARAM_TYPE enum for supported data types.
 * @param param_count Total number of onboard parameters
 * @param param_index Index of this onboard parameter
 */
#ifdef MACE_USE_CONVENIENCE_FUNCTIONS

static inline void mace_msg_param_value_send(mace_channel_t chan, const char *param_id, float param_value, uint8_t param_type, uint16_t param_count, uint16_t param_index)
{
#if MACE_NEED_BYTE_SWAP || !MACE_ALIGNED_FIELDS
    char buf[MACE_MSG_ID_PARAM_VALUE_LEN];
    _mace_put_float(buf, 0, param_value);
    _mace_put_uint16_t(buf, 4, param_count);
    _mace_put_uint16_t(buf, 6, param_index);
    _mace_put_uint8_t(buf, 24, param_type);
    _mace_put_char_array(buf, 8, param_id, 16);
    _mace_finalize_message_chan_send(chan, MACE_MSG_ID_PARAM_VALUE, buf, MACE_MSG_ID_PARAM_VALUE_MIN_LEN, MACE_MSG_ID_PARAM_VALUE_LEN, MACE_MSG_ID_PARAM_VALUE_CRC);
#else
    mace_param_value_t packet;
    packet.param_value = param_value;
    packet.param_count = param_count;
    packet.param_index = param_index;
    packet.param_type = param_type;
    mace_array_memcpy(packet.param_id, param_id, sizeof(char)*16);
    _mace_finalize_message_chan_send(chan, MACE_MSG_ID_PARAM_VALUE, (const char *)&packet, MACE_MSG_ID_PARAM_VALUE_MIN_LEN, MACE_MSG_ID_PARAM_VALUE_LEN, MACE_MSG_ID_PARAM_VALUE_CRC);
#endif
}

/**
 * @brief Send a param_value message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mace_msg_param_value_send_struct(mace_channel_t chan, const mace_param_value_t* param_value)
{
#if MACE_NEED_BYTE_SWAP || !MACE_ALIGNED_FIELDS
    mace_msg_param_value_send(chan, param_value->param_id, param_value->param_value, param_value->param_type, param_value->param_count, param_value->param_index);
#else
    _mace_finalize_message_chan_send(chan, MACE_MSG_ID_PARAM_VALUE, (const char *)param_value, MACE_MSG_ID_PARAM_VALUE_MIN_LEN, MACE_MSG_ID_PARAM_VALUE_LEN, MACE_MSG_ID_PARAM_VALUE_CRC);
#endif
}

#if MACE_MSG_ID_PARAM_VALUE_LEN <= MACE_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mace_message_t which is the size of a full mace message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mace_msg_param_value_send_buf(mace_message_t *msgbuf, mace_channel_t chan,  const char *param_id, float param_value, uint8_t param_type, uint16_t param_count, uint16_t param_index)
{
#if MACE_NEED_BYTE_SWAP || !MACE_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mace_put_float(buf, 0, param_value);
    _mace_put_uint16_t(buf, 4, param_count);
    _mace_put_uint16_t(buf, 6, param_index);
    _mace_put_uint8_t(buf, 24, param_type);
    _mace_put_char_array(buf, 8, param_id, 16);
    _mace_finalize_message_chan_send(chan, MACE_MSG_ID_PARAM_VALUE, buf, MACE_MSG_ID_PARAM_VALUE_MIN_LEN, MACE_MSG_ID_PARAM_VALUE_LEN, MACE_MSG_ID_PARAM_VALUE_CRC);
#else
    mace_param_value_t *packet = (mace_param_value_t *)msgbuf;
    packet->param_value = param_value;
    packet->param_count = param_count;
    packet->param_index = param_index;
    packet->param_type = param_type;
    mace_array_memcpy(packet->param_id, param_id, sizeof(char)*16);
    _mace_finalize_message_chan_send(chan, MACE_MSG_ID_PARAM_VALUE, (const char *)packet, MACE_MSG_ID_PARAM_VALUE_MIN_LEN, MACE_MSG_ID_PARAM_VALUE_LEN, MACE_MSG_ID_PARAM_VALUE_CRC);
#endif
}
#endif

#endif

// MESSAGE PARAM_VALUE UNPACKING


/**
 * @brief Get field param_id from param_value message
 *
 * @return Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
 */
static inline uint16_t mace_msg_param_value_get_param_id(const mace_message_t* msg, char *param_id)
{
    return _MACE_RETURN_char_array(msg, param_id, 16,  8);
}

/**
 * @brief Get field param_value from param_value message
 *
 * @return Onboard parameter value
 */
static inline float mace_msg_param_value_get_param_value(const mace_message_t* msg)
{
    return _MACE_RETURN_float(msg,  0);
}

/**
 * @brief Get field param_type from param_value message
 *
 * @return Onboard parameter type: see the UXV_PARAM_TYPE enum for supported data types.
 */
static inline uint8_t mace_msg_param_value_get_param_type(const mace_message_t* msg)
{
    return _MACE_RETURN_uint8_t(msg,  24);
}

/**
 * @brief Get field param_count from param_value message
 *
 * @return Total number of onboard parameters
 */
static inline uint16_t mace_msg_param_value_get_param_count(const mace_message_t* msg)
{
    return _MACE_RETURN_uint16_t(msg,  4);
}

/**
 * @brief Get field param_index from param_value message
 *
 * @return Index of this onboard parameter
 */
static inline uint16_t mace_msg_param_value_get_param_index(const mace_message_t* msg)
{
    return _MACE_RETURN_uint16_t(msg,  6);
}

/**
 * @brief Decode a param_value message into a struct
 *
 * @param msg The message to decode
 * @param param_value C-struct to decode the message contents into
 */
static inline void mace_msg_param_value_decode(const mace_message_t* msg, mace_param_value_t* param_value)
{
#if MACE_NEED_BYTE_SWAP || !MACE_ALIGNED_FIELDS
    param_value->param_value = mace_msg_param_value_get_param_value(msg);
    param_value->param_count = mace_msg_param_value_get_param_count(msg);
    param_value->param_index = mace_msg_param_value_get_param_index(msg);
    mace_msg_param_value_get_param_id(msg, param_value->param_id);
    param_value->param_type = mace_msg_param_value_get_param_type(msg);
#else
        uint8_t len = msg->len < MACE_MSG_ID_PARAM_VALUE_LEN? msg->len : MACE_MSG_ID_PARAM_VALUE_LEN;
        memset(param_value, 0, MACE_MSG_ID_PARAM_VALUE_LEN);
    memcpy(param_value, _MACE_PAYLOAD(msg), len);
#endif
}

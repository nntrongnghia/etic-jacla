 /*
 * rak3172_lorawan.h
 *
 *  Copyright (C) Daniel Kampert, 2022
 *	Website: www.kampis-elektroecke.de
 *  File info: RAK3172 driver for ESP32.

  GNU GENERAL PUBLIC LICENSE:
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.

  Errors and commissions should be reported to DanielKampert@kampis-elektroecke.de
 */

#ifndef RAK3172_LORAWAN_H_
#define RAK3172_LORAWAN_H_

#include "Definitions/rak3172_defs.h"
#include "Definitions/rak3172_errors.h"

/** @brief          Initialize the RAK3172 SoM in LoRaWAN mode.
 *                  NOTE: You must call RAK3172_Init first!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param TxPwr    Tx power in dB
 *  @param Retries  Number of confirmed payload retransmissions
 *  @param JoinMode LoRaWAN join mode
 *  @param p_Key1   Pointer to key 1
 *                  OTAA: DEVEUI (8 Bytes)
 *                  ABP: APPSKEY (16 bytes)
 *  @param p_Key2   Pointer to key 2
 *                  OTAA: APPEUI (16 Bytes)
 *                  ABP: NWKSKEY (16 bytes)
 *  @param p_Key3   Pointer to key 3
 *                  OTAA: APPKEY (8 Bytes)
 *                  ABP: DEVADDR (4 bytes)
 *  @param Band     LoRa frequency band
 *  @param Subband  (Optional) LoRa sub band
 *                  NOTE: Only needed when US915, AU915 or CN470 band is used. Otherwise set it to RAK_SUB_BAND_NONE!
 *  @param Class    Device class (A, B or C)
 *  @param UseADR   (Optional) Enable adaptive data rate
 *  @param Timeout  (Optional) Timeout for the device reset in seconds
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_Init_LoRaWAN(RAK3172_t* p_Device, uint8_t TxPwr, uint8_t Retries, RAK3172_JoinMode_t JoinMode, const uint8_t* p_Key1, const uint8_t* p_Key2, const uint8_t* p_Key3, char Class, RAK3172_Band_t Band, RAK3172_SubBand_t Subband = RAK_SUB_BAND_NONE, bool UseADR = true, uint32_t Timeout = 10);

/** @brief              Set the keys for OTAA mode.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_DEVEUI     Pointer to LoRaWAN DEVEUI (8 Bytes)
 *  @param p_APPEUI     Pointer to LoRaWAN APPEUI (8 Bytes)
 *  @param p_APPKEY     Pointer to LoRaWAN APPKEY (16 Bytes)
 *  @return             RAK3172_OK when successful
 *                      RAK3172_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_SetOTAAKeys(RAK3172_t* p_Device, const uint8_t* p_DEVEUI, const uint8_t* p_APPEUI, const uint8_t* p_APPKEY);

/** @brief              Set the keys for ABP mode.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_APPSKEY    Pointer to LoRaWAN APPSKEY (16 Bytes)
 *  @param p_NWKSKEY    Pointer to LoRaWAN NWKSKEY (16 Bytes)
 *  @param p_DEVADDR    Pointer to LoRaWAN DEVADDR (4 Bytes)
 *  @return             RAK3172_OK when successful
 *                      RAK3172_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_SetABPKeys(RAK3172_t* p_Device, const uint8_t* p_APPSKEY, const uint8_t* p_NWKSKEY, const uint8_t* p_DEVADDR);

/** @brief                  Start the joining process.
 *                          NOTE: This is a blocking function!
 *  @param p_Device         Pointer to RAK3172 device object
 *  @param Timeout          (Optional) Timeout in seconds
 *                          NOTE: Set to 0 to disable the timeout function
 *  @param Attempts         (Optional) No. of join attempts
 *  @param EnableAutoJoin   (Optional) Enable auto join after power up
 *  @param Interval         (Optional) Reattempt interval
 *  @param on_Wait          (Optional) Hook for a custom wait function
 *                          NOTE: The function call is time critical. Prevent long wait periods!
 *  @return                 RAK3172_OK when joined
 *                          RAK3172_FAIL when a transmission error occurs
 *                          RAK3172_TIMEOUT when the number of JOIN attemps has expired
 *                          RAK3172_INVALID_ARG when an invalid argument was passed
 */
RAK3172_Error_t RAK3172_StartJoin(RAK3172_t* p_Device, uint32_t Timeout = 0, uint8_t Attempts = 0, bool EnableAutoJoin = false, uint8_t Interval = 10, RAK3172_Wait_t on_Wait = NULL);

/** @brief          Stop the joining process.
 *  @param p_Device Pointer to RAK3172 device object
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_StopJoin(RAK3172_t* p_Device);

/** @brief          Check if the module has joined the network.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Joined Pointer to joining state. #true when joined
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_FAIL when a transmission error occurs
 */
RAK3172_Error_t RAK3172_Joined(RAK3172_t* p_Device, bool* p_Joined);

/** @brief          Start a LoRaWAN data transmission.
 *                  NOTE: This is a blocking function!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Port     LoRaWAN port
 *  @param p_Buffer Pointer to data buffer
 *  @param Length   Length of buffer
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_STATE when the device is busy
 *                  RAK3172_FAIL when a transmission error occurs
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_INVALID_RESPONSE when a send confirmation failed or when the device is busy
 *                  RAK3172_TIMEOUT when a transmit timeout occurs
 *                  RAK3172_INVALID_STATE when the device is not joined
 */
RAK3172_Error_t RAK3172_LoRaWAN_Transmit(RAK3172_t* p_Device, uint8_t Port, const uint8_t* p_Buffer, uint8_t Length);

/** @brief              Start a LoRaWAN data transmission.
 *                      NOTE: This is a blocking function!
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param Port         LoRaWAN port
 *  @param p_Buffer     Pointer to data buffer
 *  @param Length       Length of buffer
 *  @param Timeout      Receive timeout in seconds
 *  @param Confirmed    (Optional) Enable message confirmation
 *                      NOTE: Only neccessary when payload is greater than 1024 bytes (long payload)
 *  @param Wait         (Optional) Hook for a custom wait function
 *                      NOTE: The function call is time critical. Prevent long wait periods!
 *  @return             RAK3172_OK when successful
 *                      RAK3172_INVALID_STATE when the device is busy
 *                      RAK3172_FAIL when a transmission error occurs
 *                      RAK3172_INVALID_ARG when an invalid argument is passed into the function
 *                      RAK3172_INVALID_RESPONSE when a send confirmation failed or when the device is busy
 *                      RAK3172_TIMEOUT when a transmit timeout occurs
 *                      RAK3172_INVALID_STATE when the device is not joined
 */
RAK3172_Error_t RAK3172_LoRaWAN_Transmit(RAK3172_t* p_Device, uint8_t Port, const void* p_Buffer, uint8_t Length, uint32_t Timeout, bool Confirmed = false, RAK3172_Wait_t Wait = NULL);

/** @brief          Read an incomming LoRaWAN downlink message from the RAK3172.
 *                  NOTE: This is a blocking function!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Payload  Pointer to payload string
 *  @param p_RSSI   (Optional) Pointer to message RSSI value
 *  @param p_SNR    (Optional) Pointer to message SNR value
 *  @param Timeout  (Optional) Receive timeout in seconds (must be greater than 1)
 *  @return         RAK3172_OK when successful
 *                  RAK3172_TIMEOUT when a receive timeout occurs
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_INVALID_STATE when the device is not joined
 */
RAK3172_Error_t RAK3172_LoRaWAN_Receive(RAK3172_t* p_Device, std::string* p_Payload, int* p_RSSI = NULL, int* p_SNR = NULL, uint32_t Timeout = 3);

/** @brief          Set the number of confirmed payload retransmissions.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Retries  Number of retries
 *                  NOTE: Only values between 0 and 7 are allowed!
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_SetRetries(RAK3172_t* p_Device, uint8_t Retries);

/** @brief              Get the number of confirmed payload retransmissions.
 *  @param p_Device     Pointer to RAK3172 device object
 *  @param p_Retries    Pointer to number of retries
 *  @return             RAK3172_OK when successful
 *                      RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_GetRetries(RAK3172_t* p_Device, uint8_t* p_Retries);

/** @brief          Enable / Disable the usage of the public network mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Enable   Enable / Disable the mode
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_SetPNM(RAK3172_t* p_Device, bool Enable);

/** @brief          Get the status of the public network mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to public network mode status
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_GetPNM(RAK3172_t* p_Device, bool* p_Enable);

/** @brief          Enable / Disable the usage of the confirmation mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Enable   Enable / Disable the mode
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_SetConfirmation(RAK3172_t* p_Device, bool Enable);

/** @brief          Get the current state of the transmission confirmation mode.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to confirmation mode
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_GetConfirmation(RAK3172_t* p_Device, bool* p_Enable);

/** @brief          Set the used frequency band.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Pointer to frequency band
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_SetBand(RAK3172_t* p_Device, RAK3172_Band_t Band);

/** @brief          Get the used frequency band.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Band   Pointer to frequency band
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_GetBand(RAK3172_t* p_Device, RAK3172_Band_t* p_Band);

/** @brief          Set the sub band for the LoRaWAN communication.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Target sub band
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_INVALID_RESPONSE when the device is operating in the wrong frequency band
 */
RAK3172_Error_t RAK3172_SetSubBand(RAK3172_t* p_Device, RAK3172_SubBand_t Band);

/** @brief          Get the sub band for the LoRaWAN communication.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Band     Pointer to frequency sub band
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_GetSubBand(RAK3172_t* p_Device, RAK3172_SubBand_t* p_Band);

/** @brief          Set the Tx power of the RAK3172.
 *                  NOTE: The index of the Tx power and the resulting power depends on the selected region!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param TxPwr    Tx power index in dBm
 *                  NOTE: The power must be a power of two!
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_SetTxPwr(RAK3172_t* p_Device, uint8_t TxPwr);

/** @brief          Set the RX1 delay of the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Delay    RX1 delay in ms
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_SetRX1Delay(RAK3172_t* p_Device, uint16_t Delay);

/** @brief          Set the RX2 delay of the device.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Delay    RX1 delay in ms
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_SetRX2Delay(RAK3172_t* p_Device, uint16_t Delay);

/** @brief          Get the SNR value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_SNR    Pointer to SNR value
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_GetSNR(RAK3172_t* p_Device, uint8_t* p_SNR);

/** @brief          Get the RSSI value of the last packet.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_RSSI   Pointer to RSSI value
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 */
RAK3172_Error_t RAK3172_GetRSSI(RAK3172_t* p_Device, int8_t* p_RSSI);

/** @brief          Get the duty time.
 *                  NOTE: This can only be used when using EU868, RU864 or EU433 frequency band!
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Duty   Pointer to duty cycle time im seconds
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument is passed into the function
 *                  RAK3172_INVALID_RESPONSE when the device is operating in the wrong frequency band
 */
RAK3172_Error_t RAK3172_GetDuty(RAK3172_t* p_Device, uint8_t* p_Duty);

/** @brief          Set the data rate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param DR       Data rate
 *  @return         ESP_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_SetDataRate(RAK3172_t* p_Device, RAK3172_DataRate_t DR);

/** @brief          Get the data rate of the LoRa module.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_DR     Pointer to data rate
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetDataRate(RAK3172_t* p_Device, RAK3172_DataRate_t* p_DR);

/** @brief          Enable / Disable the adaptive data rate.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param Enable   Pointer to data rate
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_SetADR(RAK3172_t* p_Device, bool Enable);

/** @brief          Get the status of the adaptive data rate option.
 *  @param p_Device Pointer to RAK3172 device object
 *  @param p_Enable Pointer to adaptive data rate status
 *  @return         RAK3172_OK when successful
 *                  RAK3172_INVALID_ARG when an invalid argument was passed
 *                  RAK3172_INVALID_STATE the when the interface is not initialized
 */
RAK3172_Error_t RAK3172_GetADR(RAK3172_t* p_Device, bool* p_Enable);

#endif /* RAK3172_LORAWAN_H_ */
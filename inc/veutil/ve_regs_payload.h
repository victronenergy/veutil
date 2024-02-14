#pragma once

/* NOTE: DO NOT DEFINE NEW CONSTANTS IN THIS FILE! THIS FILE IS A COPY! */

/*
 * VE_REG_BMS_ERROR - 0x2101
 * @note Bit 1-27 of VE_REG_LYNX_ION_BMS_ERROR_FLAGS maps to value 1-27 of VE_REG_BMS_ERROR.
 * un8 error
 */
#define VE_VDATA_BMS_ERROR_NONE 					0
#define VE_VDATA_BMS_ERROR_BATTERY_INIT				1
#define VE_VDATA_BMS_ERROR_NO_BATTERY_FOUND			2
#define VE_VDATA_BMS_ERROR_UNKNOWN_PRODUCT			3
#define VE_VDATA_BMS_ERROR_BAT_TYPE					4
#define VE_VDATA_BMS_ERROR_NR_OF_BAT				5
#define VE_VDATA_BMS_ERROR_NO_SHUNT_FND				6
#define VE_VDATA_BMS_ERROR_MEASURE					7
#define VE_VDATA_BMS_ERROR_CALCULATE				8
#define VE_VDATA_BMS_ERROR_BAT_NR_SER				9
#define VE_VDATA_BMS_ERROR_BAT_NR					10
#define VE_VDATA_BMS_ERROR_HARDWARE_FAILURE			11
#define VE_VDATA_BMS_ERROR_WATCHDOG					12
#define VE_VDATA_BMS_ERROR_OVER_VOLTAGE				13
#define VE_VDATA_BMS_ERROR_UNDER_VOLTAGE			14
#define VE_VDATA_BMS_ERROR_OVER_TEMPERATURE			15
#define VE_VDATA_BMS_ERROR_UNDER_TEMPERATURE		16
#define VE_VDATA_BMS_ERROR_IO_EXPANDER				17
#define VE_VDATA_BMS_ERROR_UNDER_CHARGE_STANDBY		18
#define VE_VDATA_BMS_ERROR_RESERVED_19				19
#define VE_VDATA_BMS_ERROR_CONTACTOR_STARTUP		20
#define VE_VDATA_BMS_ERROR_RESERVED_21				21
#define VE_VDATA_BMS_ERROR_ADC_FAILURE				22
#define VE_VDATA_BMS_ERROR_SLAVE_FAILURE			23
#define VE_VDATA_BMS_ERROR_RESERVED_24				24
#define VE_VDATA_BMS_ERROR_PRE_CHARGE				25
#define VE_VDATA_BMS_ERROR_CONTACTOR				26
#define VE_VDATA_BMS_ERROR_RESERVED_27				27
#define VE_VDATA_BMS_ERROR_SLAVE_UPDATE				28
#define VE_VDATA_BMS_ERROR_SLAVE_UPDATE_UNAVAILABLE	29
#define VE_VDATA_BMS_ERROR_CALIBRATION_DATA_LOST_OLD	30 // Deprecated; use #116 instead
#define VE_VDATA_BMS_ERROR_SETTINGS_DATA_INVALID_OLD	31 // Deprecated; use #119 instead
#define VE_VDATA_BMS_ERROR_BMS_CABLE				32 // BMS cable error
#define VE_VDATA_BMS_ERROR_REF_VOLTAGE_FAILURE		33 // Reference voltage failure
#define VE_VDATA_BMS_ERROR_WRONG_SYSTEM_VOLTAGE		34
#define VE_VDATA_BMS_ERROR_PRE_CHARGE_TIMEOUT		35
#define VE_VDATA_BMS_ERROR_ATC_ATD_FAILURE			36
#define VE_VDATA_BMS_ERROR_INTERLOCK				37 // Use by MGE
#define VE_VDATA_BMS_ERROR_EMERGENCY_STOP			38 // Use by MGE
#define VE_VDATA_BMS_ERROR_COMMUNICATION_TIMEOUT	39 // Use by MGE
#define VE_VDATA_BMS_ERROR_SAFETY_LOCK				40 // Use by MGE
#define VE_VDATA_BMS_ERROR_TERMINAL_OVER_TEMPERATURE	41 // Use by MGE
#define VE_VDATA_BMS_ERROR_RESERVED_101				101 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_102				102 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_103				103 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_104				104 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_105				105 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_106				106 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_107				107 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_108				108 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_109				109 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_110				110 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_111				111 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_112				112 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_113				113 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_CALIBRATION_DATA_LOST	116 // Non-volatile calibration data lost (same as charger error)
#define VE_VDATA_BMS_ERROR_SETTINGS_DATA_INVALID	119 // Non-volatile settings data invalid/corrupted (same as charger error)
#define VE_VDATA_BMS_ERROR_RESERVED_201				201 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_202				202 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_203				203 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_204				204 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_205				205 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_206				206 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_207				207 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_208				208 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_209				209 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_210				210 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_211				211 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_212				212 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_213				213 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_214				214 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_215				215 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_216				216 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_217				217 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_218				218 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_219				219 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_220				220 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_221				221 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_222				222 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_223				223 // Do not use; taken by Lynx Smart BMS
#define VE_VDATA_BMS_ERROR_RESERVED_224				224 // Do not use; taken by Lynx Smart BMS

/* NOTE: DO NOT DEFINE NEW CONSTANTS IN THIS FILE! THIS FILE IS A COPY! */

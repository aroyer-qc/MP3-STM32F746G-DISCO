/**
******************************************************************************
* @file    audio_conf.h
* @author  MCD Application Team
* @brief   Common header file for all audio processing.
******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */


#ifndef __AUDIO_CONF_H_
#define __AUDIO_CONF_H_

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"

/*+++++++++++++++++++++ ENCODER +++++++++++++++++++++++++++++++++++*/
/*-- Audio Codecs --*/
//#define WAV_ENC_CHOSEN                /* To uncomment to encode in WAV */

/*++++++++++++++++++++ DECODER +++++++++++++++++++++++++++++++++++*/

/*-- Audio Codecs --*/
#define __WAV_DECODER__
#define __MP3_DECODER__

/*++++++++++++++++++++++++++++++++++++++*/

/** @addtogroup AUDIO_CONF_Utilities
* @{
*/

/** @defgroup AUDIO_CONF
* @brief This file is the Header file for audio processor
* @{
*/


/** @defgroup AUDIO_CONF_Exported_Defines
* @{
*/
/*File naming definitions*/
#define FILEMGR_LIST_DEPDTH                        120
#define FILEMGR_FILE_NAME_SIZE                     40
#define FILEMGR_FULL_PATH_SIZE                     256

#define FILEMGR_MAX_LEVEL                          4
#define FILETYPE_DIR                               0
#define FILETYPE_FILE                              1


#define CHAR(x) ((x) * (((sFONT *)BSP_LCD_GetFont())->Width))

/* Sample rate frequency (IN)*/
#define AUDIO_IN_FREQ_ADPT               DEFAULT_AUDIO_IN_FREQ  /*I2S_AUDIOFREQ_16K*/
#define SAMPLE_RATE_FREQ                 AUDIO_IN_FREQ_ADPT

/**
* @}
*/


/** @defgroup AUDIO_CONF_Exported_TypesDefinitions
* @{
*/
typedef enum {
  AUDIO_ERROR_NONE = 0,
  AUDIO_ERROR_IO,
  AUDIO_ERROR_EOF,
  AUDIO_ERROR_INVALID_VALUE,
  AUDIO_ERROR_CODEC,
  AUDIO_ERROR_SRC,      /*for player only*/
}AUDIO_ErrorTypeDef;


typedef enum {
  AUDIO_STATE_IDLE = 0,
  AUDIO_STATE_WAIT,
  AUDIO_STATE_INIT,
  AUDIO_STATE_START,
  AUDIO_STATE_STOP,
  AUDIO_STATE_PLAY,
  AUDIO_STATE_PRERECORD,      /*for rec only*/
  AUDIO_STATE_RECORD,         /*for rec only*/
  AUDIO_STATE_NEXT,           /*for player only*/
  AUDIO_STATE_PREVIOUS,       /*for player only*/
  /*AUDIO_STATE_FORWARD,
  AUDIO_STATE_BACKWARD,*/
  AUDIO_STATE_PAUSE,
  AUDIO_STATE_RESUME,
  AUDIO_STATE_VOLUME_UP,
  AUDIO_STATE_VOLUME_DOWN,
  AUDIO_STATE_BACKMENU,
  AUDIO_STATE_ERROR,
}AUDIO_PROC_StateTypeDef;


/* Callback functions type*/
typedef uint32_t (fnReadCallback_TypeDef)(
                                          void *       pCompressedData,             /* [OUT] Pointer to buffer to fill with coded MP3 data */
                                          uint32_t     nDataSizeInBytes,            /* Buffer size in Bytes */
                                          void *       pUser                        /* User pointer: can be used to pass additional parameters */
                                            );

typedef void (fnXFerCpltCallback_TypeDef)( uint8_t Direction,
                                          uint8_t** pbuf,
                                          uint32_t* pSize);

typedef uint32_t (fnSetPositionCallback_TypeDef)( uint32_t Pos);


/**
* @}
*/



/** @defgroup AUDIO_CONF_Exported_Macros
* @{
*/
#define BYTE_0(val)                    (uint8_t)(val & 0xFF)
#define BYTE_1(val)                    (uint8_t)((val & 0xFF00) >> 8)
#define BYTE_2(val)                    (uint8_t)((val & 0xFF0000) >> 16)
#define BYTE_3(val)                    (uint8_t)((val & 0xFF000000) >> 24)

/**
* @}
*/

/** @defgroup AUDIO_CONF_Exported_Variables
* @{
*/

/**
* @}
*/

/** @defgroup AUDIO_CONF_Exported_Functions
* @{
*/

/**
* @}
*/

#endif  /* __AUDIO_CONF_H_ */
/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

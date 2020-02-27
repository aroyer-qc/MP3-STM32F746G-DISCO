/**
******************************************************************************
* @file    player.h
* @author  MCD Application Team
* @brief   Header for audioplayer.c module.
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLAYER_H
#define __PLAYER_H

/* Includes ------------------------------------------------------------------*/
#include "lcd_log.h"
#include "ff.h" /* File System */

#include "audio_fw_glo.h"
#include "src236_glo.h"
#include "src441_glo.h"

#include "global_includes.h"
#include "audio_conf.h"

#include "fwkmempool.h"
#include "songutilities.h"
#include "coders.h"

/* Exported Defines ----------------------------------------------------------*/
/*Max Audio decoder packet buffer size (MP3,WAV...)*/
#define MAX_OUT_PACKET_SZE                        MAX_OUT_DEFAULT_PACKET_SZE     /* Maximum usage in bytes vs supported frame format*/

/*Max Audio header information buffer size (MP3,WAV...)*/
#define MAX_AUDIO_HEADER_SIZE                     MAX_AUDIO_DEFAULT_HEADER_SIZE  /* Maximum usage in bytes*/

/*output double buffer stream to headphones*/
#define AUDIO_OUT_BUFFER_SIZE                    (uint32_t)(WAV_PACKET_SZE*2)    /* (480*2*2*2): Output double buffer of 960 stereo samples in 16-bits representing 10ms frame at 48kHz
as (480 * 2 (half + half) * 2 (stereo) * 2 (bytes x sample)) in bytes*/

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum {
  BUFFER_OFFSET_NONE = 0, /*empty*/
  BUFFER_OFFSET_HALF,
  BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;

/* Audio buffer control struct */
typedef struct {
  uint8_t buff[AUDIO_OUT_BUFFER_SIZE];
  BUFFER_StateTypeDef state;   /* empty (none), half, full*/
  uint32_t fptr;
}AUDIO_OUT_BufferTypeDef;

/* Generic type containing main Audio format info */
typedef struct {
  uint32_t FileSize;       /*in bytes*/
  uint32_t FileFormat;     /* '3':MP3; 'V':WAV*/
  uint32_t AudioFormat;    /* 'P':PCM'*/
  uint16_t NbrChannels;
  uint32_t SampleRate;
  uint32_t ByteRate;
  uint16_t BitPerSample;
}Audio_InfoTypeDef;

typedef enum {
  AUDIO_CFG_NONE,
  AUDIO_CFG_VOLUME_UP,
  AUDIO_CFG_VOLUME_DOWN,
  AUDIO_CFG_EQ1_UP,
  AUDIO_CFG_EQ2_UP,
  AUDIO_CFG_EQ3_UP,
  AUDIO_CFG_EQ4_UP,
  AUDIO_CFG_EQ5_UP,
  AUDIO_CFG_EQ1_DOWN,
  AUDIO_CFG_EQ2_DOWN,
  AUDIO_CFG_EQ3_DOWN,
  AUDIO_CFG_EQ4_DOWN,
  AUDIO_CFG_EQ5_DOWN,
  AUDIO_CFG_ERROR,
}AUDIO_PLAYBACK_CfgTypeDef;

/*-----------------------------*/

typedef struct _FILELIST_LineTypeDef {
  uint8_t type;
  uint8_t name[FILEMGR_FILE_NAME_SIZE];
}FILELIST_LineTypeDef;

typedef struct _FILELIST_FileTypeDef {
  FILELIST_LineTypeDef  file[FILEMGR_LIST_DEPDTH] ;
  uint16_t              ptr;
}FILELIST_FileTypeDef;

/* Exported macro ------------------------------------------------------------*/
extern AUDIO_PROC_StateTypeDef AudioState;
extern FILELIST_FileTypeDef FileList;

#ifdef __WAV_DECODER__
extern WAVE_FormatTypeDef DecWavFmtStruct;
#endif

#ifdef __MP3_DECODER__
extern TSpiritMP3Info  mp3_Info;
#endif

/* Exported functions ------------------------------------------------------- */
AUDIO_ErrorTypeDef PLAYER_Init(void);
AUDIO_ErrorTypeDef PLAYER_Start(uint8_t idx);
AUDIO_ErrorTypeDef PLAYER_Process(void);
AUDIO_ErrorTypeDef PLAYER_Stop(void);

uint32_t PLAYER_GetTimeInfo(uint32_t* Length, uint32_t* Elapsed);
uint32_t PLAYER_GetFileLength(void);

AUDIO_ErrorTypeDef UsbToMemPool(void);
uint16_t AUDIO_GetFilObjectNumber(void);
uint32_t PlayerSetPosition(uint32_t Pos);

/* Callback functions*/
void SWI_EXTI0_Callback(void);    /* fill MemPool Callback in first loop*/
unsigned int Dec_ReadDataCallback(void* pCompressedData, uint32_t nDataSizeInChars, void* pUserData);

#endif /* __PLAYER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

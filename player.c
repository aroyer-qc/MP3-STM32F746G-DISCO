/**
******************************************************************************
* @file    player.c
* @author  MCD Application Team
* @brief   This file provides the Audio Out (playback) interface API
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


/* Includes ------------------------------------------------------------------*/
#include "player.h"
#include "id3v2lib.h"

/* Private define ------------------------------------------------------------*/

/* LCD  RK043FN48H : (X*Y):480*272 pixels */

#define TOUCH_NEXT_XMIN         325
#define TOUCH_NEXT_XMAX         365
#define TOUCH_NEXT_YMIN         212
#define TOUCH_NEXT_YMAX         252

#define TOUCH_PREVIOUS_XMIN     250
#define TOUCH_PREVIOUS_XMAX     290
#define TOUCH_PREVIOUS_YMIN     212
#define TOUCH_PREVIOUS_YMAX     252

#define TOUCH_STOP_XMIN         170
#define TOUCH_STOP_XMAX         210
#define TOUCH_STOP_YMIN         212
#define TOUCH_STOP_YMAX         252

#define TOUCH_PAUSE_XMIN        100
#define TOUCH_PAUSE_XMAX        135
#define TOUCH_PAUSE_YMIN        212
#define TOUCH_PAUSE_YMAX        252
#define TOUCH_PAUSE_XSPACE      20

#define TOUCH_VOL_MINUS_XMIN    20
#define TOUCH_VOL_MINUS_XMAX    71
#define TOUCH_VOL_MINUS_YMIN    212
#define TOUCH_VOL_MINUS_YMAX    252

#define TOUCH_VOL_XOFFSET       4
#define TOUCH_VOL_LINE          14

#define TOUCH_VOL_PLUS_XMIN     402
#define TOUCH_VOL_PLUS_XMAX     453
#define TOUCH_VOL_PLUS_YMIN     212
#define TOUCH_VOL_PLUS_YMAX     252

#define TOUCH_EQ1_PLUS_XMIN     330
#define TOUCH_EQ1_PLUS_XMAX     359
#define TOUCH_EQ1_PLUS_YMIN     50
#define TOUCH_EQ1_PLUS_YMAX     100

#define TOUCH_EQ1_MINUS_XMIN    330
#define TOUCH_EQ1_MINUS_XMAX    359
#define TOUCH_EQ1_MINUS_YMIN    110
#define TOUCH_EQ1_MINUS_YMAX    160

#define TOUCH_EQ2_PLUS_XMIN     360
#define TOUCH_EQ2_PLUS_XMAX     389
#define TOUCH_EQ2_PLUS_YMIN     50
#define TOUCH_EQ2_PLUS_YMAX     100

#define TOUCH_EQ2_MINUS_XMIN    360
#define TOUCH_EQ2_MINUS_XMAX    389
#define TOUCH_EQ2_MINUS_YMIN    110
#define TOUCH_EQ2_MINUS_YMAX    160

#define TOUCH_EQ3_PLUS_XMIN     390
#define TOUCH_EQ3_PLUS_XMAX     419
#define TOUCH_EQ3_PLUS_YMIN     50
#define TOUCH_EQ3_PLUS_YMAX     100

#define TOUCH_EQ3_MINUS_XMIN    390
#define TOUCH_EQ3_MINUS_XMAX    419
#define TOUCH_EQ3_MINUS_YMIN    110
#define TOUCH_EQ3_MINUS_YMAX    160

#define TOUCH_EQ4_PLUS_XMIN     420
#define TOUCH_EQ4_PLUS_XMAX     449
#define TOUCH_EQ4_PLUS_YMIN     50
#define TOUCH_EQ4_PLUS_YMAX     100

#define TOUCH_EQ4_MINUS_XMIN    420
#define TOUCH_EQ4_MINUS_XMAX    449
#define TOUCH_EQ4_MINUS_YMIN    110
#define TOUCH_EQ4_MINUS_YMAX    160

#define TOUCH_EQ5_PLUS_XMIN     450
#define TOUCH_EQ5_PLUS_XMAX     479
#define TOUCH_EQ5_PLUS_YMIN     50
#define TOUCH_EQ5_PLUS_YMAX     100

#define TOUCH_EQ5_MINUS_XMIN    450
#define TOUCH_EQ5_MINUS_XMAX    479
#define TOUCH_EQ5_MINUS_YMIN    110
#define TOUCH_EQ5_MINUS_YMAX    160

#define TOUCH_EQ_XOFFSET        4
#define TOUCH_EQ_MINUS_LINE     8
#define TOUCH_EQ_PLUS_LINE      4

#define PLAYER_CLEAR_XWIDTH       250
#define PLAYER_COUNT_TEXT_XMIN    263
#define PLAYER_COUNT_TEXT_YLINE   5

#define MSG_ERR_MOD_YLINE         15

/*SRC module selected*/
#define SELECT_SRC_NONE     0
#define SELECT_SRC_236      1
#define SELECT_SRC_441      2

/* MemPool size */
#define   MEMPOOLBUFSZE  (3*MAX_OUT_PACKET_SZE)

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/*Audio state variable*/
AUDIO_PLAYBACK_CfgTypeDef AudioCfgChange;

/* MemPool: Audio handler variables */
AUDIO_HANDLE_t hAudio; /*instance*/
AUDIO_HANDLE_t *hptr;  /*pointer on instance*/

/* data number read */
__IO uint32_t NumberOfData = 0;

uint32_t OutDecPacketSizeBytes = MAX_OUT_PACKET_SZE; /* in bytes */

/* Audio Decoder structure instance when present */
Decoder_TypeDef sDecoderStruct;
uint32_t decsize; /*returned output decoder size in bytes*/
uint32_t DecSampNbBytes = 2; /* 16b: nb bytes per sample */
uint32_t DecNumChannels = 2; /* 2: stereo channels*/

/* In buffer size for MemPool read */
int16_t AudioBufferSize;  /* SRC input buffer size in bytes*/
int32_t sizeAvailable;    /* MemPool available size in bytes*/

/* Time play information */
uint8_t time[30];
uint8_t time_tot[30];

int EQ[5] = {16,7,5,10,14};  /* default for the equalizer

/*Audio module parameters memory allocation*/
void *pSrc236PersistentMem;
void *pSrc441PersistentMem;
void *pSrcScratchMem;

/*--Debug IT check--*/
uint32_t  status_IT1=0;         /*loop 1 LowPriority*/
uint32_t  counter_IT1stpIT2=0;  /*loop 2 HighPriority*/

/* Input Audio File*/
FIL AudioFile;
static uint8_t  *FilePath; /*Audio file name */
static char FileExtension =' ';
static int16_t FilePos = 0;

/*Generic Audio Info*/
static Audio_InfoTypeDef AudioFormatData;
static __IO uint32_t uwVolumeRef = 30; /*80*/
static uint32_t PauseStatus = 0;

/* Graphic Player icons*/
static Point PlayPoints[] = {{TOUCH_STOP_XMIN, TOUCH_STOP_YMIN},
{TOUCH_STOP_XMAX, (TOUCH_STOP_YMIN+TOUCH_STOP_YMAX)/2},
{TOUCH_STOP_XMIN, TOUCH_STOP_YMAX}};
static Point NextPoints[] = {{TOUCH_NEXT_XMIN, TOUCH_NEXT_YMIN},
{TOUCH_NEXT_XMAX, (TOUCH_NEXT_YMIN+TOUCH_NEXT_YMAX)/2},
{TOUCH_NEXT_XMIN, TOUCH_NEXT_YMAX}};
static Point PreviousPoints[] = {{TOUCH_PREVIOUS_XMIN, (TOUCH_PREVIOUS_YMIN+TOUCH_PREVIOUS_YMAX)/2},
{TOUCH_PREVIOUS_XMAX, TOUCH_PREVIOUS_YMIN},
{TOUCH_PREVIOUS_XMAX, TOUCH_PREVIOUS_YMAX}};

/* Buffer used to store the audio file header */
static uint8_t tHeaderTmp[MAX_AUDIO_HEADER_SIZE];

/* MemPool Write: In Buffer to read audio file */
static int8_t Buffer1[MAX_OUT_PACKET_SZE];

/* MemPool Read: Out Buffer for Audio module processing */
static uint8_t WavToSrcTmpBuffer[SRC236_FRAME_SIZE_MAX*2*2]; /* SRC_FRAME_SIZE (240>147) * 2 (bytes x sample) * 2 (stereo) */

/*Double BUFFER for Output Audio stream*/
static AUDIO_OUT_BufferTypeDef  BufferCtl;  /* to the codec */

/* SRC: Sample Rate Converter variables */
static char SRC236_ratio[8][3]={" 2 "," 3 "," 6 ","1_2","1_3","1_6","3_2","2_3"};
static uint8_t SrcTypeSelected;     /* 236, 441, None */
static uint8_t SrcIterations;
static uint32_t AudioReadSize;    /* nr of bytes to retrieve from USB*/

/*BUFFERs for Audio Module interface*/
static  buffer_t InputBuffer;
static  buffer_t *pInputBuffer = &InputBuffer;
static  buffer_t OutputBuffer;
static  buffer_t *pOutputBuffer = &OutputBuffer;

/* Private function prototypes -----------------------------------------------*/
static AUDIO_ErrorTypeDef PlayerGetFileInfo(uint16_t file_idx, Audio_InfoTypeDef *info);

/* init functions */
static uint8_t PlayerStreamInit(uint32_t AudioFreq);  /* config SAI output Audio stream*/
static void SWI_EXTI0_Config(void);                   /* Loop1: EXT1 SWI used to fill MemPool in first decoder loop*/

/* internal DMA IT output Callback processing*/
static AUDIO_ErrorTypeDef AudioOutCallBackProcess(uint16_t offset); /* Loop2: DMA IT callback processing in output SRC loop*/

/* Internal Audio player functions*/
static void PlayerRecorderDisplayButtons(void);  /* display player buttons*/
static void AcquireTouchButtons(void);           /* acquire player buttons*/
static void PlayerDynamicCfg(void);              /* set dynamic parameters*/
static void PlayerDisplaySettings(void);         /* display specific audio module settings*/
static void PlayerRecorderDisplayEQ(void);
static AUDIO_ErrorTypeDef MemPoolToSrc(uint16_t offset); /* read MemPool and SRC audio processing*/

/* reset audio player buffers*/
static uint32_t ResetBuffers(void);
static uint32_t PLAYER_Close(void);

static void UpdateTimeInformation(void); /* update the audio file time information*/

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Initializes Audio Interface.
* @param  None
* @retval Audio error
*/
AUDIO_ErrorTypeDef PLAYER_Init(void)
{
  uint32_t src_scratch_mem_size;

  /* Allocat mem for SRC236 and SRC411 */
  pSrc236PersistentMem = malloc(src236_persistent_mem_size);  /* 0x1EC: 492  */
  pSrc441PersistentMem = malloc(src441_persistent_mem_size);  /* 0x0E8: 232 */
  if ((pSrc236PersistentMem == NULL) || (pSrc441PersistentMem == NULL))
  {
    free (pSrc236PersistentMem);
    pSrc236PersistentMem = NULL;
    free (pSrc441PersistentMem);
    pSrc441PersistentMem = NULL;
    while(1);
  }

  if (src236_scratch_mem_size > src441_scratch_mem_size)
  {
    src_scratch_mem_size = src236_scratch_mem_size;    /* 0x784:1924 */
  }
  else
  {
    src_scratch_mem_size = src441_scratch_mem_size;    /* 0xC9C: 3228*/
  }
  pSrcScratchMem = malloc(src_scratch_mem_size);

  if (pSrcScratchMem == NULL)
  {
    free (pSrcScratchMem);
    pSrcScratchMem = NULL;
    while(1);
  }
  /* MemPool: RAM dynamic memory allocation,
  Allocate SRAM memory MemPool structure */
  hAudio.pMemPool = (FWK_MEMPOOL_t*) malloc(sizeof(FWK_MEMPOOL_t)); /* 0x10: 16 */

  if (hAudio.pMemPool == NULL)
  {
    free(hAudio.pMemPool);
    hAudio.pMemPool = NULL;
    while(1);
  }
  /* Allocate SRAM memory for MemPool Buffer: at least 3 times Max output audio decoder packet */
  hAudio.pMemPool->pBuff = (uint8_t*) malloc(MEMPOOLBUFSZE);    /* 0x9000: 36864 */
  if(hAudio.pMemPool->pBuff == NULL)
  {
    free(hAudio.pMemPool->pBuff);
    hAudio.pMemPool->pBuff = NULL;
    while(1);
  }
  FWK_MEMPOOL_Init(hAudio.pMemPool, MEMPOOLBUFSZE);

  /* pointer on audio handler structure*/
  hptr = (AUDIO_HANDLE_t*)&hAudio;

  /* Configure EXTI in SWI mode: to fill MemPool with Buffer1 from Input Audio file */
  SWI_EXTI0_Config();

  /* Configure the audio peripherals: as output SAI (I2S mode) audio stream 48kHz to headphones*/
  if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, uwVolumeRef, I2S_AUDIOFREQ_48K) == 0)
  {
      return AUDIO_ERROR_NONE;
  }
  else
  {
    return AUDIO_ERROR_IO;
  }
}

/**
* @brief  Configures EXTI Line0 in interrupt SWI mode
*         used to fill MemPool in first loop with decoded data
* @param  None
* @retval None
*/
static void SWI_EXTI0_Config(void)
{
  /* Loop1: Unmask EXT line0 IT */
  EXTI->IMR =  EXTI_IMR_MR0;

  /* Enable and set EXTI Line0 Interrupt Preempt-priority:  Loop 1 has to be in lower priority (compared to DMA IT loop2 or LCD IT) */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}


/**
* @brief  Starts Audio streaming.
* @param  idx: File index
* @retval Audio error
*/
AUDIO_ErrorTypeDef PLAYER_Start(uint8_t idx)
{
  AUDIO_ErrorTypeDef error = AUDIO_ERROR_NONE;
  AUDIO_ErrorTypeDef error_tmp;

  src236_static_param_t src236_static_param;
  src441_static_param_t src441_static_param;
  uint32_t src_input_frame_size;
  uint32_t src_error;

  /* Enables and resets CRC-32 from STM32 HW */
  __HAL_RCC_CRC_CLK_ENABLE();
  CRC->CR = CRC_CR_RESET;

  /* Reinit and force close file*/
  //PLAYER_Close();

  if(AUDIO_GetFilObjectNumber() > idx)
  {
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(0, LINE(3), BSP_LCD_GetXSize() , LINE(1));

    /*-- Select decoder type and Get info from header --*/
    error_tmp=PlayerGetFileInfo(idx, &AudioFormatData);
    if(error_tmp != AUDIO_ERROR_NONE)
    {
      return AUDIO_ERROR_CODEC;
    }

    /* Input stereo audio only */
    if(AudioFormatData.NbrChannels != 2)
    {
      BSP_LCD_SetTextColor(LCD_COLOR_RED);
      BSP_LCD_DisplayStringAtLine(MSG_ERR_MOD_YLINE, (uint8_t*) "Error: only stereo audio input is supported.");
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

      f_close(&AudioFile);
      AudioState = AUDIO_STATE_STOP;
      return (AUDIO_ERROR_INVALID_VALUE);
    }

    /*Input frame size to read from MemPool before SRC processing to get 480 samples output*/
    switch(AudioFormatData.SampleRate)
    {
    case 8000:
      src236_static_param.src_mode = SRC236_RATIO_6;
      SrcTypeSelected = SELECT_SRC_236;
      SrcIterations = 1;
      src_input_frame_size = AUDIO_OUT_BUFFER_SIZE/(8*SrcIterations*6);
      break;
    case 16000:
      src236_static_param.src_mode = SRC236_RATIO_3;
      SrcTypeSelected = SELECT_SRC_236;
      SrcIterations = 1;
      src_input_frame_size = AUDIO_OUT_BUFFER_SIZE/(8*SrcIterations*3);
      break;
    case 24000:
      src236_static_param.src_mode = SRC236_RATIO_2;
      SrcTypeSelected = SELECT_SRC_236;
      SrcIterations = 1;
      src_input_frame_size = AUDIO_OUT_BUFFER_SIZE/(8*SrcIterations*2);
      break;
    case 32000:
      src236_static_param.src_mode = SRC236_RATIO_3_2;
      SrcTypeSelected = SELECT_SRC_236;
      SrcIterations = 2;   /* frame size smaller but processing repeated 2 times */
      src_input_frame_size = AUDIO_OUT_BUFFER_SIZE/(8*SrcIterations*3/2);
      break;
    case 44100:
      /* src441_static_param does not have params to be configured */
      SrcTypeSelected = SELECT_SRC_441;
      SrcIterations = 3;   /* frame size smaller but processing repeated 3 times */
      src_input_frame_size = (AUDIO_OUT_BUFFER_SIZE/480)*441/(8*SrcIterations);
      break;
    case 48000:
      SrcTypeSelected = SELECT_SRC_NONE;
      SrcIterations = 2;   /* frame size smaller but processing repeated 2 times considering SRC236 input req.*/
      src_input_frame_size = AUDIO_OUT_BUFFER_SIZE/(8*SrcIterations); /* half buff, stereo, byte x sample */
      break;
    case 96000:
      src236_static_param.src_mode = SRC236_RATIO_1_2;
      SrcTypeSelected = SELECT_SRC_236;
      SrcIterations = 4;  /* frame size smaller but processing repeated 4 times */
      src_input_frame_size = AUDIO_OUT_BUFFER_SIZE/(8*SrcIterations*1/2);
      break;
    default:
      BSP_LCD_SetTextColor(LCD_COLOR_RED);
      BSP_LCD_DisplayStringAtLine(MSG_ERR_MOD_YLINE,  (uint8_t*) "Error: This sample frequency is not supported.");
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      f_close(&AudioFile);
      AudioState = AUDIO_STATE_STOP;
      return (AUDIO_ERROR_INVALID_VALUE);
    }
    AudioBufferSize = src_input_frame_size*4; /* stereo & byte x sample */
    AudioReadSize = AudioBufferSize;          /* Buffer size in number of bytes used to read MemPool */

    if (SrcTypeSelected == SELECT_SRC_236)
    {
      /* SRC236 effect reset */
      src_error = src236_reset(pSrc236PersistentMem, pSrcScratchMem);
      if (src_error != SRC236_ERROR_NONE)
      {
        return (AUDIO_ERROR_SRC);
      }
      /* SRC236 effect static parameters setting */
      src_error = src236_setParam(&src236_static_param, pSrc236PersistentMem);
      if (src_error != SRC236_ERROR_NONE)
      {
        return (AUDIO_ERROR_SRC);
      }
    }

    if (SrcTypeSelected == SELECT_SRC_441)
    {
      /* SRC236 effect reset */
      src_error = src441_reset(pSrc441PersistentMem, pSrcScratchMem);
      if (src_error != SRC441_ERROR_NONE)
      {
        return (AUDIO_ERROR_SRC);
      }
      /* SRC236 effect static parameters setting */
      src_error = src441_setParam(&src441_static_param, pSrc441PersistentMem);
      if (src_error != SRC441_ERROR_NONE)
      {
        return (AUDIO_ERROR_SRC);
      }
    }

    /* Buffers used for audio module interface after MemPool */
    InputBuffer.data_ptr = &WavToSrcTmpBuffer;
    InputBuffer.nb_bytes_per_Sample = AudioFormatData.BitPerSample/8; /* 8 bits in 0ne byte */
    InputBuffer.nb_channels = AudioFormatData.NbrChannels;
    InputBuffer.mode = INTERLEAVED;
    InputBuffer.buffer_size = src_input_frame_size;                    /* in number of samples */

    OutputBuffer.nb_bytes_per_Sample = AudioFormatData.BitPerSample/8; /* 8 bits in 0ne byte */
    OutputBuffer.nb_channels = AudioFormatData.NbrChannels;
    OutputBuffer.mode = INTERLEAVED;
    OutputBuffer.buffer_size = AUDIO_OUT_BUFFER_SIZE/8; /* half buff of stereo samples in bytes */

    /* Adjust the Audio frequency codec is always 48Khz for this application*/
    if (PlayerStreamInit(I2S_AUDIOFREQ_48K) != 0)
    {
      return AUDIO_ERROR_CODEC;
    }

    /* Final Output double buffer structure */
    BufferCtl.state = BUFFER_OFFSET_NONE;
    /*eqv read file pointer on output */
    BufferCtl.fptr = 0;

    /* WAV format */
    if (FileExtension =='V') /*bypass header: only for Audio files WAV format*/
    {
      /* Get Data from USB Flash Disk and Remove Wave format header */
      f_lseek(&AudioFile, 44);
    }

    /*** Get Data from USB Flash Disk ***/

    /*++ Loop1 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* -- Write MemPool and Fill Buffer1 from Audio file at start
    (same code as callback processing of EXT0 line SWI) */
    UsbToMemPool();

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* -- Loop2: Process MemPool and Fill complete double output buffer at first
    time */
    error = MemPoolToSrc(0);
    if ( error == AUDIO_ERROR_NONE)
    {
      error = MemPoolToSrc(AUDIO_OUT_BUFFER_SIZE/2);
    }
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Loop1 (again if new available space) */
    UsbToMemPool();

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    /*  Play complete double output buffer at first time */
    if ( error == AUDIO_ERROR_NONE)
    {
      AudioState = AUDIO_STATE_PLAY;
      PlayerRecorderDisplayButtons();
      BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE+1), (uint8_t *)"[PLAY ]", LEFT_MODE);

      if(BufferCtl.fptr != 0)

      { /*DMA stream from output double buffer to codec in Circular mode launch*/
        BSP_AUDIO_OUT_Play((uint16_t*)&BufferCtl.buff[0], AUDIO_OUT_BUFFER_SIZE);

        // Reset previous EQ setting
        BSP_AUDIO_OUT_AdjustEQ(0, EQ[0]);
        BSP_AUDIO_OUT_AdjustEQ(1, EQ[1]);
        BSP_AUDIO_OUT_AdjustEQ(2, EQ[2]);
        BSP_AUDIO_OUT_AdjustEQ(3, EQ[3]);
        BSP_AUDIO_OUT_AdjustEQ(4, EQ[4]);

        BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_ON);

        for(int i = 0; i <uwVolumeRef; i++)
        {
            BSP_AUDIO_OUT_SetVolume(i);
        }

        return AUDIO_ERROR_NONE;
      }
    }
  }
  return AUDIO_ERROR_IO;
}

/**
* @brief  Manages Loop1 Audio process.
*         Write MemPool and Fill Buffer1 from Audio file
* @param  None
* @retval Audio error
*/
AUDIO_ErrorTypeDef UsbToMemPool(void)
{

  /*++ Loop1 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
  /* -- Write MemPool and Fill Buffer1 from Audio file at start (same code as callback processing of EXT0 line SWI) */

  FWK_MEMPOOL_GetAvailableSize(hptr->pMemPool, &sizeAvailable);
  while (sizeAvailable >= OutDecPacketSizeBytes)
  {
    /* Audio Decoder between Audio File (MST) and Buffer1 then MemPool writing */
    if (sDecoderStruct.Decoder_DecodeData != NULL) {
      DecSampNbBytes=AudioFormatData.BitPerSample/8;    /* 2 bytes*/
      DecNumChannels= AudioFormatData.NbrChannels;      /* stereo*/
      /*OutDecPacketSizeBytes: frame packet to decode (in bytes)*/

      decsize = sDecoderStruct.Decoder_DecodeData((__IO int16_t*)(&Buffer1[0]), (OutDecPacketSizeBytes/(DecNumChannels*DecSampNbBytes)), NULL);
    }

    if(decsize != 0)
    {
      /*write Buffer1 to MemPool*/
      FWK_MEMPOOL_Write(hptr->pMemPool, (uint8_t*)Buffer1, decsize); /* input buffer1 in bytes */
    }
    else /* End of file detected*/
    {
      /*PLAYER_Stop();
      break; */
    }
    /*update available size in MemPool*/
    FWK_MEMPOOL_GetAvailableSize(hptr->pMemPool, &sizeAvailable);
  }
  return AUDIO_ERROR_NONE;
}

/**
* @brief  Manages Audio process.
* @param  None
* @retval Audio error
*/
AUDIO_ErrorTypeDef PLAYER_Process(void)
{

  AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;
  uint8_t str[10];


  if((AudioState != AUDIO_STATE_PLAY) && (AudioState != AUDIO_STATE_START))
  {
      BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);
  }

  switch(AudioState)
  {

    /*Audio player state machine */
  case AUDIO_STATE_PLAY:

    if(AudioFile.fptr >= AudioFormatData.FileSize)
    {
      BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
      AudioState = AUDIO_STATE_NEXT;
    }

    /*--------------------------------------------------------------------*/
    /*-- Loop1: Compute sizeAvailable in MemPool and activate SWI for MemPool writing --*/

    /* Check if pool state is not full */
    if (FWK_MEMPOOL_GetState(hptr->pMemPool) != FWK_MEMPOOL_STATUS_FULL)
    {
      /* Check if pool state is not empty */
      if (FWK_MEMPOOL_GetState(hptr->pMemPool) == FWK_MEMPOOL_STATUS_EMPTY)
      {
        sizeAvailable = hptr->pMemPool->buffSze; /* Empty case*/
      }
      else
      {
        FWK_MEMPOOL_GetAvailableSize(hptr->pMemPool, &sizeAvailable); /* Other */
      }
    }
    else
    {
      sizeAvailable = 0; /* Full case */
    }
    /* MemPool writing from Buffer1 filled with audio file (Lower priority) */
    /* Callback: Loop while there is enough room in Pool_1 for decoding */
    if(sizeAvailable >= OutDecPacketSizeBytes)
    {
      /* activate SW interrupt on EXT0 line */
      EXTI->SWIER|= 0x00000001u;  /* EXTI_SWIER_SWIER0_Msk */
    }

    /*--------------------------------------------------------------------*/
    /*Update file elapsed time*/
    *time=0;        /*elapsed time*/
    *time_tot=0;    /*duration*/
    UpdateTimeInformation();

    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
    BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE), time, LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    /* Update audio state machine according to touch acquisition */
    AcquireTouchButtons();
    PlayerDynamicCfg();
    break;

  case AUDIO_STATE_START:

    /*reset audio player buffers*/
    ResetBuffers();

    audio_error = PLAYER_Start(FilePos);

    if (audio_error != AUDIO_ERROR_NONE)
    {
      return audio_error;
    }
    AudioState = AUDIO_STATE_PLAY;
    break;

  case AUDIO_STATE_STOP:
    if(PauseStatus==1)
    {
      /* Display blue cyan pause rectangles */
      BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
      BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
      BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
      PauseStatus=0;
    }
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(TOUCH_STOP_XMIN, TOUCH_STOP_YMIN , /* Delete Stop rectangle */
                     TOUCH_STOP_XMAX - TOUCH_STOP_XMIN,
                     TOUCH_STOP_YMAX - TOUCH_STOP_YMIN);
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
    BSP_LCD_FillPolygon(PlayPoints, 3);   /* Play icon */

    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
    sprintf((char *)str, "[00:00]");
    BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE), str, LEFT_MODE);
    sprintf((char *)str, "[STOP ]");
    BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE+1), str, LEFT_MODE);

    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
    PLAYER_Close();
    AudioState = AUDIO_STATE_WAIT;
    break;

  case AUDIO_STATE_NEXT:
    if(++FilePos >= AUDIO_GetFilObjectNumber())
    {
      FilePos = 0;
    }
    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
    PLAYER_Close();
    BSP_LCD_DisplayStringAtLine(15,  (uint8_t *)"                                                                     ");
    AudioState = AUDIO_STATE_START;
    break;

  case AUDIO_STATE_PREVIOUS:
    if(--FilePos < 0)
    {
      FilePos = AUDIO_GetFilObjectNumber() - 1;
    }
    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
    PLAYER_Close();
    BSP_LCD_DisplayStringAtLine(15,  (uint8_t *)"                                                                     ");
    AudioState = AUDIO_STATE_START;
    break;

  case AUDIO_STATE_PAUSE:
    PauseStatus=1;
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
    BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE+1), (uint8_t *)"[PAUSE]", LEFT_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);    /* Display red pause rectangles */
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_AUDIO_OUT_Pause();
    AudioState = AUDIO_STATE_WAIT;
    break;

  case AUDIO_STATE_RESUME:
    PauseStatus=0;
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
    BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE+1), (uint8_t *)"[PLAY ]", LEFT_MODE);
    /* Display blue cyan pause rectangles */
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_AUDIO_OUT_Resume();
    AudioState = AUDIO_STATE_PLAY;
    break;

  case AUDIO_STATE_INIT:
    PlayerRecorderDisplayButtons();
    AudioState = AUDIO_STATE_STOP;
  case AUDIO_STATE_WAIT:
  case AUDIO_STATE_IDLE:
  default:
    /* Update audio state machine according to touch acquisition */
    AcquireTouchButtons();
    PlayerDynamicCfg();
    break;
  }
  return audio_error;
}

/**
* @brief  Reset All buffers.
* @param  None
* @retval None
*/
static uint32_t ResetBuffers(void)
{
  uint32_t i =0;

  /*reset MemPool buffer*/
  /* FWK_MEMPOOL_DeInit(hAudio.pMemPool); */
  FWK_MEMPOOL_Init(hAudio.pMemPool,MEMPOOLBUFSZE);

  /*reset header buffer*/
  for (i = 0; i < MAX_AUDIO_HEADER_SIZE; i++)
  {
    tHeaderTmp[i] = 0;
  }

  /*reset output double buffer*/
  for (i = 0; i < AUDIO_OUT_BUFFER_SIZE; i++)
  {
    BufferCtl.buff[i] = 0;
  }

  return 0;
}

/**
* @brief  Stops Audio streaming.
* @param  None
* @retval Audio error
*/
AUDIO_ErrorTypeDef PLAYER_Stop(void)
{
    for(int i = uwVolumeRef; i >= 0; i--)
    {
        BSP_AUDIO_OUT_SetVolume(i);
    }


  AudioState = AUDIO_STATE_STOP;

  /*reset files reading list*/
  FilePos = 0;

  /*Stop the output audio stream*/
  BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);

  /* Close the current file */
  f_close(&AudioFile);

  return AUDIO_ERROR_NONE;
}

/**
* @brief  Close Audio decoder instance.
* @param  None
* @retval None
* @note: call before Player_Stop then exit.
*/
static uint32_t PLAYER_Close(void)
{
  /* Close the decoder instance */
  if (sDecoderStruct.DecoderDeInit != NULL)
  {
    sDecoderStruct.DecoderDeInit();
  }

  /* Empty the decoder structure */
  CODERS_SelectDecoder(&sDecoderStruct, ' ');

  /* Close the current file */
  f_close(&AudioFile);

  return AUDIO_ERROR_NONE;
}

/**
* @brief  This function Manages the DMA Transfer interrupts callback processing.
* @param  uint16_t offset
* @retval audio_error
*/
static AUDIO_ErrorTypeDef AudioOutCallBackProcess(uint16_t offset)
{
  AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;

  audio_error = MemPoolToSrc(offset);
  if (audio_error != AUDIO_ERROR_NONE)
  {
    return audio_error;
  }
  BufferCtl.state = BUFFER_OFFSET_NONE;
  return audio_error;
}

/**
* @brief  Manages the DMA Complete Transfer interrupt
*         Calculates the remaining file size and new position of the pointer.
* @param  None
* @retval None
*/
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
  /* Debug */
  if(status_IT1==1) counter_IT1stpIT2++;

  if(AudioState == AUDIO_STATE_PLAY)
  {
    BufferCtl.state = BUFFER_OFFSET_FULL;

    AudioOutCallBackProcess(AUDIO_OUT_BUFFER_SIZE/2);

  }
}

/**
* @brief  Manages the DMA Half Transfer interrupt.
* @param  None
* @retval None
*/
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
  /* Debug */
  if(status_IT1==1) counter_IT1stpIT2++;

  if(AudioState == AUDIO_STATE_PLAY)
  {
    BufferCtl.state = BUFFER_OFFSET_HALF;

    AudioOutCallBackProcess(0);

  }
}

/**
* @brief  SWI EXTI0 line detection callback.
*         Loop1: processing to fill MemPool with Buffer1 after USB key file read
* @retval None
*/
void SWI_EXTI0_Callback(void)
{
  /* Debug */
  status_IT1=1;

  /* Loop1 processing*/
  UsbToMemPool();

  /* Debug */
  status_IT1=0;

}

/*******************************************************************************
Static Functions
*******************************************************************************/
/**
* @brief  Gets the file info from header.
* @param  file_idx: File index
* @param  info: Pointer to Audio file (MP3, WAV)
* @retval Audio error
*/
static AUDIO_ErrorTypeDef PlayerGetFileInfo(uint16_t file_idx, Audio_InfoTypeDef *info)
{
  uint32_t bytesread;
  uint8_t *header;
  uint8_t error_tmp=0;

  /*header pointer allocation*/
  header=tHeaderTmp;
  uint8_t str[FILEMGR_FILE_NAME_SIZE + 20];


  ID3v2_tag* pTag;

  pTag = load_tag((char *)FileList.file[file_idx].name);





  /* Open Audio File */
  if(f_open(&AudioFile, (char *)FileList.file[file_idx].name, FA_OPEN_EXISTING | FA_READ) != FR_OK)
  {
    return AUDIO_ERROR_IO;
  }

  /* Select Audio Decoder with Abs Layer */
  FilePath = FileList.file[file_idx].name; /*EXPLE_FILE_NAME */

  /* Get the extension of audio file */
  FileExtension =  FilePath[strlen((char *)FilePath) - 1];



  /* ++ Read Header File information ++ */
  if(f_read(&AudioFile, header, MAX_AUDIO_HEADER_SIZE, (void *)&bytesread) != FR_OK)
  {
    f_close(&AudioFile);
    return AUDIO_ERROR_IO;
  }




  /* Inititalize the decoder Abs layer structure instance with matching decoder decoder or Null pointers */
  if(CODERS_SelectDecoder(&sDecoderStruct, FileExtension) != 0)
  {
    return AUDIO_ERROR_CODEC;
  }

  /* Use the most suitable packet size */
  OutDecPacketSizeBytes = sDecoderStruct.PacketSize;

  /* Initialization of the decoder */
  if (sDecoderStruct.DecoderInit != NULL)
  {
    error_tmp=sDecoderStruct.DecoderInit((uint8_t*)header,Dec_ReadDataCallback, PlayerSetPosition);

    if (error_tmp == 0)
    {
      /* ++ Fill the "info" structure with audio format parameters after decoding++ */

      /* Extract the current sampling rate */
      if (sDecoderStruct.Decoder_GetSamplingRate != NULL)
      {
        info->SampleRate = sDecoderStruct.Decoder_GetSamplingRate();
      }
      info->FileSize = AudioFile.fsize; /*in bytes*/

      /* --MP3 --*/
      if (FileExtension=='3')
      {
        info->NbrChannels = sDecoderStruct.Decoder_GetNbChannels();  /*stereo== 2*/

        info->FileFormat ='3';   /*MP3*/
        info->AudioFormat ='P';  /*PCM*/

        info->BitPerSample =16;
        /*After decoding: ((NumberOfFrames * mp3_Info.nSamplesPerFrame))/ NumberOfBytes;*/
        info->ByteRate =sDecoderStruct.Decoder_GetBitRateKbps()*1024/8;

      }
      /*--WAVE--*/
      else if ((FileExtension=='v')||(FileExtension=='V'))
      {
        info->NbrChannels = sDecoderStruct.Decoder_GetNbChannels();

        info->FileFormat = 'V';   /*WAVE*/
#ifdef __WAV_DECODER__
        if (DecWavFmtStruct.FormatTag == 1)
        {
          info->AudioFormat = 'P';
        }
        else
        {
          return AUDIO_ERROR_INVALID_VALUE;
        }
#else
        info->AudioFormat = 'P';
#endif
        info->BitPerSample =sDecoderStruct.Decoder_GetNbBits();
        info->ByteRate = (sDecoderStruct.Decoder_GetSamplingRate())*(sDecoderStruct.Decoder_GetNbChannels())*(sDecoderStruct.Decoder_GetNbBits())/8; /*raw PCM data*/
      }

      else
      {
        return AUDIO_ERROR_INVALID_VALUE;
      }
    }
    else
    {
      return AUDIO_ERROR_CODEC;
    }
  }

  /* ++ Audio info displayed on LCD ++ */
  BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(0, LINE(3), PLAYER_CLEAR_XWIDTH , LINE(1));
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  sprintf((char *)str, "Playing file (%d/%d): %s",
          file_idx + 1, FileList.ptr,
          (char *)FileList.file[file_idx].name);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(0, LINE(4), PLAYER_CLEAR_XWIDTH , LINE(1));
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_DisplayStringAtLine(4, str);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(0, LINE(5), PLAYER_CLEAR_XWIDTH , LINE(1));
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
  sprintf((char *)str,  "Sample rate : %d Hz", (int)(info->SampleRate));
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(0, LINE(6), PLAYER_CLEAR_XWIDTH , LINE(1));
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_DisplayStringAtLine(6, str);

  sprintf((char *)str,  "Channels number : %d", info->NbrChannels);
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(0, LINE(7), PLAYER_CLEAR_XWIDTH , LINE(1));
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_DisplayStringAtLine(7, str);

  /* Update file duration */
  *time=0;
  *time_tot=0;
  UpdateTimeInformation();

  BSP_LCD_ClearStringLine(8);
  sprintf((char *)str, "File Size : %d KB %s", (int)(info->FileSize/1024), time_tot );
  BSP_LCD_DisplayStringAtLine(8, str);
  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
  BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_YLINE), (uint8_t *)"[00:00]", LEFT_MODE);


  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  sprintf((char *)str,  "Volume : %lu", uwVolumeRef);
  BSP_LCD_ClearStringLine(9);
  BSP_LCD_DisplayStringAtLine(9, str);

  /*clear next lines*/
  BSP_LCD_ClearStringLine(10);
  BSP_LCD_ClearStringLine(11);
  BSP_LCD_ClearStringLine(12);
  BSP_LCD_ClearStringLine(13);
  BSP_LCD_ClearStringLine(14);
  BSP_LCD_ClearStringLine(15);
  BSP_LCD_ClearStringLine(16);

  return AUDIO_ERROR_NONE;
}

/**
* @brief  Initializes the Audio player.
* @param  AudioFreq: Audio sampling frequency
* @retval None
*/
static uint8_t PlayerStreamInit(uint32_t AudioFreq)
{
  /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */
  if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_BOTH, uwVolumeRef, AudioFreq) != 0)
  {
    return 1;
  }
  else
  {
    BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
    return 0;
  }
}

/**
* @brief  Display interface touch screen buttons
* @param  None
* @retval None
*/
static void PlayerRecorderDisplayButtons(void)
{
  BSP_LCD_SetFont(&LCD_LOG_HEADER_FONT);
  BSP_LCD_ClearStringLine(13);            /* Clear dedicated zone */
  BSP_LCD_ClearStringLine(14);
  BSP_LCD_ClearStringLine(15);

  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);

  BSP_LCD_FillPolygon(PreviousPoints, 3);   /* Previous track icon */
  BSP_LCD_FillRect(TOUCH_PREVIOUS_XMIN, TOUCH_PREVIOUS_YMIN , 10, TOUCH_PREVIOUS_YMAX - TOUCH_PREVIOUS_YMIN);

  BSP_LCD_FillPolygon(NextPoints, 3);       /* Next track icon */
  BSP_LCD_FillRect(TOUCH_NEXT_XMAX-9, TOUCH_NEXT_YMIN , 10, TOUCH_NEXT_YMAX - TOUCH_NEXT_YMIN);

  BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);    /* Pause rectangles */
  BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);

  if (AudioState == AUDIO_STATE_WAIT){
    BSP_LCD_FillPolygon(PlayPoints, 3);   /* Play icon */
  }
  else
  {
    BSP_LCD_FillRect(TOUCH_STOP_XMIN, TOUCH_STOP_YMIN , /* Activate Stop rectangle */
                     TOUCH_STOP_XMAX - TOUCH_STOP_XMIN,
                     TOUCH_STOP_YMAX - TOUCH_STOP_YMIN);
  }

  BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
  BSP_LCD_DrawRect(TOUCH_VOL_MINUS_XMIN, TOUCH_VOL_MINUS_YMIN , /* VOl- rectangle */
                   TOUCH_VOL_MINUS_XMAX - TOUCH_VOL_MINUS_XMIN,
                   TOUCH_VOL_MINUS_YMAX - TOUCH_VOL_MINUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_VOL_MINUS_XMIN+TOUCH_VOL_XOFFSET, LINE(TOUCH_VOL_LINE), (uint8_t *)"VOl-", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_VOL_PLUS_XMIN, TOUCH_VOL_PLUS_YMIN , /* VOl+ rectangle */
                   TOUCH_VOL_PLUS_XMAX - TOUCH_VOL_PLUS_XMIN,
                   TOUCH_VOL_PLUS_YMAX - TOUCH_VOL_PLUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_VOL_PLUS_XMIN+TOUCH_VOL_XOFFSET, LINE(TOUCH_VOL_LINE), (uint8_t *)"VOl+", LEFT_MODE);

  // EQ

  BSP_LCD_DrawRect(TOUCH_EQ1_MINUS_XMIN, TOUCH_EQ1_MINUS_YMIN , /* EQ1- rectangle */
                   TOUCH_EQ1_MINUS_XMAX - TOUCH_EQ1_MINUS_XMIN,
                   TOUCH_EQ1_MINUS_YMAX - TOUCH_EQ1_MINUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ1_MINUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_MINUS_LINE), (uint8_t *)"1-", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ2_MINUS_XMIN, TOUCH_EQ2_MINUS_YMIN , /* EQ2- rectangle */
                   TOUCH_EQ2_MINUS_XMAX - TOUCH_EQ2_MINUS_XMIN,
                   TOUCH_EQ2_MINUS_YMAX - TOUCH_EQ2_MINUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ2_MINUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_MINUS_LINE), (uint8_t *)"2-", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ3_MINUS_XMIN, TOUCH_EQ3_MINUS_YMIN , /* EQ3- rectangle */
                   TOUCH_EQ3_MINUS_XMAX - TOUCH_EQ3_MINUS_XMIN,
                   TOUCH_EQ3_MINUS_YMAX - TOUCH_EQ3_MINUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ3_MINUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_MINUS_LINE), (uint8_t *)"3-", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ4_MINUS_XMIN, TOUCH_EQ4_MINUS_YMIN , /* EQ4- rectangle */
                   TOUCH_EQ4_MINUS_XMAX - TOUCH_EQ4_MINUS_XMIN,
                   TOUCH_EQ4_MINUS_YMAX - TOUCH_EQ4_MINUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ4_MINUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_MINUS_LINE), (uint8_t *)"4-", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ5_MINUS_XMIN, TOUCH_EQ5_MINUS_YMIN , /* EQ5- rectangle */
                   TOUCH_EQ5_MINUS_XMAX - TOUCH_EQ5_MINUS_XMIN,
                   TOUCH_EQ5_MINUS_YMAX - TOUCH_EQ5_MINUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ5_MINUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_MINUS_LINE), (uint8_t *)"5-", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ1_PLUS_XMIN, TOUCH_EQ1_PLUS_YMIN , /* EQ1+ rectangle */
                   TOUCH_EQ1_PLUS_XMAX - TOUCH_EQ1_PLUS_XMIN,
                   TOUCH_EQ1_PLUS_YMAX - TOUCH_EQ1_PLUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ1_PLUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_PLUS_LINE), (uint8_t *)"1+", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ2_PLUS_XMIN, TOUCH_EQ2_PLUS_YMIN , /* EQ2+ rectangle */
                   TOUCH_EQ2_PLUS_XMAX - TOUCH_EQ2_PLUS_XMIN,
                   TOUCH_EQ2_PLUS_YMAX - TOUCH_EQ2_PLUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ2_PLUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_PLUS_LINE), (uint8_t *)"2+", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ3_PLUS_XMIN, TOUCH_EQ3_PLUS_YMIN , /* EQ3+ rectangle */
                   TOUCH_EQ3_PLUS_XMAX - TOUCH_EQ3_PLUS_XMIN,
                   TOUCH_EQ3_PLUS_YMAX - TOUCH_EQ3_PLUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ3_PLUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_PLUS_LINE), (uint8_t *)"3+", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ4_PLUS_XMIN, TOUCH_EQ4_PLUS_YMIN , /* EQ4+ rectangle */
                   TOUCH_EQ4_PLUS_XMAX - TOUCH_EQ4_PLUS_XMIN,
                   TOUCH_EQ4_PLUS_YMAX - TOUCH_EQ4_PLUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ4_PLUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_PLUS_LINE), (uint8_t *)"4+", LEFT_MODE);

  BSP_LCD_DrawRect(TOUCH_EQ5_PLUS_XMIN, TOUCH_EQ5_PLUS_YMIN , /* EQ5+ rectangle */
                   TOUCH_EQ5_PLUS_XMAX - TOUCH_EQ5_PLUS_XMIN,
                   TOUCH_EQ5_PLUS_YMAX - TOUCH_EQ5_PLUS_YMIN);
  BSP_LCD_DisplayStringAt(TOUCH_EQ5_PLUS_XMIN+TOUCH_EQ_XOFFSET, LINE(TOUCH_EQ_PLUS_LINE), (uint8_t *)"5+", LEFT_MODE);

  PlayerDisplaySettings();
  PlayerRecorderDisplayEQ();
  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
}

/**
* @brief  Detect touch screen state and modify audio state machine accordingly
* @param  None
* @retval None
*/
static void AcquireTouchButtons(void)
{
  static TS_StateTypeDef  TS_State={0};

  if(TS_State.touchDetected == 1)   /* If previous touch has not been released, we don't proceed any touch command */
  {
    BSP_TS_GetState(&TS_State);
  }
  else
  {
    BSP_TS_GetState(&TS_State);
    if(TS_State.touchDetected == 1)
    {
      if ((TS_State.touchX[0] > TOUCH_PAUSE_XMIN) && (TS_State.touchX[0] < TOUCH_PAUSE_XMAX) &&
          (TS_State.touchY[0] > TOUCH_PAUSE_YMIN) && (TS_State.touchY[0] < TOUCH_PAUSE_YMAX))
      {
        if (AudioState == AUDIO_STATE_PLAY)
        {
          AudioState = AUDIO_STATE_PAUSE;
        }
        else
        {
          AudioState = AUDIO_STATE_RESUME;
        }
      }
      else if ((TS_State.touchX[0] > TOUCH_NEXT_XMIN) && (TS_State.touchX[0] < TOUCH_NEXT_XMAX) &&
               (TS_State.touchY[0] > TOUCH_NEXT_YMIN) && (TS_State.touchY[0] < TOUCH_NEXT_YMAX))
      {
        AudioState = AUDIO_STATE_NEXT;
      }
      else if ((TS_State.touchX[0] > TOUCH_PREVIOUS_XMIN) && (TS_State.touchX[0] < TOUCH_PREVIOUS_XMAX) &&
               (TS_State.touchY[0] > TOUCH_PREVIOUS_YMIN) && (TS_State.touchY[0] < TOUCH_PREVIOUS_YMAX))
      {
        AudioState = AUDIO_STATE_PREVIOUS;
      }
      else if ((TS_State.touchX[0] > TOUCH_STOP_XMIN) && (TS_State.touchX[0] < TOUCH_STOP_XMAX) &&
               (TS_State.touchY[0] > TOUCH_STOP_YMIN) && (TS_State.touchY[0] < TOUCH_STOP_YMAX))
      {
        if ((AudioState == AUDIO_STATE_PLAY)|| ((AudioState == AUDIO_STATE_WAIT) && (PauseStatus == 1)))
        {
          AudioState = AUDIO_STATE_STOP;
        }
        else
        {
          AudioState = AUDIO_STATE_START;
        }
      }
      else if((TS_State.touchX[0] >= TOUCH_VOL_MINUS_XMIN) && (TS_State.touchX[0] <= TOUCH_VOL_MINUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_VOL_MINUS_YMIN) && (TS_State.touchY[0] <= TOUCH_VOL_MINUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_VOLUME_DOWN;
      }
      else if((TS_State.touchX[0] >= TOUCH_VOL_PLUS_XMIN) && (TS_State.touchX[0] <= TOUCH_VOL_PLUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_VOL_PLUS_YMIN) && (TS_State.touchY[0] <= TOUCH_VOL_PLUS_YMAX))
      {
        AudioCfgChange = AUDIO_CFG_VOLUME_UP;
      }

      else if((TS_State.touchX[0] >= TOUCH_EQ1_PLUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ1_PLUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ1_PLUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ1_PLUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ1_UP;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ2_PLUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ2_PLUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ2_PLUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ2_PLUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ2_UP;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ3_PLUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ3_PLUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ3_PLUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ3_PLUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ3_UP;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ4_PLUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ4_PLUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ4_PLUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ4_PLUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ4_UP;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ5_PLUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ5_PLUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ5_PLUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ5_PLUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ5_UP;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ1_MINUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ1_MINUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ1_MINUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ1_MINUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ1_DOWN;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ2_MINUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ2_MINUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ2_MINUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ2_MINUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ2_DOWN;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ3_MINUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ3_MINUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ3_MINUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ3_MINUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ3_DOWN;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ4_MINUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ4_MINUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ4_MINUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ4_MINUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ4_DOWN;
      }
      else if((TS_State.touchX[0] >= TOUCH_EQ5_MINUS_XMIN) && (TS_State.touchX[0] <= TOUCH_EQ5_MINUS_XMAX) &&
              (TS_State.touchY[0] >= TOUCH_EQ5_MINUS_YMIN) && (TS_State.touchY[0] <= TOUCH_EQ5_MINUS_YMAX))
      {
          AudioCfgChange = AUDIO_CFG_EQ5_DOWN;
      }

    }
  }
}


/**
* @brief  Manages Audio dynamic changes during playback (e.g. volume up/down).
* @param  None
* @retval None
*/
static void PlayerDynamicCfg(void)
{
  uint8_t str[13];
  static uint32_t Channel;

  switch(AudioCfgChange)
  {
  case AUDIO_CFG_VOLUME_UP:

    if (((AudioState == AUDIO_STATE_WAIT)||(AudioState == AUDIO_STATE_IDLE)) && (FilePos == 0))
    {
      /*clear list lines*/
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      BSP_LCD_FillRect(0, LINE(4), PLAYER_COUNT_TEXT_XMIN, LINE(16)-LINE(4));
      BSP_LCD_FillRect(0, LINE(4), BSP_LCD_GetXSize(), LINE(8)-LINE(4));
      PlayerRecorderDisplayButtons();
    }

    if( uwVolumeRef <= 90)
    {
      uwVolumeRef += 5;
    }
    BSP_AUDIO_OUT_SetVolume(uwVolumeRef);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    sprintf((char *)str,  "Volume : %lu ", uwVolumeRef);
    BSP_LCD_DisplayStringAtLine(9, str);
    AudioCfgChange = AUDIO_CFG_NONE;
    break;

  case AUDIO_CFG_VOLUME_DOWN:

    if (((AudioState == AUDIO_STATE_WAIT)||(AudioState == AUDIO_STATE_IDLE)) && (FilePos == 0))
    {
      /*clear list lines*/
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      BSP_LCD_FillRect(0, LINE(4), PLAYER_COUNT_TEXT_XMIN, LINE(16)-LINE(4));
      BSP_LCD_FillRect(0, LINE(4), BSP_LCD_GetXSize(), LINE(8)-LINE(4));
      PlayerRecorderDisplayButtons();
    }

    if( uwVolumeRef > 20)
    {
      uwVolumeRef -= 5;
    }
    BSP_AUDIO_OUT_SetVolume(uwVolumeRef);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    sprintf((char *)str,  "Volume : %lu ", uwVolumeRef);
    BSP_LCD_DisplayStringAtLine(9, str);
    AudioCfgChange = AUDIO_CFG_NONE;
    break;


  case AUDIO_CFG_EQ1_DOWN:
  case AUDIO_CFG_EQ2_DOWN:
  case AUDIO_CFG_EQ3_DOWN:
  case AUDIO_CFG_EQ4_DOWN:
  case AUDIO_CFG_EQ5_DOWN:
  {
      Channel = AudioCfgChange - AUDIO_CFG_EQ1_DOWN;
      if(EQ[Channel] > 0) EQ[Channel]--;
      BSP_AUDIO_OUT_AdjustEQ(Channel, EQ[Channel]);
      AudioCfgChange = AUDIO_CFG_NONE;
      PlayerRecorderDisplayEQ();
  }
  break;

  case AUDIO_CFG_EQ1_UP:
  case AUDIO_CFG_EQ2_UP:
  case AUDIO_CFG_EQ3_UP:
  case AUDIO_CFG_EQ4_UP:
  case AUDIO_CFG_EQ5_UP:
  {
      Channel = AudioCfgChange - AUDIO_CFG_EQ1_UP;
      if(EQ[Channel] < 24) EQ[Channel]++;
      BSP_AUDIO_OUT_AdjustEQ(Channel, EQ[Channel]);
      AudioCfgChange = AUDIO_CFG_NONE;
      PlayerRecorderDisplayEQ();
  }
  break;

  case AUDIO_CFG_NONE:
  default:
    break;
  }
}

/**
* @brief  Restore display text color and font.
* @param  None
* @retval None
*/
static void PlayerDisplaySettings(void)
{
  uint8_t str[30];
  src236_static_param_t src236_static_param;
  char *rate;

  BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
  BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);

  BSP_LCD_ClearStringLine(MSG_ERR_MOD_YLINE);

  switch(SrcTypeSelected) {
  case SELECT_SRC_NONE:
    sprintf((char *)str, "SRC: [NONE]");
    break;
  case SELECT_SRC_236:
    src236_getParam(&src236_static_param, pSrc236PersistentMem);
    rate= SRC236_ratio[src236_static_param.src_mode];
    sprintf((char *)str, "SRC: [SRC236, rate=%c%c%c]",rate[0],rate[1],rate[2]);
    break;
  case SELECT_SRC_441:
    sprintf((char *)str, "SRC: [SRC441]");
    break;
  }
  BSP_LCD_DisplayStringAt(TOUCH_NEXT_XMIN-CHAR(10), LINE(MSG_ERR_MOD_YLINE), (uint8_t *)str, LEFT_MODE);

}

#pragma GCC push_options
#pragma GCC optimize ("O0")

static void PlayerRecorderDisplayEQ(void)
{
    uint32_t Value;

    __asm("cpsid i" : : : "memory");


    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_DrawRect(402, 170, 51, 28);

    for(int i = 0; i < 5; i++)
    {
        Value = EQ[i];
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
        BSP_LCD_FillRect(404 + (i * 10),
                         173 + (24 - Value),
                         8,
                         Value);
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(404 + (i * 10),  //
                         172,
                         8,
                         24 - Value);
    }

    __asm("cpsie i" : : : "memory");

}
#pragma GCC pop_options
/**
* @brief  Read from USB a number of samples such that after SRC conversion
*         480 stereo samples are provided to the codec each 10 ms
* @param  bytesread: returns to the caller the bytes read from USB
* @param  offset: first flip flop buffer or second flip flop buffer
* @retval error: error type (default AUDIO_ERROR_NONE)
*/

static AUDIO_ErrorTypeDef MemPoolToSrc(uint16_t offset)
{
  uint32_t bytesread;

  uint32_t src_error;
  uint16_t i;

  /* Bypass SRC processing */
  if (SrcTypeSelected == SELECT_SRC_NONE)
  {
    /* Read from memory pool1 to temporary module processing computation buffer */
    if(FWK_MEMPOOL_Read(hptr->pMemPool, (uint8_t*)&BufferCtl.buff[offset],  AUDIO_OUT_BUFFER_SIZE/2)!= FWK_MEMPOOL_ERROR_NONE)
    {
      return (AUDIO_ERROR_IO);
    }
    bytesread=AUDIO_OUT_BUFFER_SIZE/2; /*force value*/
    BufferCtl.fptr += bytesread;
  }
  /* Selected SRC processing */
  else
  {
    for (i = 0; i < SrcIterations; ++i)
    {
      /* Read from memory pool1 to temporary module processing computation buffer*/
      if(FWK_MEMPOOL_Read(hptr->pMemPool, (uint8_t*)&WavToSrcTmpBuffer, AudioReadSize)== FWK_MEMPOOL_ERROR_NONE)
      {
        bytesread=AudioReadSize;
        BufferCtl.fptr += bytesread;

        OutputBuffer.data_ptr = &BufferCtl.buff[offset + i*AUDIO_OUT_BUFFER_SIZE /(2*SrcIterations)];

        if (SrcTypeSelected == SELECT_SRC_236)/* SrcTypeSelected == 236 */
        {
          src_error = src236_process(pInputBuffer, pOutputBuffer, pSrc236PersistentMem);
          if (src_error != SRC236_ERROR_NONE)
          {
            return (AUDIO_ERROR_SRC);
          }
        }
        else /* SrcTypeSelected == 441 */
        {
          src_error = src441_process(pInputBuffer, pOutputBuffer, pSrc441PersistentMem);
          if (src_error != SRC441_ERROR_NONE)
          {
            return (AUDIO_ERROR_SRC);
          }
        }
      }
      else
      {
        return (AUDIO_ERROR_IO);
      }
    }
  }

  return AUDIO_ERROR_NONE;
}

/**
* @brief  Sets the current position of the audio file pointer.
* @param  Pos: pointer position to be set
* @retval None
*/
/**
* @brief  Callback function to supply the decoder with input  bitsteram.
* @param  pCompressedData: pointer to the target buffer to be filled.
* @param  nDataSizeInChars: number of data to be read in bytes
* @param  pUserData: optional parameter (not used in this version)
* @retval return the decode frame.
*/
unsigned int Dec_ReadDataCallback(void *    pCompressedData,        /* [OUT] Bitbuffer */
                                  uint32_t  nDataSizeInChars,       /* sizeof(Bitbuffer) */
                                  void *    pUserData               /* Application-supplied parameter */ )
{
  uint32_t error_tmp = 0x00;

  error_tmp = f_read(&AudioFile, pCompressedData, nDataSizeInChars, (uint32_t*)(&NumberOfData));

  /* Check read issue */
  if (error_tmp != FR_OK)
  {
    PLAYER_Stop();
    return error_tmp;
  }

  /* Check the end of file */
  if (AudioFile.fptr >= AudioFile.fsize)
  {
    /* Fast replay without GUI update */
    PlayerSetPosition(0);

    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
    AudioState = AUDIO_STATE_NEXT;
    return FR_OK;
  }

  return  NumberOfData;
}

/**
* @brief  Sets the current position of the audio file pointer.
* @param  Pos: pointer position to be set
* @retval None
*/
uint32_t PlayerSetPosition(uint32_t Pos)
{
  /* Call the Fat FS seek function */
  return f_lseek(&AudioFile, Pos);
}


/**
* @brief  get file size
* @param  None
* @retval file size in bytes
*/
uint32_t PLAYER_GetFileLength(void)
{
  /* Call the Fat FS seek function */
  return AudioFile.fsize;
}


/**
* @brief  Updates the current time information (expressed in seconds).
* @param  Length: pointer to the variable containing the total audio track length
* @param  Total: pointer to the variable containing the elapsed time
* @retval 0 if Passed, !0 else.
*/
uint32_t PLAYER_GetTimeInfo(uint32_t* Length, uint32_t* Elapsed)
{
  if (sDecoderStruct.Decoder_GetStreamLength != NULL)
  {
    *Length = sDecoderStruct.Decoder_GetStreamLength(PLAYER_GetFileLength());
  }
  if (sDecoderStruct.Decoder_GetElapsedTime != NULL)
  {
    *Elapsed = sDecoderStruct.Decoder_GetElapsedTime(AudioFile.fptr);
  }

  return 0;
}

/**
* @brief  Function called to update the audio file time information
* @param  None
* @retval None
*/
static void UpdateTimeInformation(void)
{
  static uint32_t tLength = 0, tElapsed = 0;

  /* Get the total and elapsed time */
  PLAYER_GetTimeInfo(&tLength, &tElapsed);

  sprintf((char *)time_tot, "[%02d:%02d]", (int)(tLength/60), (int)(tLength%60));  /*duration in sec */
  sprintf((char *)time, "[%02d:%02d]", (int)(tElapsed/60), (int)(tElapsed%60));   /*elapsed time in sec*/

}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
  * @file    explorer.c
  * @author  MCD Application Team
  * @brief   This file provides USB Key drive configuration
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
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
FATFS USBH_FatFs;
char USBKey_Path[4] = "0:/";
FILELIST_FileTypeDef FileList;
uint16_t NumObs = 0;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the USB KEY Storage.
  * @param  None
  * @retval Status
  */
uint8_t AUDIO_StorageInit(void)
{
  /* Link the USB Key disk I/O driver */
  if((f_mount(&USBH_FatFs, (TCHAR const*)USBKey_Path, 0) != FR_OK))
  {
    /* FatFs Initialization Error */
    LCD_ErrLog("Cannot Initialize FatFs! \n");
    return 1;
  }
  else
  {
    LCD_DbgLog ("INFO : FatFs Initialized! \n");
    return 0;
  }
}

/**
  * @brief  Copies disk content in the explorer list.
  * @param  None
  * @retval Operation result
  */
FRESULT AUDIO_StorageParse(void)
{
  FRESULT res = FR_OK;
  FILINFO fno;
  DIR dir;
  char *fn;

#if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
#endif

  res = f_opendir(&dir, USBKey_Path);
  FileList.ptr = 0;

  if(res == FR_OK)
  {
    while(USBH_MSC_IsReady(&hUSBHost))
    {
      res = f_readdir(&dir, &fno);
      if(res != FR_OK || fno.fname[0] == 0)
      {
        break;
      }
      if(fno.fname[0] == '.')
      {
        continue;
      }
#if _USE_LFN
      fn = *fno.lfname ? fno.lfname : fno.fname;
#else
      fn = fno.fname;
#endif

      if(FileList.ptr < 240)
      {
        if((fno.fattrib & AM_DIR) == 0)
        {
          if((strstr(fn, "wav")) || (strstr(fn, "WAV")) || (strstr(fn, "mp3")) || (strstr(fn, "MP3")))
          {
            strncpy((char *)FileList.file[FileList.ptr].name, (char *)fn, FILEMGR_FILE_NAME_SIZE);
            FileList.file[FileList.ptr].type = FILETYPE_FILE;
            FileList.ptr++;
          }
        }
      }
    }
  }
  NumObs = FileList.ptr;
  f_closedir(&dir);
  return res;
}


/**
  * @brief  Shows audio file (*.wav) on the root
  * @param  None
  * @retval None
  */
uint8_t AUDIO_ShowAudioFiles(void)
{
  uint8_t i = 0;
  uint8_t line_idx = 0;
  if(AUDIO_StorageInit() == FR_OK)
  {
    if(AUDIO_StorageParse() ==  FR_OK)
    {
      if(FileList.ptr > 0)
      {

        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        LCD_UsrLog("audio file(s) [ROOT]:\n\n");

        for(i = 0; i < FileList.ptr; i++)
        {
          line_idx++;
          LCD_DbgLog("   |__");
          LCD_DbgLog((char *)FileList.file[i].name);
          LCD_DbgLog("\n");
        }
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        LCD_UsrLog("\nEnd of files list.\n");
        return 0;
      }
      return 1;
    }
    return 2;
  }
  else
  {
    return 3;
  }
}

/**
  * @brief  Gets Fil Object Number.
  * @param  None
  * @retval None
  */
uint16_t AUDIO_GetFilObjectNumber(void)
{
  return NumObs;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

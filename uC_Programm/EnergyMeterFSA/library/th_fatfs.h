//--------------------------------------------------------------
// File     : th_fatfs.h
//--------------------------------------------------------------
#ifndef TH_FATFS_H
#define TH_FATFS_H

/*
   _________________
  / 1 2 3 4 5 6 7 8 |       |SPI INTERFACE
 /					|   	|NAME	STM32F4XX	DESCRIPTION
/ 9					|   	|			4-BIT	1-BIT
| 					|   	|
|					|   1	|CS		PB5			Chip select for SPI
|					|   2	|MOSI	PA7			Data input for SPI
|					|   3	|VSS1	GND			GND
|   SD CARD Pinout	|   4	|VDD	3.3V		3.3V Power supply
|					|   5	|SCK	PA5			Clock for SPI
|					|   6	|VSS2	GND			GND
|					|   7	|MISO	PA6			Data output for SPI
|					|   8	|-		-			-
|___________________|   9	|-		-			-

 */

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "defines.h"
#include "th_gpio.h"
#include "ff.h"
#include "diskio.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

//--------------------------------------------------------------
// Defines / Typedefs
//--------------------------------------------------------------
#ifndef FATFS_TRUNCATE_BUFFER_SIZE
#define FATFS_TRUNCATE_BUFFER_SIZE	256
#endif

/* Memory allocation function */
#ifndef LIB_ALLOC_FUNC
#define LIB_ALLOC_FUNC    malloc
#endif

/* Memory free function */
#ifndef LIB_FREE_FUNC
#define LIB_FREE_FUNC     free
#endif

typedef struct {
	uint32_t TotalSize; /*!< Total size of memory */
	uint32_t FreeSize;  /*!< Free size of memory */
} TH_FATFS_Size_t;

typedef struct {
	uint32_t FoldersCount; /*!< Number of folders in last search operation */
	uint32_t FilesCount;   /*!< Number of files in last search operation */
} TH_FATFS_Search_t;

//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
/**
 * @brief   Gets total and free memory sizes of any drive
 * @param   *str: Pointer to string for drive to be checked
 * @param   *SizeStruct: Pointer to empty @ref TM_FATFS_Size_t structure to store data about memory
 * @retval  FRESULT structure members. If data are valid, FR_OK is returned
 * @example Get memory sizes of USB device:
 *             TM_FATFS_GetDriveSize("USB:", &SizeStruct);
 */
FRESULT TH_FATFS_GetDriveSize(char* str, TH_FATFS_Size_t* SizeStruct);

/*
 * @param  *Folder: Folder start location where search will be performed
 * @param  *tmp_buffer: Pointer to empty buffer where temporary data for filepath will be stored. It's size must be larger than bigest filepath on FATFS.
 *            Set this parameter to NULL and function will use @ref LIB_ALLOC_FUNC() to allocate memory for tmp buffer of size @arg tmp_buffer_size
 * @param  tmp_buffer_size: Number of bytes reserver for tmp_buffer so we won't overlaps buffer
 * @param  *FindStructure: Pointer to @ref TM_FATFS_Search_t structure
 * @retval Member of @ref FRESULT enumeration
 */
FRESULT TH_FATFS_Search(char* Folder, char* tmp_buffer, uint16_t tmp_buffer_size, TH_FATFS_Search_t* FindStructure);

/**
 * @brief  Search procedure callback function with filename result
 * @param  *path: Full path and file/folder name from search operation
 * @param  is_file: Is item a file or directory:
 *            - 0: Item is folder
 *            - > 0: Item is file
 * @param  Pointer to @ref TM_FATFS_Search_t structure which was passed to @ref TM_FATFS_Search with updated data
 * @retval Search status:
 *            - 0: Stop search operation
 *            - > 0: Continue with search
 * @note   With __weak parameter to prevent link errors if not defined by user
 */
uint8_t TH_FATFS_SearchCallback(char* path, uint8_t is_file, TH_FATFS_Search_t* FindStructure);


#endif


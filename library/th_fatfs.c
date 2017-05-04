//--------------------------------------------------------------
// File     : th_fatfs.c
// Datum    : 26.04.2017
// Version  : 1.0
// Autor    : Thomas Hackl
// EMail    : tom.hackl3@gmail.com
// CPU      : STM32F407VG
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO / RCC
// Funktion : FatFs for writing SD Card
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------

#include "th_fatfs.h"

//--------------------------------------------------------------
// Private Funktionen
//--------------------------------------------------------------

static FRESULT scan_files(char* path, uint16_t tmp_buffer_size, TH_FATFS_Search_t* FindStructure);

FRESULT TH_FATFS_GetDriveSize(char* str, TH_FATFS_Size_t* SizeStruct) {
	FATFS *fs;
    DWORD fre_clust;
	FRESULT res;

    /* Get volume information and free clusters of drive */
    res = f_getfree(str, &fre_clust, &fs);
    if (res != FR_OK) {
		return res;
	}

    /* Get total sectors and free sectors */
    SizeStruct->TotalSize = (fs->n_fatent - 2) * fs->csize * 0.5;
    SizeStruct->FreeSize = fre_clust * fs->csize * 0.5;

	/* Return OK */
	return FR_OK;
}

FRESULT TH_FATFS_Search(char* Folder, char* tmp_buffer, uint16_t tmp_buffer_size, TH_FATFS_Search_t* FindStructure) {
	uint8_t malloc_used = 0;
	FRESULT res;

	/* Reset values first */
	FindStructure->FilesCount = 0;
	FindStructure->FoldersCount = 0;

	/* Check for buffer */
	if (tmp_buffer == NULL) {
		/* Try to allocate memory */
		tmp_buffer = (char *) LIB_ALLOC_FUNC(tmp_buffer_size);

		/* Check for success */
		if (tmp_buffer == NULL) {
			return FR_NOT_ENOUGH_CORE;
		}
	}

	/* Check if there is a lot of memory allocated */
	if (strlen(Folder) < tmp_buffer_size) {
		/* Reset TMP buffer */
		tmp_buffer[0] = 0;

		/* Format path first */
		strcpy(tmp_buffer, Folder);

		/* Call search function */
		res = scan_files(tmp_buffer, tmp_buffer_size, FindStructure);
	} else {
		/* Not enough memory */
		res = FR_NOT_ENOUGH_CORE;
	}

	/* Check for malloc */
	if (malloc_used) {
		LIB_FREE_FUNC(tmp_buffer);
	}

	/* Return result */
	return res;
}

/*******************************************************************/
/*                      FATFS SEARCH CALLBACK                      */
/*******************************************************************/
__weak uint8_t TH_FATFS_SearchCallback(char* path, uint8_t is_file, TH_FATFS_Search_t* FindStructure) {
	/* NOTE: This function Should not be modified, when the callback is needed,
             the TH_FATFS_SearchCallback could be implemented in the user file
	*/

	/* Allow next search */
	return 1;
}

/*******************************************************************/
/*                    FATFS PRIVATE FUNCTIONS                      */
/*******************************************************************/
static FRESULT scan_files(char* path, uint16_t tmp_buffer_size, TH_FATFS_Search_t* FindStructure) {
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i;
	uint8_t gonext;
	char* fn;
#if _USE_LFN
	static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
	fno.lfname = lfn;
	fno.lfsize = sizeof lfn;
#endif

	/* Try to open file */
	if ((res = f_opendir(&dir, path)) == FR_OK) {
		/* Get length of current path */
		i = strlen(path);

		/* Read item from card */
		while ((res = f_readdir(&dir, &fno)) == FR_OK && fno.fname[0] != 0) {

			/* Ignore files */
			if (fno.fname[0] == '.') {
				continue;
			}

			/* Format name */
	#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
	#else
			fn = fno.fname;
	#endif

			/* Check if available memory for tmp buffer */
			/* + 1 is for "/" used for path formatting */
			if ((i + strlen(fn) + 1) >= tmp_buffer_size) {
				/* Not enough memory */
				res = FR_NOT_ENOUGH_CORE;

				break;
			}

			/* Format temporary path */
			sprintf(&path[i], "/%s", fn);

			/* Check for folder or file */
			if (fno.fattrib & AM_DIR) {
				/* We are folder */

				/* Increase number of folders */
				FindStructure->FoldersCount++;

				/* Call user function */
				gonext = TH_FATFS_SearchCallback(path, 0, FindStructure);

				/* Stop execution if user wants that */
				if (gonext) {
					/* Call recursive */
					res = scan_files(path, tmp_buffer_size, FindStructure);

					/* Check recursive scan result */
					if (res != FR_OK) {
						break;
					}
				}
			} else {
				/* We are file */

				/* Increase number of files */
				FindStructure->FilesCount++;

				/* Call user function */
				gonext = TH_FATFS_SearchCallback(path, 1, FindStructure);
			}

			/* Set path back */
			path[i] = 0;

			/* Stop execution if user wants that */
			if (!gonext || res != FR_OK) {
				break;
			}
		}

		/* Close directory */
		f_closedir(&dir);
	}

	/* Return result */
	return res;
}

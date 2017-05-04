#ifndef TH_DEFINES_H
#define TH_DEFINES_H

/* Put your global defines for all libraries here used in your project */
#define FATFS_USE_SDIO		0
#define FATFS_SPI			SPI1
#define FATFS_SPI_PINSPACK	TH_SPI_PinsPack_1
#define FATFS_CS_PORT		GPIOA
#define FATFS_CS_PIN		GPIO_Pin_4


// Check for GNUC
#if defined (__GNUC__)
	#ifndef __weak
		#define __weak   	__attribute__((weak))
	#endif	// Weak attribute
	#ifndef __packed
		#define __packed 	__attribute__((__packed__))
	#endif	// Packed attribute
#endif

#endif

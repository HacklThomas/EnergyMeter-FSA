#include "main.h"

/* ALPHA Prototyp:
 * IVT-A:    SPI1 PinsPack1 | EOC: PE3 | OCS: PE2 | SS: PA4
 * SD-Card:  SPI3 PinsPack2 | CS: PA15
 * BT-Modul: RX: PA9 | TX: PA10 = USART1 PinsPack1
 *
 * BETA Prototyp:
 * IVT-A:    SPI2 PinsPack2 | EOC: PB10 | OCS: PB11 | SS: PB12
 * SD-Card:  SPI1 PinsPack1 | CS: PA4
 * BT-Modul: RX: PC10 | TX: PC11 = USART3 PinsPack2
 */

int main(void) {
	init();

#if defined (MINVOLTAGE)
	state = IDLE;
#else
	state = INITIALISIEREN;
#endif
	interface = UNDEFINED;

	uint32_t pointer = 0;
	int ret = 5;
	int temp = 0;
	bool_t prevcanstate;

	bool_t crcinit = TRUE;
	uint32_t crcresult;
	int cryptcounter = 0;

	uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab,
			0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
	uint8_t iv[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	uint8_t in[16];

	while (1) {
		switch (state) {
		case INITIALISIEREN:
			TH_LED_Off(LED_RED);
			TH_LED_On(LED_GREEN);
			insertHeaderToFile();
			state = LOGGING;
			break;
		case LOGGING:
			if (interrupttimer > 50000) {
				ivterror = TRUE;
				TH_LED_Off(LED_GREEN);
				TH_LED_On(LED_RED);
			} else {
				ivterror = FALSE;
				TH_LED_Off(LED_RED);
			}
			if (blink >= 1000000) {
				blink = 0;
				TH_LED_Toggle(LED_GREEN);
			}
			if (checkForStateChange() == TRUE) {
				f_close(&fil);
				f_mount(0, "", 1);
				TH_LED_Off(LED_GREEN);
			}
#if defined (MINVOLTAGE)
			if (voltage < (MINVOLTAGE - 2000)) { //Spannung fällt unter Schwelle
				f_close(&fil);
				f_mount(0, "", 1);
				TH_LED_Off(LED_GREEN);
				state = IDLE;
			}
#endif
			break;
		case LISTFILES:
			TH_LED_On(LED_RED);
			prevcanstate = canstate;
			canstate = FALSE;
			f_mount(&FatFs, "", 1);
			temp = countFiles();
			f_open(&fil, "allFiles.txt", FA_READ);
			f_lseek(&fil, 0);
			for (int i = 1; i <= temp; i++) {
				f_gets(buffer, sizeof(buffer), &fil);
				pointer = f_tell(&fil);
				f_close(&fil);
				ret = strlen(buffer);
				ret--;
				buffer[ret] = 0x00;
				sendData(buffer);
				sendData(" - Size (kB): ");
				sprintf(buffer, "%i.txt", i);
				f_open(&fil, buffer, FA_READ);
				itoa((f_size(&fil) / 1000), buffer, 10);
				sendData(buffer);
				sendData("\n");
				f_close(&fil);
				f_open(&fil, "allFiles.txt", FA_READ);
				f_lseek(&fil, pointer);
			}
			sendData("ENDE\r\n");
			f_close(&fil);
			f_mount(0, "", 1);
			TH_LED_Off(LED_RED);
			state = IDLE;
			canstate = prevcanstate;
			break;
		case AUSLESEN:
			prevcanstate = canstate;
			canstate = FALSE;
			f_mount(&FatFs, "", 1);
			TH_LED_On(LED_RED);
			do {
				ret = receiveData();
			} while (ret == -1);
			sprintf(buffer, "%i.txt", ret);
			f_open(&fil, buffer, FA_READ);
			itoa((f_size(&fil)), buffer, 10);
			sendData(buffer);
			sendData("\n");
			while (receiveData() != 3) {
			}
			;
			f_lseek(&fil, 0);
			while (f_eof(&fil) == 0) {
				f_gets(buffer, sizeof(buffer), &fil);
				sendData(buffer);
#ifdef ENCRYPTION
				cryptcounter++;
				if (crcinit) {
					crcresult = TH_CRC_Calculate8(buffer, strlen(buffer) - 1,
							1);
					crcinit = FALSE;
				} else {
					if (cryptcounter >= 200) {
						crcresult = TH_CRC_Calculate8(buffer,
								strlen(buffer) - 1, 0);

						in[0] = (crcresult & 0xFF000000) >> 24;
						in[4] = in[8] = in[12] = in[0];
						in[1] = (crcresult & 0x00FF0000) >> 16;
						in[5] = in[9] = in[13] = in[1];
						in[2] = (crcresult & 0x0000FF00) >> 8;
						in[6] = in[10] = in[14] = in[2];
						in[3] = crcresult & 0x000000FF;
						in[7] = in[11] = in[15] = in[3];

						for (int i = 0; i < 16; i++) {
							cyphertext[i] = 0x00;
						}

						AES128_CBC_encrypt_buffer(cyphertext, in, 16, key, iv);
						sendCyphertext();

						cryptcounter = 0;
						crcinit = TRUE;
					} else {
						crcresult = TH_CRC_Calculate8(buffer,
								strlen(buffer) - 1, 0);
					}
				}
#endif
			}
#ifdef ENCRYPTION
			in[0] = (crcresult & 0xFF000000) >> 24;
			in[4] = in[8] = in[12] = in[0];
			in[1] = (crcresult & 0x00FF0000) >> 16;
			in[5] = in[9] = in[13] = in[1];
			in[2] = (crcresult & 0x0000FF00) >> 8;
			in[6] = in[10] = in[14] = in[2];
			in[3] = crcresult & 0x000000FF;
			in[7] = in[11] = in[15] = in[3];

			for (int i = 0; i < 16; i++) {
				cyphertext[i] = 0x00;
			}

			AES128_CBC_encrypt_buffer(cyphertext, in, 16, key, iv);
			sendCyphertext();
#endif
			sendData("ENDE\r\n");
			f_close(&fil);
			f_mount(0, "", 1);
			state = IDLE;
			TH_LED_Off(LED_RED);
			canstate = prevcanstate;
			break;
		case SETTIME:
			TH_LED_On(LED_RED);
			setTime();
			sendData("EM: Time set\r\n");
			state = IDLE;
			TH_LED_Off(LED_RED);
			break;
		case BLUETOOTHGRAPH:
			// SEQUENCE: "E902.0,123.4,193.43\n"
			TH_LED_On(LED_RED);
			sprintf(buffer, "E%5.2lf,%5.2lf,%5.2lf\n", current / 1000.0,
					voltage / 1000.0, temperature / 1000.0);
			TH_UART_SendString(COM1, buffer, NONE);
			if (checkForStateChange()) {
				TH_LED_Off(LED_RED);
			}
			break;
		case DELETEFILES:
			TH_LED_On(LED_RED);
			f_mount(&FatFs, "", 1);
			f_findfirst(&dj, &fno, "", "*.txt");
			while (fno.fname[0]) {
				if ((strcmp(fno.fname, "canactive.txt") != 0)
						&& (strcmp(fno.fname, "initfile.txt") != 0)
						&& (strcmp(fno.fname, "ivtserial.txt") != 0)) {
					f_unlink(fno.fname);
				}
				f_findnext(&dj, &fno);
			}
			f_mount(0, "", 1);
			insertHeaderToFile();
			f_close(&fil);
			f_mount(0, "", 1);
			sendData("EM: All Files deleted\r\n");
			state = IDLE;
			TH_LED_Off(LED_RED);
			break;
		case IDLE:
			TH_LED_On(LED_GREEN);
			if (checkForStateChange()) {
				TH_LED_Off(LED_GREEN);
			}
#if defined (MINVOLTAGE)
			if (voltage > MINVOLTAGE) { //Spannung geht über Schwelle
				state = INITIALISIEREN;
			}
#endif
			break;
		case TOGGLECAN:
			TH_LED_On(LED_RED);
			f_mount(&FatFs, "", 1);
			if (canstate == TRUE) {
				f_unlink("canactive.txt");
				sendData("EM: CAN disabled\r\n");
				canstate = FALSE;
			} else {
				f_open(&initfile, "canactive.txt", FA_CREATE_ALWAYS | FA_WRITE);
				f_close(&initfile);
				sendData("EM: CAN enabled\r\n");
				canstate = TRUE;
			}
			state = IDLE;
			f_mount(0, "", 1);
			TH_LED_Off(LED_RED);
			break;
		case DELETEINITFILE:
			TH_LED_On(LED_RED);
			f_mount(&FatFs, "", 1);
			f_unlink("initfile.txt");
			f_mount(0, "", 1);
			TH_LED_Off(LED_RED);
			sendData("EM: Init File Deleted\r\n");
			state = IDLE;
			break;
		default:
			break;
		}
	}
}


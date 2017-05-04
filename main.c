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

	state = INITIALISIEREN;
	interface = UNDEFINED;

	uint32_t pointer = 0;
	int ret = 5;
	int temp = 0;
	bool_t prevcanstate;

	for (int i = 0; i < 64; i++) {
		cyphertext[i] = 0x00;
	}

	while (1) {
		switch (state) {
		case INITIALISIEREN:
			insertHeaderToFile();
			state = LOGGING;
			TH_LED_Off(LED_RED);
			TH_LED_On(LED_GREEN);
			break;
		case LOGGING:
			if (blink >= 1000000) {
				blink = 0;
				TH_LED_Toggle(LED_GREEN);
			}
			if (checkForStateChange() == TRUE) {
				f_close(&fil);
				f_mount(0, "", 1);
				TH_LED_Off(LED_GREEN);
			}
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
				sendData(" - Size (Bytes): ");
				sprintf(buffer, "%i.txt", i);
				f_open(&fil, buffer, FA_READ);
				itoa(f_size(&fil), buffer, 10);
				sendData(buffer);
				sendData("\n");
				f_close(&fil);
				f_open(&fil, "allFiles.txt", FA_READ);
				f_lseek(&fil, pointer);
			}
			sendData("ENDE");
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
			f_lseek(&fil, 0);
			while (f_eof(&fil) == 0) {
				f_gets(buffer, sizeof(buffer), &fil);
				sendData(buffer);
				temp++;
				//encrypt_cbc();
				if (temp > 200) {
					temp = 0;
					//sendCyphertext();
					for (int i = 0; i < 64; i++) {
						cyphertext[i] = 0x00;
					}
				}
			}
			//sendCyphertext();
			temp = 0;
			sendData("ENDE");
			f_close(&fil);
			f_mount(0, "", 1);
			state = IDLE;
			TH_LED_Off(LED_RED);
			for (int i = 0; i < 64; i++) {
				cyphertext[i] = 0x00;
			}
			canstate = prevcanstate;
			break;
		case SETTIME:
			TH_LED_On(LED_RED);
			setTime();
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
				f_unlink(fno.fname);
				f_findnext(&dj, &fno);
			}
			f_mount(0, "", 1);
			insertHeaderToFile();
			f_close(&fil);
			f_mount(0, "", 1);
			state = IDLE;
			TH_LED_Off(LED_RED);
			break;
		case IDLE:
			TH_LED_On(LED_GREEN);
			if (checkForStateChange()) {
				TH_LED_Off(LED_GREEN);
			}
			break;
		case TOGGLECAN:
			TH_LED_On(LED_RED);
			if (canstate == TRUE) {
				canstate = FALSE;
			} else {
				canstate = TRUE;
			}
			state = IDLE;
			TH_LED_Off(LED_RED);
			break;
		case TEST:
			TH_SYSTICK_Pause_ms(500);
			sendCAN();
			break;
		default:
			break;
		}
	}
}


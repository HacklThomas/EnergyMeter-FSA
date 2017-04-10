//--------------------------------------------------------------
// File     : stm32_ub_ext_int10to15.c
// Datum    : 05.04.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO, EXTI, SYSCFG, MISC
// Funktion : External Interrupt10 bis 15
//
// Hinweis  : mögliche Pinbelegungen
//            EXT_INT10 :
//               [PA10,PB10,PC10,PD10,PE10,PF10,PG10,PH10,PI10]
//            EXT_INT11 :
//               [PA11,PB11,PC11,PD11,PE11,PF11,PG11,PH11,PI11]
//            EXT_INT12 :
//               [PA12,PB12,PC12,PD12,PE12,PF12,PG12,PH12,PI12]
//            EXT_INT13 :
//               [PA13,PB13,PC13,PD13,PE13,PF13,PG13,PH13,PI13]
//            EXT_INT14 :
//               [PA14,PB14,PC14,PD14,PE14,PF14,PG14,PH14,PI14]
//            EXT_INT15 :
//               [PA15,PB15,PC15,PD15,PE15,PF15,PG15,PH15,PI15]
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_ext_int10to15.h"
#include "main.h"
// ab hier eigene includes

//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_EXT_INT10_LoFlanke(void);
void P_EXT_INT10_HiFlanke(void);
void P_EXT_INT11_LoFlanke(void);
void P_EXT_INT11_HiFlanke(void);
void P_EXT_INT12_LoFlanke(void);
void P_EXT_INT12_HiFlanke(void);
void P_EXT_INT13_LoFlanke(void);
void P_EXT_INT13_HiFlanke(void);
void P_EXT_INT14_LoFlanke(void);
void P_EXT_INT14_HiFlanke(void);
void P_EXT_INT15_LoFlanke(void);
void P_EXT_INT15_HiFlanke(void);

//--------------------------------------------------------------
// Definition vom EXT_INT10 bis 15 Pin
//
// Widerstand : [GPIO_PuPd_NOPULL,GPIO_PuPd_UP,GPIO_PuPd_DOWN]
// Trigger    : [EXTI_Trigger_Rising,EXTI_Trigger_Falling,EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
EXT_INT10TO15_t EXT_INT10TO15[] = {
//NR , PORT, PIN      , CLOCK              ,Source,             ,Widerstand      , Trigger
		{ 15, GPIOC, GPIO_Pin_15, RCC_AHB1Periph_GPIOC, EXTI_PortSourceGPIOC,
				GPIO_PuPd_DOWN, EXTI_Trigger_Falling }, // EXT_Int-Pin = PC15
		{ 10, GPIOB, GPIO_Pin_10, RCC_AHB1Periph_GPIOB, EXTI_PortSourceGPIOB,
				GPIO_PuPd_DOWN, EXTI_Trigger_Falling }, // EXT_Int-Pin = PB14
		};

//--------------------------------------------------------------
// Init vom external Interrupt 10 bis 15
//--------------------------------------------------------------
void UB_Ext_INT10TO15_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	uint8_t nr;

	for (nr = 0; nr < EXT_INT10TO15_ANZ; nr++) {
		// Clock enable (GPIO)
		RCC_AHB1PeriphClockCmd(EXT_INT10TO15[nr].INT_CLK, ENABLE);

		// Config als Digital-Eingang
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = EXT_INT10TO15[nr].INT_R;
		GPIO_InitStructure.GPIO_Pin = EXT_INT10TO15[nr].INT_PIN;
		GPIO_Init(EXT_INT10TO15[nr].INT_PORT, &GPIO_InitStructure);

		// Clock enable (SYSCONFIG)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		if (EXT_INT10TO15[nr].INT_NR == 10) {
			// EXT_INT10 mit Pin verbinden
			SYSCFG_EXTILineConfig(EXT_INT10TO15[nr].INT_SOURCE,
					EXTI_PinSource10);

			// EXT_INT10 config
			EXTI_InitStructure.EXTI_Line = EXTI_Line10;
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
			EXTI_InitStructure.EXTI_Trigger = EXT_INT10TO15[nr].INT_TRG;
			EXTI_InitStructure.EXTI_LineCmd = ENABLE;
			EXTI_Init(&EXTI_InitStructure);
		} else if (EXT_INT10TO15[nr].INT_NR == 11) {
			// EXT_INT11 mit Pin verbinden
			SYSCFG_EXTILineConfig(EXT_INT10TO15[nr].INT_SOURCE,
					EXTI_PinSource11);

			// EXT_INT11 config
			EXTI_InitStructure.EXTI_Line = EXTI_Line11;
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
			EXTI_InitStructure.EXTI_Trigger = EXT_INT10TO15[nr].INT_TRG;
			EXTI_InitStructure.EXTI_LineCmd = ENABLE;
			EXTI_Init(&EXTI_InitStructure);
		} else if (EXT_INT10TO15[nr].INT_NR == 12) {
			// EXT_INT12 mit Pin verbinden
			SYSCFG_EXTILineConfig(EXT_INT10TO15[nr].INT_SOURCE,
					EXTI_PinSource12);

			// EXT_INT12 config
			EXTI_InitStructure.EXTI_Line = EXTI_Line12;
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
			EXTI_InitStructure.EXTI_Trigger = EXT_INT10TO15[nr].INT_TRG;
			EXTI_InitStructure.EXTI_LineCmd = ENABLE;
			EXTI_Init(&EXTI_InitStructure);
		} else if (EXT_INT10TO15[nr].INT_NR == 13) {
			// EXT_INT13 mit Pin verbinden
			SYSCFG_EXTILineConfig(EXT_INT10TO15[nr].INT_SOURCE,
					EXTI_PinSource13);

			// EXT_INT13 config
			EXTI_InitStructure.EXTI_Line = EXTI_Line13;
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
			EXTI_InitStructure.EXTI_Trigger = EXT_INT10TO15[nr].INT_TRG;
			EXTI_InitStructure.EXTI_LineCmd = ENABLE;
			EXTI_Init(&EXTI_InitStructure);
		} else if (EXT_INT10TO15[nr].INT_NR == 14) {
			// EXT_INT14 mit Pin verbinden
			SYSCFG_EXTILineConfig(EXT_INT10TO15[nr].INT_SOURCE,
					EXTI_PinSource14);

			// EXT_INT14 config
			EXTI_InitStructure.EXTI_Line = EXTI_Line14;
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
			EXTI_InitStructure.EXTI_Trigger = EXT_INT10TO15[nr].INT_TRG;
			EXTI_InitStructure.EXTI_LineCmd = ENABLE;
			EXTI_Init(&EXTI_InitStructure);
		} else if (EXT_INT10TO15[nr].INT_NR == 15) {
			// EXT_INT15 mit Pin verbinden
			SYSCFG_EXTILineConfig(EXT_INT10TO15[nr].INT_SOURCE,
					EXTI_PinSource15);

			// EXT_INT15 config
			EXTI_InitStructure.EXTI_Line = EXTI_Line15;
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
			EXTI_InitStructure.EXTI_Trigger = EXT_INT10TO15[nr].INT_TRG;
			EXTI_InitStructure.EXTI_LineCmd = ENABLE;
			EXTI_Init(&EXTI_InitStructure);
		}

	}

	// NVIC config
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Lo-Flanke am EXT-INT10
// wenn [EXTI_Trigger_Falling oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT10_LoFlanke(void) {
	int crc = 0xFFFF;
	interrupttimer = 0;
	sendCAN();

	switch (ivtConfig) {
	case 0:
		if (configout[2] == 4) {
			configout[2] = 3;
			measureTemperature = FALSE;
			Led_Toggle(LED_BLUE);
		} else {
			configout[2] = 4;
			measureTemperature = TRUE;
		}
		crc = 0xFFFF;
		for (int i = 0; i < 7; i++) {
			crc = crc16_update(crc, configout[i]);
		}
		configout[7] = crc & 0x00FF;
		configout[8] = crc >> 8;

		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
		for (int j = 0; j < 10; j++) {
		}
		for (int i = 0; i < 9; i++) {
			SPI2->DR = configout[i];
			SPI_WAIT(SPI2);
			measurein[i] = SPI2->DR;
			for (int j = 0; j < 10; j++) {
			}
		}
		for (int j = 0; j < 10; j++) {
		}
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
		crc = 0xFFFF;
		for (int i = 0; i < 9; i++) {
			crc = crc16_update(crc, measurein[i]);
		}

		if (configout[2] == 4) {
			voltage = interpret24bitAsInt32(measurein[6], measurein[5],
					measurein[4]);
		} else {
			temperature = interpret24bitAsInt32(measurein[6], measurein[5],
					measurein[4]);
		}
		current = interpret24bitAsInt32(measurein[3], measurein[2],
				measurein[1]);
		if (state == LOGGING && crc == 0x0000) {
			saveData();
		}
		ivtConfig = 1;
		break;
	case 1:
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
		for (int j = 0; j < 10; j++) {
		}
		for (int i = 0; i < 9; i++) {
			SPI2->DR = measureout[i];
			SPI_WAIT(SPI2);
			measurein[i] = SPI2->DR;
			for (int j = 0; j < 10; j++) {
			}
		}
		for (int j = 0; j < 10; j++) {
		}
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
		crc = 0xFFFF;
		for (int i = 0; i < 9; i++) {
			crc = crc16_update(crc, measurein[i]);
		}

		if (measureTemperature == TRUE) {
			temperaturecounter++;
			if (temperaturecounter >= 3) {
				temperaturecounter = 0;
				ivtConfig = 0;
			}
			break;
		}
		if (temperaturecounter < 3) {
			temperaturecounter++;
			break;
		}
		voltage = interpret24bitAsInt32(measurein[6], measurein[5],
				measurein[4]);
		current = interpret24bitAsInt32(measurein[3], measurein[2],
				measurein[1]);
		if (state == LOGGING && crc == 0x0000) {
			saveData();
		}
		temperaturecounter++;
		if (temperaturecounter > 1000) {
			temperaturecounter = 0;
			ivtConfig = 0;
		}
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Hi-Flanke am EXT-INT10
// wenn [EXTI_Trigger_Rising oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT10_HiFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Lo-Flanke am EXT-INT11
// wenn [EXTI_Trigger_Falling oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT11_LoFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Hi-Flanke am EXT-INT11
// wenn [EXTI_Trigger_Rising oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT11_HiFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Lo-Flanke am EXT-INT12
// wenn [EXTI_Trigger_Falling oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT12_LoFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Hi-Flanke am EXT-INT12
// wenn [EXTI_Trigger_Rising oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT12_HiFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Lo-Flanke am EXT-INT13
// wenn [EXTI_Trigger_Falling oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT13_LoFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Hi-Flanke am EXT-INT13
// wenn [EXTI_Trigger_Rising oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT13_HiFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Lo-Flanke am EXT-INT14
// wenn [EXTI_Trigger_Falling oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT14_LoFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Hi-Flanke am EXT-INT14
// wenn [EXTI_Trigger_Rising oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT14_HiFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Lo-Flanke am EXT-INT15
// wenn [EXTI_Trigger_Falling oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT15_LoFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// diese Funktion wird aufgerufen,
// bei einer Hi-Flanke am EXT-INT15
// wenn [EXTI_Trigger_Rising oder EXTI_Trigger_Rising_Falling]
//--------------------------------------------------------------
void P_EXT_INT15_HiFlanke(void) {
	// hier eigenen Code eintragen

}

//--------------------------------------------------------------
// external Interrupt-10 bis 15
// wird je nach einstellung der Triggerflanke
// entweder bei Hi- oder Lo- oder beiden Flanken ausgelöst
//--------------------------------------------------------------
void EXTI15_10_IRQHandler(void) {
	uint32_t wert;
	uint8_t nr;

	if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
		// wenn Interrupt aufgetreten
		EXTI_ClearITPendingBit(EXTI_Line10);

		for (nr = 0; nr < EXT_INT10TO15_ANZ; nr++) {
			if (EXT_INT10TO15[nr].INT_NR == 10) {
				wert = GPIO_ReadInputDataBit(EXT_INT10TO15[nr].INT_PORT,
						EXT_INT10TO15[nr].INT_PIN);
				if (wert == Bit_RESET) {
					P_EXT_INT10_LoFlanke();
				} else {
					P_EXT_INT10_HiFlanke();
				}
				break;
			}
		}
	}
	if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
		// wenn Interrupt aufgetreten
		EXTI_ClearITPendingBit(EXTI_Line11);

		for (nr = 0; nr < EXT_INT10TO15_ANZ; nr++) {
			if (EXT_INT10TO15[nr].INT_NR == 11) {
				wert = GPIO_ReadInputDataBit(EXT_INT10TO15[nr].INT_PORT,
						EXT_INT10TO15[nr].INT_PIN);
				if (wert == Bit_RESET) {
					P_EXT_INT11_LoFlanke();
				} else {
					P_EXT_INT11_HiFlanke();
				}
				break;
			}
		}
	}
	if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
		// wenn Interrupt aufgetreten
		EXTI_ClearITPendingBit(EXTI_Line12);

		for (nr = 0; nr < EXT_INT10TO15_ANZ; nr++) {
			if (EXT_INT10TO15[nr].INT_NR == 12) {
				wert = GPIO_ReadInputDataBit(EXT_INT10TO15[nr].INT_PORT,
						EXT_INT10TO15[nr].INT_PIN);
				if (wert == Bit_RESET) {
					P_EXT_INT12_LoFlanke();
				} else {
					P_EXT_INT12_HiFlanke();
				}
				break;
			}
		}
	}
	if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
		// wenn Interrupt aufgetreten
		EXTI_ClearITPendingBit(EXTI_Line13);

		for (nr = 0; nr < EXT_INT10TO15_ANZ; nr++) {
			if (EXT_INT10TO15[nr].INT_NR == 13) {
				wert = GPIO_ReadInputDataBit(EXT_INT10TO15[nr].INT_PORT,
						EXT_INT10TO15[nr].INT_PIN);
				if (wert == Bit_RESET) {
					P_EXT_INT13_LoFlanke();
				} else {
					P_EXT_INT13_HiFlanke();
				}
				break;
			}
		}
	}
	if (EXTI_GetITStatus(EXTI_Line14) != RESET) {
		// wenn Interrupt aufgetreten
		EXTI_ClearITPendingBit(EXTI_Line14);

		for (nr = 0; nr < EXT_INT10TO15_ANZ; nr++) {
			if (EXT_INT10TO15[nr].INT_NR == 14) {
				wert = GPIO_ReadInputDataBit(EXT_INT10TO15[nr].INT_PORT,
						EXT_INT10TO15[nr].INT_PIN);
				if (wert == Bit_RESET) {
					P_EXT_INT14_LoFlanke();
				} else {
					P_EXT_INT14_HiFlanke();
				}
				break;
			}
		}
	}
	if (EXTI_GetITStatus(EXTI_Line15) != RESET) {
		// wenn Interrupt aufgetreten
		EXTI_ClearITPendingBit(EXTI_Line15);

		for (nr = 0; nr < EXT_INT10TO15_ANZ; nr++) {
			if (EXT_INT10TO15[nr].INT_NR == 15) {
				wert = GPIO_ReadInputDataBit(EXT_INT10TO15[nr].INT_PORT,
						EXT_INT10TO15[nr].INT_PIN);
				if (wert == Bit_RESET) {
					P_EXT_INT15_LoFlanke();
				} else {
					P_EXT_INT15_HiFlanke();
				}
				break;
			}
		}
	}
}


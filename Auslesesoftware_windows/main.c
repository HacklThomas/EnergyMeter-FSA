//
//  main.c
//  SerialTerminal
//
//  Created by Thomas Hackl on 21.03.17.
//  Copyright © 2017 Thomas Hackl. All rights reserved.
//

#include "main.h"
#include "serial.h"


//  --- DEFINES
#define TEXT_BUFFER 10

int main(int argc, const char * argv[]) {
    int i=1;
    int cport_nr=0;

    char portname[50];
    unsigned char buf[1024];

    int selection = 0;
    int numberOfPorts = 0;
    int selectedPort = -1;
    int stateOfEM = 1;
    int running = 1;
    char receiveText[TEXT_BUFFER];
    char sendText[TEXT_BUFFER];

    FILE *file;

    char temp[200];
    char * commandsForGnuplot[] = {"cd 'C:/Users/thomashackl/cb_workspace/Serial Terminal'", "set title \"DATA\"", "x=0; y=0"};

    char buffer[64];
    uint8_t cyphertext[64];

    uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab,
        0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t iv[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t in[64];

    char *comports[RS232_PORTNR]={"\\\\.\\COM1",  "\\\\.\\COM2",  "\\\\.\\COM3",  "\\\\.\\COM4",
                              "\\\\.\\COM5",  "\\\\.\\COM6",  "\\\\.\\COM7",  "\\\\.\\COM8",
                              "\\\\.\\COM9",  "\\\\.\\COM10", "\\\\.\\COM11", "\\\\.\\COM12",
                              "\\\\.\\COM13", "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16"};

    char temporary[4];
    char hexstring[195];
    int verified = 1;
    state_t state = CONNECTPORT;

    const char * name[numberOfPorts];


    printf("\n\t*****************************************\n");
    printf("\t*                                       *\n");
    printf("\t*            SERIAL TERMINAL            *\n");
    printf("\t*                                       *\n");
    printf("\t*  -----------------------------------  *\n");
    printf("\t*                                       *\n");
    printf("\t*  Formula Student Austria Energymeter  *\n");
    printf("\t*                                       *\n");
    printf("\t*              Created by               *\n");
    printf("\t*     HACKL Thomas / KRIMMEL Florian    *\n");
    printf("\t*                                       *\n");
    printf("\t*  Technical University of Vienna 2017  *\n");
    printf("\t*****************************************\n\n");

    while(running){
        switch (state) {
            case CONNECTPORT: // Verbinden zu einem verfügbaren Serial Port
                comEnumerate();
                numberOfPorts = comGetNoPorts();
                selectedPort = -1;
                while(selectedPort < 0){
                    printf("--- AVAILABLE PORTS: ---\n\n");
                    for(i = 0; i < numberOfPorts; i++){
                        printf("%d - ", i);
                        name[i] = comGetPortName(i);
                        printf("%s\n", name[i]);
                    }
                    printf("\n%d - REFRESH\n", numberOfPorts);

                    printf("\n\nSelect Port to open: ");
                    scanf("%d", &selectedPort);
                    if((selectedPort > numberOfPorts || selectedPort < 0)){
                        system("cls");
                        printf("Invalid Input!\n\n");
                        selectedPort = -1;
                        continue;
                    }
                }
                if(selectedPort == numberOfPorts){
                    system("cls");
                    break;
                }

                sprintf(portname, "\\\\.\\%s", comGetPortName(selectedPort));
                for(i=0; i < RS232_PORTNR; i++){
                    if(strcmp(portname, comports[i]) == 0){
                        cport_nr = i;
                    }
                }

                system("cls");
                if(RS232_OpenComport(cport_nr))
                {
                    system("cls");
                    printf("Can not open comport\n");
                    break;
                }else{
                    printf("\nPORT OPEN\n\n");
                    state = MENU;
                }
                break;
            case MENU:
                printf("---------  MENU  --------- Current State of EM: ");
                printStateOfEM(stateOfEM);
                printf("\n 1 - Start Logging");
                printf("\n 2 - List Files");
                printf("\n 3 - Download File from EM");
                printf("\n 4 - Set Time");
                printf("\n 5 - Delete all Files on EM");
                printf("\n 6 - Bluetooth Graph");
                printf("\n 7 - Plot File");
                printf("\n 8 - Disconnect");
                printf("\n 9 - Disconnect and close Terminal");
                printf("\n     Choice: ");

                selection = 0;

                while(selection == 0){
                    scanf("%i", &selection);
                    if(selection < 1 || selection > 9){
                        selection = 0;
                        printf("Invalid Input!\n\n");
                    } else{
                        state = selection;
                    }
                }
                break;
            case LOGGING:
                RS232_cputs(cport_nr, "1\r\n");
                state = MENU;
                stateOfEM = 1;
                system("cls");
                break;
            case LISTFILES: // Liste aller auf dem EM gespeicherten Files wird angezeigt
                RS232_cputs(cport_nr, "2\r\n");
                i = 1;
                system("cls");
                int timeout = 0;
                //RECEIVE FILENAMES
                while(1){
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                    i = strstr(buf, "ENDE"); // Auslesen bis "ENDE" empfangen wird
                    if(i != 0){
                        i = 0;
                        timeout = 0;
                        while(i < strlen(buf)){
                            if(buf[i] == 69){ // Ausgabe bis 'E' = 69
                                break;
                            }
                            putc(buf[i], stdout);
                            i++;
                        }
                        break;
                    }
                    printf("%s", buf);
                    timeout++;
                    if(timeout > 1000000){
                        break;
                    }
                }
                printf("\n");
                state = MENU;
                stateOfEM = 2;
                break;
            case DOWNLOADFILE: // Auslesen eines Files und Überprüfen der Verschlüsselung
                RS232_cputs(cport_nr, "3\r\n");
                printf("Number of the File: ");

                scanf("%d", &selection);
                sprintf(sendText, "%d", selection);
                system("cls");
                printf("\n\tDownloading File ... \n");
                RS232_cputs(cport_nr, sendText);
                RS232_cputs(cport_nr, "\r\n");
                strcat(sendText, ".txt");

                file = fopen(sendText, "w+");

                while(1){
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                    i = strstr(buf, "ENDE");
                    if(i != 0){
                        i = 0;
                        while(i < strlen(buf)){
                            if(buf[i] == 69){
                                break;
                            }
                            putc(buf[i], file);
                            i++;
                        }
                        break;
                    }
                    fputs((char *)buf, file);
                }

                // END! in COIDE am Schluss senden -> folgende 2 Zeilen löschen
                // Derzeit END! vor letztem Schlüssel
                usleep(10000);
                RS232_PollComport(cport_nr, buf, 1023);

                stateOfEM = 2;
                system("cls");
                printf("\n File Nr. %d downloaded\n\n", atoi(sendText));

                // VERIFY
                fseek(file, 0, SEEK_SET);

                while(feof(file) == 0){
                    fgets(buffer, sizeof(buffer), file);
                    if(strcmp(buffer, "CRYPT: \n") == 0){
                        fgets(temp, sizeof(temp), file);
                        fseek(file, -193, SEEK_CUR);
                        fputs("#", file); // '#' vor jeden Schlüssel setzen -> Auskommentieren für GNUPLOT
                        fputs(temp, file);

                        for (i = 0; i < 64; i++) {
                            sprintf(temporary, "%2x ", cyphertext[i]);
                            strcat(hexstring, temporary);
                        }
                        strcat(hexstring, "\n");

                        while(temp[0] < 48 && temp[0] != 32){ // Wenn temp[0] == EOT -> 1 nach vor schieben
                            for(i = 0; i<200; i++){
                                temp[i] = temp[i+1];
                            }
                        }
                        if(strcmp(temp, hexstring) != 0){
                            printf("\n ERROR VERIFYING FILE:\n");
                            printf("\n FILE: %s", temp);
                            printf(" CALC: %s\n", hexstring);
                            verified = 0;
                        }

                        hexstring[0] = 0;
                        cyphertext[0] = 0;
                        buffer[0] = 0;
                        temp[0] = 0;
                    } else {
                        for (i = 0; i < 64; i++) {
                            in[i] = buffer[i];
                        }

                        AES128_CBC_encrypt_buffer(cyphertext, in, 64, key, iv);
                    }
                }
                if(verified){
                    printf(" File Nr. %d verified\n\n", atoi(sendText));
                }else{
                    printf(" File Nr. %d  NOT verified\n\n", atoi(sendText));
                }
                verified = 1;

                fseek(file, 109, SEEK_SET);
                fputs("#", file);

                fclose(file);
                state = MENU;
                break;
            case SETTIME:
                RS232_cputs(cport_nr, "4\r\n");

                time_t rawtime;
                struct tm * timeinfo;
                time ( &rawtime );
                timeinfo = localtime ( &rawtime );

                sprintf(sendText, "%i\r\n", timeinfo->tm_year - 100);
                RS232_cputs(cport_nr, sendText);
                usleep(1000);
                sprintf(sendText, "%i\r\n", timeinfo->tm_mon);
                RS232_cputs(cport_nr, sendText);
                usleep(1000);
                sprintf(sendText, "%i\r\n", timeinfo->tm_mday);
                RS232_cputs(cport_nr, sendText);
                usleep(1000);
                sprintf(sendText, "%i\r\n", timeinfo->tm_hour);
                RS232_cputs(cport_nr, sendText);
                usleep(1000);
                sprintf(sendText, "%i\r\n", timeinfo->tm_min);
                RS232_cputs(cport_nr, sendText);
                usleep(1000);
                sprintf(sendText, "%i\r\n", timeinfo->tm_sec);
                RS232_cputs(cport_nr, sendText);

                state = MENU;
                stateOfEM = 2;
                system("cls");
                printf ("\n Current local time and date: %i/%i/%i - %i:%i:%i\n", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year + 1900,
                        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
                printf("\n Time set\n\n");
                break;
            case DELETEFILES:
                printf("Do you really want do delete all Files on the EnergyMeter? (y to confirm): ");
                scanf("%s", receiveText);
                system("cls");
                if(strcmp(receiveText, "y") == 0){
                    RS232_cputs(cport_nr, "7\r\n");
                    printf("\n All Files deleted\n\n");
                }else{
                    printf("\n Canceled\n\n");
                }
                state = MENU;
                stateOfEM = 2;
                break;
            case BLUETOOTHGRAPH:
                RS232_cputs(cport_nr, "6\r\n");
                state = MENU;
                stateOfEM = 3;
                system("cls");
                break;
            case PLOTFILE:
                printf("Number of the File: ");
                scanf("%d", &selection);
                system("cls");
                printf("\n\nPlotting File ... \n");
                sprintf(temp, "plot '%d.txt' using (x=x+($1/1000000)):($2) with lines, '%d.txt' using (y=y+($1/1000000)):($3) with lines", selection, selection);

                FILE * gnuplotPipe = _popen("c:\\gnuplot\\bin\\pgnuplot.exe -persist", "w");
                fprintf(gnuplotPipe, "set datafile commentschars \"UT#C\"\n");
                for (i=0; i < 3; i++){
                    fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]);
                }
                fprintf(gnuplotPipe, "%s \n", temp);

                fflush(gnuplotPipe);
                _pclose(gnuplotPipe);

                state = MENU;
                break;
            case DISCONNECT:
                RS232_CloseComport(cport_nr);
                system("cls");
                printf("\n Disconnected\n\n");
                state = CONNECTPORT;
                break;
            case ENDSESSION:
                printf("Ending Session...\n");
                RS232_CloseComport(cport_nr);
                running = 0;
                break;
            default:
                break;
        }
    }

    return 0;
}


void printStateOfEM(int state){
    switch (state) {
        case 1:
            printf("LOGGING");
            break;
        case 2:
            printf("IDLE");
            break;
        case 3:
            printf("BLUETOOTH GRAPH");
            break;
        default:
            break;
    }
}


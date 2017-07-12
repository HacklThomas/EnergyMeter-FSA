//
//  main.c
//  SerialTerminal
//
//  Created by Thomas Hackl on 21.03.17.
//  Copyright © 2017 Thomas Hackl. All rights reserved.
//

#include "main.h"


//  --- DEFINES
#define TEXT_BUFFER 10
#define TIMEOUT 4 //s
//#define PRINTFULL


/*
 
 Normal: \x1b[0m
 Background: \x1b[42m
 Foreground: \x1b[32m
 
 red 1
 green 2
 yellow 3
 blue 4
 magenta 5
 cyan 6
 white 7
 
 */

int main(int argc, const char * argv[]) {
    int i=1;
    int cport_nr=0;       /* /dev/ttyS0 (COM1 on windows) */

    char portname[50];
    unsigned char buf[1024];
    time_t start;
    
    int selection = 0;
    int numberOfPorts = 0;
    int selectedPort = -1;
    int stateOfEM = 1;
    bool_t running = TRUE;
    char receiveText[TEXT_BUFFER];
    char sendText[TEXT_BUFFER];
    
    FILE *file;
    
    char temp[200];
    
    bool_t verified = TRUE;
    state_t state = CONNECTPORT;
    
    const char * name[numberOfPorts];
    
    system("printf '\e[3;0;0t'");
    system("printf '\e[8;70;80t'");
    
    while(running == TRUE){
        switch (state) {
            case CONNECTPORT: // Verbinden zu einem verfügbaren Serial Port
                printf("\n\t*****************************************\n");
                printf("\t*                                       *\n");
                printf("\t*            \x1b[34mSERIAL TERMINAL\x1b[0m            *\n");
                printf("\t*                                       *\n");
                printf("\t*---------------------------------------*\n");
                printf("\t*                                       *\n");
                printf("\t*  \x1b[34mFormula Student Austria Energymeter\x1b[0m  *\n");
                printf("\t*                                       *\n");
                printf("\t*              \x1b[34mCreated by\x1b[0m               *\n");
                printf("\t*     \x1b[34mHACKL Thomas / KRIMMEL Florian\x1b[0m    *\n");
                printf("\t*                                       *\n");
                printf("\t*     \x1b[34mVienna University of Technology\x1b[0m   *\n");
                printf("\t*****************************************\n\n");
                comEnumerate();
                numberOfPorts = comGetNoPorts();
                selectedPort = -1;
                while(selectedPort < 0){
                    printf("--- AVAILABLE PORTS: ---\n\n");
                    for(int i = 0; i < numberOfPorts; i++){
                        printf("\x1b[34m%d\x1b[0m - ", i);
                        name[i] = comGetPortName(i);
                        printf("%s\n", name[i]);
                    }
                    printf("\n\x1b[34m%d\x1b[0m - REFRESH\n", numberOfPorts);
                    
                    printf("\n\nSelect Port to open: ");
                    scanf("%d", &selectedPort);
                    if((selectedPort > numberOfPorts || selectedPort < 0)){
                        system("clear");
                        errorprint("\n Invalid Input!\n\n");
                        selectedPort = -1;
                        continue;
                    }
                }
                if(selectedPort == numberOfPorts){
                    system("clear");
                    break;
                }
                
                sprintf(portname, "/dev/%s", comGetPortName(selectedPort));
                
                system("clear");
                
                if(RS232_OpenComport(cport_nr, portname))
                {
                    errorprint("Can not open comport\n");
                    system("clear");
                    break;
                }else{
                    printf("\n PORT OPEN\n\n");
                    state = MENU;
                }
                break;
            case MENU:
                printf("---------  MENU  --------- Current State of EM: ");
                printStateOfEM(stateOfEM);
                printf("\n \x1b[34m1\x1b[0m - Start Logging");
                printf("\n \x1b[34m2\x1b[0m - List Files");
                printf("\n \x1b[34m3\x1b[0m - Download File from EM");
                printf("\n \x1b[34m4\x1b[0m - Set Time");
                printf("\n \x1b[34m5\x1b[0m - Delete all Files on EM");
                printf("\n \x1b[34m6\x1b[0m - Bluetooth Graph");
                printf("\n \x1b[34m7\x1b[0m - Plot File");
                printf("\n \x1b[34m8\x1b[0m - De-/Aktivieren des CAN Busses");
                printf("\n \x1b[34m9\x1b[0m - Leere Empfangspuffer");
                printf("\n \x1b[34m10\x1b[0m - Terminal");
                printf("\n");
                printf("\n \x1b[34m11\x1b[0m - Disconnect");
                printf("\n \x1b[34m12\x1b[0m - Disconnect and close Terminal");
                printf("\n \x1b[34m13\x1b[0m - Test");
                printf("\n\n     Choice: ");
                
                selection = 0;
                
                while(selection == 0){
                    scanf("%i", &selection);
                    if(selection < 1 || selection > 13){
                        selection = 0;
                        system("clear");
                        errorprint("\n Invalid Input!\n\n");
                        break;
                    } else{
                        state = selection;
                    }
                }
                break;
            case LOGGING:
                RS232_cputs(cport_nr, "1\r\n");
                state = MENU;
                stateOfEM = 1;
                system("clear");
                break;
            case LISTFILES: // Liste aller auf dem EM gespeicherten Files wird angezeigt
                RS232_cputs(cport_nr, "2\r\n");
                i = 1;
                system("clear");
                //RECEIVE FILENAMES
                while(1){
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                    start = time(NULL);
                    i = strstr(buf, "ENDE"); // Auslesen bis "ENDE" empfangen wird
                    if(i != 0){
                        start = time(NULL);
                        i = 0;
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
                    if(time(NULL) > start + TIMEOUT){
                        errorprint("\nTIMEOUT\n");
                        break;
                    }
                }
                printf("\n");
                state = MENU;
                stateOfEM = 2;
                break;
            case DOWNLOADFILE: // Auslesen eines Files und Überprüfen der Verschlüsselung
                RS232_cputs(cport_nr, "3\r\n");
                printf("\t-> Number of the File: ");
                
                scanf("%d", &selection);
                sprintf(sendText, "%d", selection);
                RS232_cputs(cport_nr, sendText);
                RS232_cputs(cport_nr, "\r\n");
                strcat(sendText, ".txt");
                
                file = fopen(sendText, "w+");
                
                i = 0;
                start = time(NULL);
                do{
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                    if(time(NULL) > start + TIMEOUT){
                        errorprint("\nTIMEOUT\n");
                        break;
                    }
                }while(i == 0);
                
                long txsize = atoi(buf);
                long rxsize = 0;
                float oldpercentage = 0;
                float percentage = 0;
                
                RS232_cputs(cport_nr, "3\r\n");
                
                while(1){
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    rxsize += i;
                    buf[i] = 0;
                    
                    if(strstr(buf, "ENDE") != 0){
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
                    
                    percentage = (rxsize * 100.0) / txsize;
                    if((int) percentage > (int) oldpercentage){
                        printprogressbar((int) percentage);
                        oldpercentage = percentage;
                    }
                }
                
                fclose(file);
                printprogressbar(100);
                
                // END! in COIDE am Schluss senden -> folgende 2 Zeilen löschen
                // Derzeit END! vor letztem Schlüssel
                usleep(10000);
                RS232_PollComport(cport_nr, buf, 1023);
            
                stateOfEM = 2;
                system("clear");
                printf("\n File Nr. %d downloaded\n\n", atoi(sendText));
                
                // VERIFY
                if(verify(selection)){
                    printf(" File Nr. %d verified\n\n", atoi(sendText));
                }else{
                    sprintf(temp, " File Nr. %d  NOT verified\n\n", atoi(sendText));
                    errorprint(temp);
                }
                verified = TRUE;
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
                sprintf(sendText, "%i\r\n", (timeinfo->tm_mon)+1);
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
                
                stateOfEM = 2;
                system("clear");
                printf ("\n Current local time and date: %i/%i/%i - %i:%i:%i\n\n ", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year + 1900,
                        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
                
                i = 0;
                start = time(NULL);
                do{
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                    if(time(NULL) > start + TIMEOUT){
                        errorprint("\nTIMEOUT\n");
                        break;
                    }
                }while(i == 0);
                printf("\n ");
                puts(buf);
                state = MENU;
                
                break;
            case DELETEFILES:
                printf("\t-> Enter PIN: ");
                scanf("%s", receiveText);
                system("clear");
                strcat(receiveText, "\r\n");
                RS232_cputs(cport_nr, receiveText);
                
                i = 0;
                start = time(NULL);
                do{
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                    if(time(NULL) > start + TIMEOUT){
                        errorprint("\n FALSE PIN OR TIMEOUT\n");
                        break;
                    }
                }while(i == 0);
                printf("\n ");
                puts(buf);
                
                state = MENU;
                stateOfEM = 2;
                break;
            case BLUETOOTHGRAPH:
                RS232_cputs(cport_nr, "6\r\n");
                state = MENU;
                stateOfEM = 3;
                system("clear");
                break;
            case PLOTFILE:
                printf("\t-> Number of the File: ");
                scanf("%d", &selection);
                system("clear");
                printf("\n\nPlotting File ... \n");
                
                sprintf(temp, "%d.txt", selection);
                
                FILE *gnuplotfile = fopen("gnuplotfile.txt", "w+");
                file = fopen(temp, "r");
                if(file == 0){
                    errorprint("\n FILE NOT EXISTING\n");
                    state = MENU;
                    break;
                }
                for (i = 0; i < 10; i++) {
                    fgets(temp, sizeof(temp), file);
                }
                while(feof(file) == 0){
                    fgets(temp, sizeof(temp), file);
                    if(strcmp(temp, "CRYPT: \n") == 0){
                        fgets(temp, sizeof(buf), file);
                    } else {
                        fputs(temp, gnuplotfile);
                    }
                }
                fclose(file);
                
                char tim[50];
                char voltage[50];
                char current[50];
                char lv[50];
                
                double v = 0;
                double c = 0;
                double t = 0;
                double energy = 0;
                double logtime = 0;
#ifndef PRINTFULL
                int average = 200;
                double avgv = 0;
                double avgc = 0;
                long avgt = 0;
                double avglv = 0;
                double ctr = 0;
                FILE *avgfile = fopen("gnuplotfile_avg.txt", "w+");
#endif
                fseek(gnuplotfile, 0, SEEK_SET);
                while(feof(gnuplotfile) == 0){
                    fscanf(gnuplotfile, "%s", tim);
                    fscanf(gnuplotfile, "%s", voltage);
                    fscanf(gnuplotfile, "%s", current);
                    fscanf(gnuplotfile, "%s", lv);
                    
                    v = atof(voltage);
                    c = atof(current);
                    t = atof(tim) / 1000000;
                    energy += v*c*t;
                    logtime += t;
                    
#ifndef PRINTFULL
                    ctr++;
                    if(ctr >= average){
                        ctr = 0;
                        sprintf(temp, "%ld\t%lf\t%lf\t%lf\n", avgt, avgv / 200.0, avgc / 200.0, avglv / 200.0);
                        fputs(temp, avgfile);
                        avgt = avgv = avgc = avglv = 0;
                    } else {
                        avgv += v;
                        avgc += c;
                        avgt += atoi(tim);
                        avglv += atof(lv);
                    }
#endif
                }
                
                energy = energy / (1000*3600);
                
                fclose(gnuplotfile);
                
                
                FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
                //fprintf(gnuplotPipe, "set datafile commentschars \"UT#C\"\n");
                
                fprintf(gnuplotPipe, "cd '/Users/thomashackl/' \n");
                fprintf(gnuplotPipe, "x=0; y=0; z=0 \n");
#ifdef PRINTFULL
                sprintf(temp, "Energy Consumption: %lfkWh / Time: %lf", energy, logtime);
                fprintf(gnuplotPipe, "set title \"%s\" \n", temp);
                fprintf(gnuplotPipe, "plot 'gnuplotfile.txt' using (x=x+($1/1000000)):($2) with lines title \"voltage\", 'gnuplotfile.txt' using (y=y+($1/1000000)):($3) with lines title \"current\", 'gnuplotfile.txt' using (z=z+($1/1000000)):($4) with lines title \"low voltage\"\n");
#else
                sprintf(temp, "Energy Consumption: %lfkWh / Time: %lf / Averaged over %d values", energy, logtime, average);
                fprintf(gnuplotPipe, "set title \"%s\" \n", temp);
                fprintf(gnuplotPipe, "plot 'gnuplotfile_avg.txt' using (x=x+($1/1000000)):($2) with lines title \"voltage\", 'gnuplotfile_avg.txt' using (y=y+($1/1000000)):($3) with lines title \"current\", 'gnuplotfile_avg.txt' using (z=z+($1/1000000)):($4) with lines title \"low voltage\"\n");
#endif
                pclose(gnuplotPipe);
                
                state = MENU;
                break;
            case TOGGLECAN:
                RS232_cputs(cport_nr, "8\r\n");
                system("clear");
                printf("\n ");
                
                i = 0;
                start = time(NULL);
                do{
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                    if(time(NULL) > start + TIMEOUT){
                        errorprint("\n FALSE PIN OR TIMEOUT\n");
                        break;
                    }
                }while(i == 0);
                printf("\n ");
                puts(buf);
                
                state = MENU;
                break;
            case CLEARBUFFER:
                i = RS232_PollComport(cport_nr, buf, 1023);
                buf[i] = 0;
                while(i != 0){
                    i=0;
                    usleep(10000);
                    while(i < strlen(buf)){
                        putc(buf[i], stdout);
                        i++;
                    }
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                }
                printf("\n");
                state = MENU;
                break;
            case TERMINAL:
                printf("\n\n\tx - Exit Terminal and go to Menu");
                printf("\n\n\tInput: ");
                scanf("%s", receiveText);
                
                if(strcmp(receiveText, "x") == 0){
                    state = MENU;
                    system("clear");
                }else{
                    strcat(receiveText, "\r\n");
                    RS232_cputs(cport_nr, receiveText);
                }
                
                usleep(TIMEOUT*1000000);
                
                i = RS232_PollComport(cport_nr, buf, 1023);
                buf[i] = 0;
                while(i != 0){
                    i=0;
                    usleep(10000);
                    while(i < strlen(buf)){
                        putc(buf[i], stdout);
                        i++;
                    }
                    i = RS232_PollComport(cport_nr, buf, 1023);
                    buf[i] = 0;
                }
                break;
            case DISCONNECT:
                RS232_CloseComport(cport_nr);
                system("clear");
                printf("\n Disconnected\n\n");
                state = CONNECTPORT;
                break;
            case ENDSESSION:
                printf("Ending Session...\n");
                RS232_CloseComport(cport_nr);
                running = FALSE;
                break;
            case TEST:
                printf("TEST: \n");
                verify(2);
                printf("\nFINISHED\n");
                state = MENU;
            default:
                break;
        }
    }
    return 0;
}

void printprogressbar(int percent){
    system("clear");
    printf("\n\tDownloading File ... \n\n\t[");
    for(int i = 0; i < 50; i++){
        if(i < percent/2){
            printf("#");
        } else {
            printf(" ");
        }
    }
    printf("] - [%d%%]\n", percent);
}

bool_t verify(int number){
    FILE *file;
    char buffer[10];
    char temp[64];
    char x[4];
    char hexstring[50];
    uint32_t crc = 0;
    
    bool_t crcinit = TRUE;
    bool_t status = TRUE;
    uint8_t in[16];
    uint8_t cyphertext[16];
    uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab,
        0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t iv[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    
    sprintf(buffer, "%d.txt", number);
    
    file = fopen(buffer, "r");
    if(file == 0){
        errorprint("NO FILE");
        return FALSE;
    }
    
    fseek(file, 0, SEEK_SET);
    
    while(feof(file) == 0){
        fgets(temp, sizeof(temp), file);
        
        if(strcmp(temp, "CRYPT: \r\n") == 0){
            fgets(temp, sizeof(temp), file);
            //crc = -1888508381;
            in[0] = (crc & 0xFF000000) >> 24;
            in[4] = in[8] = in[12] = in[0];
            in[1] = (crc & 0x00FF0000) >> 16;
            in[5] = in[9] = in[13] = in[1];
            in[2] = (crc & 0x0000FF00) >> 8;
            in[6] = in[10] = in[14] = in[2];
            in[3] = crc & 0x000000FF;
            in[7] = in[11] = in[15] = in[3];
            
            for (int i = 0; i < 16; i++) {
                cyphertext[i] = 0x00;
            }
            AES128_CBC_encrypt_buffer(cyphertext, in, 16, key, iv);
            
            hexstring[0] = 0;
            for (int i = 0; i < 16; i++) {
                sprintf(x, "%2x ", cyphertext[i]);
                strcat(hexstring, x);
            }
            temp[48] = 0;
            hexstring[48] = 0;
            
            if(strcmp(temp, hexstring) != 0){
                errorprint("ERROR AT VERIFYING FILE:\n");
                printf("IN   : %s\n", temp);
                printf("CALC : %s\n", hexstring);
                status = FALSE;
            }
            
            crcinit = TRUE;
        } else {
            if (crcinit) {
                crc = 0xFFFFFFFF;
                for (int i = 0; i < strlen(temp)-2.0; i++) {
                    calcCRC32_STM(&crc, temp[i]);
                }
                crcinit = FALSE;
            } else {
                for (int i = 0; i < strlen(temp)-2.0; i++) {
                    calcCRC32_STM(&crc, temp[i]);
                }
            }
        }
    }
    return status;
}

void calcCRC32_STM(uint32_t* crc, uint32_t data)
{
    uint32_t cnt;
    
    for(cnt=0;cnt<32;cnt++)
    {
        if((int32_t)(*crc ^ data)<0)
            *crc = (*crc << 1) ^ 0x04C11DB7;
        else
            *crc <<= 1;
        data <<=1;
    }
}

void errorprint(char *s){
    printf("\x1b[31m%s\x1b[0m", s);
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


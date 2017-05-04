//
//  main.h
//  SerialTerminal
//
//  Created by Thomas Hackl on 23.03.17.
//  Copyright © 2017 Thomas Hackl. All rights reserved.
//

#ifndef TH_MAIN_H
#define TH_MAIN_H

//------------------------------------
// INCLUDES
//------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "serial.h"
#include "aes.h"

//------------------------------------
// TYPEDEFS
//------------------------------------
typedef enum{
    MENU = 0,
    LOGGING = 1,
    LISTFILES = 2,
    DOWNLOADFILE = 3,
    SETTIME = 4,
    DELETEFILES = 5,
    BLUETOOTHGRAPH = 6,
    PLOTFILE = 7,
    DISCONNECT = 8,
    ENDSESSION = 9,
    CONNECTPORT = 10
} state_t;

//------------------------------------
// GLOBAL FUNCTIONS
//------------------------------------
void printStateOfEM(int state);

#endif //TH_MAIN_H

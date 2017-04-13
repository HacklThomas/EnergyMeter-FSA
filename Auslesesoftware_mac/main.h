//
//  main.h
//  SerialTerminal
//
//  Created by Thomas Hackl on 23.03.17.
//  Copyright © 2017 Thomas Hackl. All rights reserved.
//

#ifndef main_h
#define main_h

//  --- INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "serial.h"
#include "aes.h"



//  --- ENUMERATIONS/TYPEDEFS
typedef enum{
    MENU = 0,
    LOGGING = 1,
    LISTFILES = 2,
    DOWNLOADFILE = 3,
    SETTIME = 4,
    DELETEFILES = 5,
    BLUETOOTHGRAPH = 6,
    PLOTFILE = 7,
    ENDSESSION = 8,
    CONNECTPORT = 9,
} state_t;

typedef enum{
    FALSE = 0,
    TRUE = 1
}bool_t;


//  --- FUNCTION DECARATIONS
void printStateOfEM(int state);


#endif /* main_h */

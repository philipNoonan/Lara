#ifndef THERM_H
#define THERM_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <string>

#include <libserial/SerialPort.h>
#include <libserial/SerialStream.h>

using LibSerial::SerialPort;
using LibSerial::SerialStream;


class thermal
{
private:

    const char* const SERIAL_PORT_1 = "/dev/ttyUSB0" ;
    SerialPort serial_port;
    SerialStream serial_stream;

    size_t ms_timeout = 250 ;

    void initialize();
    void finalize();

public:
    // Constructor
    thermal();

    // Destructor
    ~thermal();

    std::string run_blocking();


    std::vector<float> get_data();
};

#endif
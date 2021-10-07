#include "thermal.hpp"


// Constructor
thermal::thermal()
{
    // Initialize
    initialize();
}

thermal::~thermal()
{
    // Finalize
    finalize();
}

std::vector<float> thermal::get_data() {
    std::vector<float> outData(1024, 0);

    return outData;
}

void thermal::initialize() {

    // Open the Serial Port at the desired hardware port.
    serial_stream.Open(SERIAL_PORT_1);
    serial_stream.SetBaudRate(LibSerial::BaudRate::BAUD_2000000);
    serial_stream.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
    serial_stream.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
    serial_stream.SetParity(LibSerial::Parity::PARITY_NONE);
    serial_stream.SetStopBits(LibSerial::StopBits::STOP_BITS_1);

    // char read_byte_1 = 'A' ;

    // for (int i = 0; i < 1000; i++) {
    //     serial_port.ReadByte(read_byte_1, 250) ;

    //     std::cout << read_byte_1 << std::endl;
    // }

   
}

void thermal::finalize() {
    serial_port.Close() ;
    serial_stream.Close() ;
}

std::string thermal::run_blocking() {

    std::string output;
    serial_stream >> output;
    
    return output;
}
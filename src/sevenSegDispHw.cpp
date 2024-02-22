#include "sevenSegDispHw.h"

SevenSegDispHw::SevenSegDispHw()
{
}

SevenSegDispHw::~SevenSegDispHw()
{
}

//============================================================> Class methods separator

SevenSegDynamic::SevenSegDynamic(){}

SevenSegDynamic::~SevenSegDynamic(){}

void SevenSegDynamic::fastSend(uint8_t content){

    return;

}

void SevenSegDynamic::fastSend(const uint8_t &segments, const uint8_t &port){

   return;
}

//============================================================> Class methods separator

SevenSegHC595Dyn::SevenSegHC595Dyn()
{
   // Configure display communications pins
    pinMode(_sclk, OUTPUT);
    pinMode(_rclk, OUTPUT);
    pinMode(_dio, OUTPUT);

}

SevenSegHC595Dyn::~SevenSegHC595Dyn()
{
}

void SevenSegHC595Dyn::fastSend(uint8_t content){
    for (int i {7}; i >= 0; i--){   //Send each of the 8 bits representing the character
        if (content & 0x80)
            digitalWrite(_dio, HIGH);
        else
            digitalWrite(_dio, LOW);
        content <<= 1;
        digitalWrite(_sclk, LOW);
        digitalWrite(_sclk, HIGH);
    }

    return;

}

void SevenSegHC595Dyn::fastSend(const uint8_t &segments, const uint8_t &port){

    digitalWrite(_rclk, LOW);
    fastSend(segments);
    fastSend(port);
    digitalWrite(_rclk, HIGH);

   return;
}
//============================================================> Class methods separator

SevenSegHC595Stat::SevenSegHC595Stat()
{
}

SevenSegHC595Stat::~SevenSegHC595Stat()
{
}

//============================================================> Class methods separator

SevenSegTM1637::SevenSegTM1637()
{
}

SevenSegTM1637::~SevenSegTM1637()
{
}


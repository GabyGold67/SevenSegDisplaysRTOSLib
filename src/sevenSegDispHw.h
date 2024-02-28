#ifndef sevenSegDispHw_H
#define sevenSegDispHw_H

#include "Arduino.h"
//============================================================> Class declarations separator

class SevenSegDispHw{
    static uint16_t _dspHwSerialNum;
protected:
    bool _commAnode {true}; //SevenSegDisplays objects will have to retrieve this info to build the right segments for each character
    uint8_t* _digitPosPtr{nullptr};
    uint8_t* _dspBuffPtr{nullptr};
    const uint8_t _dspDigitsQty{}; //Display size in digits    
    uint16_t _dspHwInstNbr{0};

    // virtual void send(uint8_t* digitsBuffer);  //===================>> To be implemented
    // virtual void send(const uint8_t &segments, const uint8_t &port);  //===================>> To be implemented
public:
    SevenSegDispHw();
    SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegDispHw();    
    bool getCommAnnode();
    uint8_t* getDspBuffPtr();
    uint8_t getDspDigits();
    bool setDigitsOrder(uint8_t* newOrderPtr);
    void setDspBuffPtr(uint8_t* newDspBuffPtr);
};

//============================================================> Class declarations separator

class SevenSegDynamic: public SevenSegDispHw{
    static TimerHandle_t _dspRfrshTmrHndl;
    static SevenSegDynamic** _dynDspInstncsLstPtr;
    static void tmrCbRefresh(TimerHandle_t rfrshTmrCbArg);
protected:
    uint8_t _firstRefreshed{0};
    uint8_t* _ioPins{};
    // void fastRefresh();  //===================>> To be implemented
    void refresh();
    void send(uint8_t content);
    void send(const uint8_t &segments, const uint8_t &port);
    TimerHandle_t _svnSgDynTmrHndl{NULL};
public:
    SevenSegDynamic();   //No diferentiated default constructor for this class yet!!
    ~SevenSegDynamic();
    bool begin();
    bool stop();
};

//============================================================> Class declarations separator

class SevenSegStatic: public SevenSegDispHw{

public:
    // SevenSegStatic();    //No diferentiated default constructor for this class yet!!
    ~SevenSegStatic();
};

//============================================================> Class declarations separator

class SevenSegHC595Dyn: public SevenSegDynamic{
private:
    // uint8_t* _ioPins{};
    const uint8_t _sclk {0};
    const uint8_t _rclk {1};
    const uint8_t _dio {2};
protected:
    void send(uint8_t content);
    void send(const uint8_t &segments, const uint8_t &port);
public:
    SevenSegHC595Dyn(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    ~SevenSegHC595Dyn();
};

//============================================================> Class declarations separator

class SevenSegTM1637: public SevenSegStatic{
    const uint8_t maxBrightLvl{0b0111};
    const uint8_t minBrightLvl{0b0000};
protected:
    uint8_t _brightLvl{};
    void send();
public:
    // SevenSegTM1637();
    bool turnOff();
    bool turnOn();
    bool setBrightness(uint8_t &newBrightLevel);
    ~SevenSegTM1637();
};

//============================================================> Class declarations separator

class SevenSegHC595Stat: public SevenSegStatic{
// protected:
public:
    // SevenSegHC595Stat();
    ~SevenSegHC595Stat();
};

// Classes for the TM1638 and Max7219 under implementation need analisys

#endif
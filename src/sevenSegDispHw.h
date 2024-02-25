#ifndef sevenSegDispHw_H
#define sevenSegDispHw_H

#include "Arduino.h"
//============================================================> Class declarations separator

class SevenSegDispHw{
protected:
    const uint8_t _dspDigitsQty{}; //Display size in digits
    bool _commAnode {true}; //SevenSegDisplays objects will have to retrieve this info to build the right segments for each character
    
    uint8_t* _dspBuffPtr{nullptr};
    uint8_t* _digitPosPtr{nullptr};
public:
    SevenSegDispHw();
    SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegDispHw();    
    bool getCommAnnode();
    uint8_t getDspDigits();
    bool setDigitsOrder(uint8_t* newOrderPtr);
    void setDspBuffPtr(uint8_t* newDspBuffPtr);

};

//============================================================> Class declarations separator

class SevenSegDynamic: public SevenSegDispHw{
    // static SevenSegDisplays** _instancesLstPtr;  //To be refactored line
    static SevenSegDynamic** _DynDspInstncsLstPtr;

    static TimerHandle_t _dspRfrshTmrHndl;
    static void tmrCbRefresh(TimerHandle_t rfrshTmrCbArg);

protected:
    uint8_t* _ioPins{};
    void send(uint8_t content);
    uint8_t _firstRefreshed{0};
public:
    SevenSegDynamic();
    ~SevenSegDynamic();
    bool begin();
    void refresh();
    void send(const uint8_t &segments, const uint8_t &port);
    bool stop();
};

//============================================================> Class declarations separator

class SevenSegStatic: public SevenSegDispHw{

public:
    // SevenSegStatic();
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

public:
    SevenSegHC595Dyn(uint8_t* ioPins, uint8_t dspDigits, bool commAnode);
    ~SevenSegHC595Dyn();
    void send(const uint8_t &segments, const uint8_t &port);
};

//============================================================> Class declarations separator

class SevenSegTM1637: public SevenSegStatic{
public:
    // SevenSegTM1637();
    ~SevenSegTM1637();
};

//============================================================> Class declarations separator

class SevenSegHC595Stat: public SevenSegStatic{
// protected:
public:
    // SevenSegHC595Stat();
    ~SevenSegHC595Stat();
};

#endif
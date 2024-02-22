#include "Arduino.h"

//============================================================> Class declarations separator

class SevenSegDispHw{
private:
    
protected:
    uint8_t* _ioPins{nullptr};
    bool _commAnode {true};
    const uint8_t _dspDigits{};
    uint8_t* _digitPosPtr{nullptr};

public:
    SevenSegDispHw();
    ~SevenSegDispHw();
    
};

//============================================================> Class declarations separator

class SevenSegDynamic: public SevenSegDispHw{

protected:
    void fastSend(uint8_t content);

public:
    SevenSegDynamic();
    ~SevenSegDynamic();
    void fastSend(const uint8_t &segments, const uint8_t &port);

};

//============================================================> Class declarations separator

class SevenSegStatic: public SevenSegDispHw{

};

//============================================================> Class declarations separator

class SevenSegHC595Dyn: public SevenSegDynamic{
private:
    const uint8_t _sclk {0};
    const uint8_t _rclk {1};
    const uint8_t _dio {2};
protected:
    void fastSend(uint8_t content);

public:
    SevenSegHC595Dyn();
    ~SevenSegHC595Dyn();
    void fastSend(const uint8_t &segments, const uint8_t &port);
};

//============================================================> Class declarations separator

class SevenSegTM1637: public SevenSegDynamic{
public:
SevenSegTM1637();
~SevenSegTM1637();
};

//============================================================> Class declarations separator

class SevenSegHC595Stat: public SevenSegStatic{
protected:
public:
SevenSegHC595Stat();
~SevenSegHC595Stat();
};


#ifndef SEVENSEG_74HC595_H
#define SEVENSEG_74HC595_H
#include <Arduino.h>

class SevenSeg74HC595 {
    static uint8_t _displaysCount;
    static uint8_t _dspPtrArrLngth;
    static SevenSeg74HC595** _instancesLstPtr;
    static void tmrCbRefresh(TimerHandle_t dspTmrCbArg);
    static TimerHandle_t _dspRfrshTmrHndl;

private:
    uint8_t _waitChar  {0xBF};
    uint8_t _waitCount {0};
    bool _waiting {false};
    unsigned long _waitRate {250};
    unsigned long _waitTimer {0};

protected:
    uint8_t _sclk;
    uint8_t _rclk;
    uint8_t _dio;
    bool _commAnode {true};
    const uint8_t _dspDigits{};
    uint8_t* _digitPosPtr{nullptr};
    uint8_t* _digitPtr{nullptr};
    bool* _blinkMaskPtr{nullptr};

    int32_t _dspValMax{};
    int32_t _dspValMin{};

    const unsigned long _minBlinkRate{100};
    const unsigned long _maxBlinkRate{2000};
    SevenSeg74HC595* _dispInstance;
    uint8_t _dispInstNbr{0};
    uint8_t _firstRefreshed{0};
    bool _blinking{false};
    bool _blinkShowOn{false};
    unsigned long _blinkOffRate{500};
    unsigned long _blinkOnRate{500};
    unsigned long _blinkTimer{0};

    String _charSet{"0123456789AabCcdEeFGHhIiJLlnOoPqrStUuY-_=~* ."}; // for using indexOf() method
    uint8_t _charLeds[45] = {   //Values valid for a Common Anode display. For a Common Cathode display values must be logically bit negated
        0xC0, // 0
        0xF9, // 1
        0xA4, // 2
        0xB0, // 3
        0x99, // 4
        0x92, // 5
        0x82, // 6
        0xF8, // 7
        0x80, // 8
        0x90, // 9
        0x88, // A
        0xA0, // a
        0x83, // b
        0xC6, // C
        0xA7, // c
        0xA1, // d
        0x86, // E
        0x84, // e
        0x8E, // F
        0xC2, // G
        0x89, // H
        0x8B, // h
        0xF9, // I
        0xFB, // i
        0xF1, // J
        0xC7, // L
        0xCF, // l
        0xAB, // n
        0xC0, // O
        0xA3, // o
        0x8C, // P
        0x98, // q
        0xAF, // r
        0x92, // S
        0x87, // t
        0xC1, // U
        0xE3, // u
        0x91, // Y
        0xBF, // Minus -
        0xF7, // Underscore _
        0xB7, // Low =
        0xB6, //~ for Equivalent symbol
        0x9C, // °
        0xFF, // Space
        0x7F  //.
    };    
    
    uint8_t _space {0xFF};
    uint8_t _dot {0x7F};
    String _zeroPadding{""};
    String _spacePadding{""};

    void setAttrbts();
    void fastSend(uint8_t content);
    void send(const uint8_t &content);
    void updBlinkState();
    void updWaitState();

public:
    SevenSeg74HC595();
    SevenSeg74HC595(uint8_t sclk, uint8_t rclk, uint8_t dio, bool commAnode = true, const uint8_t dspDigits = 4);
    ~SevenSeg74HC595();
    bool begin();
    bool blink();
    bool blink(const unsigned long &onRate, const unsigned long &offRate = 0);
    void clear();
    bool doubleGauge(const int &levelLeft, const int &levelRight, char labelLeft = ' ', char labelRight = ' ');
    void fastRefresh();
    void fastSend(const uint8_t &segments, const uint8_t &port);
    bool gauge(const int &level, char label = ' ');
    bool gauge(const double &level, char label = ' ');
    uint8_t getDigitsQty();
    uint32_t getDspValMax();
    uint32_t getDspValMin();
    uint8_t getInstanceNbr();
    unsigned long getMaxBlinkRate();
    unsigned long getMinBlinkRate();
    bool isBlinking();
    bool isWaiting();
    bool noBlink();
    bool noWait();
    bool print(String text);
    bool print(const int32_t &value, bool rgtAlgn = false, bool zeroPad = false);
    bool print(const double &value, const unsigned int &decPlaces, bool rgtAlgn = false, bool zeroPad = false);
    void refresh();
    void resetBlinkMask();
    void send(const uint8_t &segments, const uint8_t &port);
    void setBlinkMask(const bool blnkPort[]);
    bool setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate = 0);
    bool setDigitsOrder(uint8_t* newOrderPtr);
    bool setWaitChar (const char &newChar);
    bool setWaitRate(const unsigned long &newWaitRate);
    bool stop();
    bool wait(const unsigned long &newWaitRate = 0);
    bool write(const uint8_t &segments, const uint8_t &port);
    bool write(const String &character, const uint8_t &port);

};

//============================================================> Class declarations separator

class ClickCounter{
private:
    SevenSeg74HC595 _display;
    int _count{0};
    int _beginStartVal{0};
    bool _countRgthAlgn{true};
    bool _countZeroPad{false};

public:
    ClickCounter(uint8_t ccSclk, uint8_t ccRclk, uint8_t ccDio, bool rgthAlgn = true, bool zeroPad = false, bool commAnode = true, const uint8_t dspDigits = 4);
    ~ClickCounter();
    bool blink();
    bool blink(const unsigned long &onRate, const unsigned long &offRate = 0);
    void clear();
    bool countBegin(int32_t startVal = 0);
    bool countDown(int32_t qty = 1);
    bool countReset();
    bool countRestart(int32_t restartValue = 0);
    bool countStop();
    bool countToZero(int32_t qty = 1);
    bool countUp(int32_t qty = 1);
    int32_t getCount();
    int32_t getStartVal();
    bool noBlink();
    bool setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate = 0);
    bool updDisplay();

};

#endif
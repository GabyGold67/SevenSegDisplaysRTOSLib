#include <Arduino.h>
#include "SevenSeg-74HC595.h"

//Changes:
//_ Make the instancesList a dynamic array in the heap, attribute of the class, with getter and setters

uint8_t SevenSeg74HC595::displaysCount = 0;
SevenSeg74HC595 *instancesList[MAX_PTR_ARRAY] {nullptr};

void SevenSeg74HC595::tmrCbRefresh(TimerHandle_t dspTmrCbArg){
   //SevenSeg74HC595 *argObj = (SevenSeg74HC595*)pvTimerGetTimerID(dspTmrCbArg);
   //Timer Callback to keep the display lit by calling each display's fastRefresh() method
    for(uint8_t i {0}; i < MAX_PTR_ARRAY; i++)
        if (instancesList[i] != nullptr){
            instancesList[i]->fastRefresh();
        }

    return;
}

SevenSeg74HC595::SevenSeg74HC595(uint8_t sclk, uint8_t rclk, uint8_t dio, bool commAnode, const uint8_t dspDigits)
:_sclk{sclk}, _rclk{rclk}, _dio{dio}, _commAnode{commAnode}, _dspDigits{dspDigits}
{
    pinMode(_sclk, OUTPUT);
    pinMode(_rclk, OUTPUT);
    pinMode(_dio, OUTPUT);

    _digitPtr = new uint8_t(_dspDigits);    //Create a charcters array of the required size to hold the display content for each digit
    _blinkMaskPtr = new bool(_dspDigits);   //Create a boolean array of the required size to hold the blink mask
    if(_dspDigits > 1){
        _dspValMin = 1;
        for (uint8_t i{0}; i < (_dspDigits - 1); i++){
            _dspValMin *= 10;
        }
        _dspValMin =(-1)*(_dspValMin - 1);

    }
    else{
        _dspValMin = 0;
    }
    _dspValMax = 1;
    for (uint8_t i{0}; i < _dspDigits; i++){
        _dspValMax *= 10;
        _zeroMask+="0";
        _spaceMask+=" ";
        *(_blinkMaskPtr + i) = true;
    }
    _dspValMax -=1;


    _dispInstNbr = displaysCount++;
    _dispInstance = this;

    if(!_commAnode){
        _waitChar = ~_waitChar;
        _space = ~_space;
        _dot = ~_dot;
        for(int i {0}; i < (int)_charSet.length(); i++)
            _charLeds[i] = ~_charLeds[i];
    }
    
    clear();
}

SevenSeg74HC595::~SevenSeg74HC595(){
    delete _digitPtr;    
}

bool SevenSeg74HC595::begin(){
    bool result {false};

    //Verify if the timer interrupt service was started by checking if there are displays added to the pointers vector
   if (!_dspRfrshTmrHndl){
      //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (_dspDigits * 30Hz)
      _dspRfrshTmrHndl = xTimerCreate(
         "Display Refresh",
         pdMS_TO_TICKS((int)(1000/(50*_dspDigits))),
         pdTRUE,  //Autoreload
         NULL, //TimerID, data to be passed to the callback function
         tmrCbRefresh);  //Callback function

      assert (_dspRfrshTmrHndl);      
   }
    
    // Include the object's pointer to the array of pointers to be serviced by the timer Callback, 
    // if there's available space to hook the object will return true
    for(uint8_t i {0}; i < MAX_PTR_ARRAY; i++)
        if (instancesList[i] == nullptr){
            instancesList[i] = _dispInstance;
            result = true;
            break;
        }
        else if (instancesList[i] == _dispInstance){
            // The object pointer was already in the vector, the method will return true because the purpose of including it in te array 
            // was achieved, although some mistake must have been done in the logic to try to include twice the same display
            result = true;
            break;
        }
      if(result && (!xTimerIsTimerActive(_dspRfrshTmrHndl)))
         xTimerStart(_dspRfrshTmrHndl, portMAX_DELAY);

    return result;
}

bool SevenSeg74HC595::blink(){
   bool result {false};

   if (!_blinking){
      _blinkTimer = 0;
      _blinkShowOn = false;
      _blinking = true;
      result = true;
   }

   return result;
}

bool SevenSeg74HC595::blink(const unsigned long &onRate, const unsigned long &offRate){
    bool result {false};

   if (!_blinking){
      if (offRate == 0)
         result = setBlinkRate(onRate, onRate);
      else
         result = setBlinkRate(onRate, offRate);        
      if (result)
         result = blink();
   }

   return result;
}

void SevenSeg74HC595::clear(){
   //Cleans the contents of the internal display buffer (All leds off for all digits)
   for (int i{0}; i < _dspDigits; i++){
      *(_digitPtr + i) = _space;
   }
   refresh();

   return;
}

bool SevenSeg74HC595::doubleGauge(const int &levelLeft, const int &levelRight, char labelLeft, char labelRight){
    bool displayable{true};
    String readOut{""};

    if ((levelLeft < 0) || (levelRight < 0) || (levelLeft > 3 || (levelRight > 3))) {
        clear();
        displayable = false;
    }
    else {
        readOut += labelLeft;
        if (readOut == "")
            readOut = " ";
        switch (levelLeft) {
            case 0:
                readOut += " ";
                break;
            case 1:
                readOut += "_";
                break;
            case 2:
                readOut += "=";
                break;
            case 3:
                readOut += "~";
                break;
        };
        if(_dspDigits > 4){
            for (int i{0}; i < (_dspDigits-4)/2; i++)
               readOut += " ";
        }
        readOut += labelRight;
        if (readOut.length() == 2)
            readOut += " ";
        switch (levelRight) {
            case 0:
                readOut += " ";
                break;
            case 1:
                readOut += "_";
                break;
            case 2:
                readOut += "=";
                break;
            case 3:
                readOut += "~";
                break;
        };
        displayable = print(readOut);
    }

    return displayable;
}

void SevenSeg74HC595::fastRefresh(){
   bool tmpLogic {true};

   updBlinkState();
   updWaitState();
   if ((_blinking == false) || (_blinkShowOn == true)) {
      fastSend(*(_digitPtr+firstRefreshed), 1 << firstRefreshed);
   }

   if(_blinking && !_blinkShowOn){
      for(int i{0}; i<_dspDigits; i++)
         tmpLogic = tmpLogic && *(_blinkMaskPtr + i);
      if (!tmpLogic){   //At least one digit is set NOT TO BLINK
         if(!*(_blinkMaskPtr+firstRefreshed))
            fastSend(*(_digitPtr+firstRefreshed), 1 << firstRefreshed);
      }
   }
   firstRefreshed++;
   if (firstRefreshed == _dspDigits)
      firstRefreshed = 0;

   return;
}

void SevenSeg74HC595::fastSend(uint8_t content){
    //Sends the byte value (char <=> unsigned short int) to the 4 7-segment display bit by bit
    //by direct manipulation of the microcontroller pins. There is no time added, so the total time
    //consumed to shift an entire byte is supposed to be the lowest achievable in this level of abstraction.
    //So this is the method suggested to be called from an ISR to keep the ISR time consumed to the lowest

    for (int i {7}; i >= 0; i--){
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

void SevenSeg74HC595::fastSend(volatile const uint8_t &segments, const uint8_t &port){
    // Sends the character 'segments' to the digit 'port' of the display
    // Content and Port must be sent in two sequencial parts, character first, port second
    // so this overloaded two char fastSend() method uses the one char fastSend() method twice and then moves
    // up the RCLK pin to present the content in the shift register. This method doesn't add time delays, 
    //So this is the method suggested to be called from an ISR to keep the ISR time consumed to the lowest

    digitalWrite(_rclk, LOW);
    fastSend(segments);
    fastSend(port);
    digitalWrite(_rclk, HIGH);

    return;
}

bool SevenSeg74HC595::gauge(const int &level, char label){
    bool displayable{true};
    String readOut{""};

    if ((level < 0) || (level > 3)) {
        clear();
        displayable = false;
    }
    else {
        readOut += label;
        if (readOut == "")
            readOut = " ";
        switch (level) {
            case 0:
                readOut += "   ";
                break;
            case 1:
                readOut += "_  ";
                break;
            case 2:
                readOut += "_= ";
                break;
            case 3:
                readOut += "_=~";
                break;
        };
        displayable = print(readOut);
    }

    return displayable;
}

bool SevenSeg74HC595::gauge(const double &level, char label) {
    bool displayable{true};
    int intLevel{0};

    if ((level < 0.0) || (level > 1.0)) {
        clear();
        displayable = false;
    }
    else {
        if (level < 0.25)
            intLevel = 0;
        else if (level < 0.50)
            intLevel = 1;
        else if (level < 0.75)
            intLevel = 2;
        else
            intLevel = 3;
        displayable = gauge(intLevel, label);
    }

    return displayable;
}

uint8_t SevenSeg74HC595::getInstanceNbr(){

    return _dispInstNbr;
}

unsigned long SevenSeg74HC595::getMaxBlinkRate(){
    
    return _maxBlinkRate;
}

unsigned long  SevenSeg74HC595::getMinBlinkRate(){

    return _minBlinkRate;
}

bool SevenSeg74HC595::isBlinking(){

   return _blinking;
}

bool SevenSeg74HC595::isWaiting(){

    return _waiting;
}

bool SevenSeg74HC595::noBlink(){
   bool result {false};

   if(_blinking){
      _blinking = false;
      _blinkTimer = 0;
      _blinkShowOn = true;
      result = true;
   }

   return result;
}

bool SevenSeg74HC595::noWait(){
   bool result {false};

   if (_waiting){
      _waiting = false;
      _waitTimer = 0;
      clear();
      result = true;
   }

   return result;
}

bool SevenSeg74HC595::print(String text){
    bool displayable{true};
    int position{-1};
    String tempText{""};
    uint8_t temp7SegData[_dspDigits];
    uint8_t tempDpData[_dspDigits];

    for (int i{0}; i < _dspDigits; i++){
        temp7SegData[i] = _space;
        tempDpData[i] = _space;
    }
    // Finds out if there are '.' in the string to display, creates a mask to add them to the display
    // and takes them out of the string to process the chars/digits
    for(unsigned int i{0}; i < text.length(); ++i){
        if (text.charAt(i) != '.')
            tempText += text.charAt(i);
        else{
            if (i == 0 || text.charAt(i-1) == '.')
                tempText += " ";
            if(tempText.length() <= _dspDigits)
                tempDpData[_dspDigits - tempText.length()] = _dot;
        }
    }
    text = tempText;

    if (text.length() <= _dspDigits){
        for (unsigned int i {0}; i < text.length(); ++i){
            position = _charSet.indexOf(text.charAt(i));
            if (position > -1) {
                // Character found for translation
                temp7SegData[(_dspDigits - 1) - i] = _charLeds[position];
            }
            else {
                displayable = false;
                break;
            }
        }
    }
    else {
        displayable = false;
    }

    if (displayable) {
        for (int i{0}; i < _dspDigits; ++i)
            *(_digitPtr + i) = temp7SegData[i] & tempDpData[i];
    }

    return displayable;
}

bool SevenSeg74HC595::print(const int &value, bool rgtAlgn, bool zeroPad){
    bool displayable{true};
    String readOut{""};
    // if ((value < MIN_DISP_VALUE) || (value > MAX_DISP_VALUE)) {
    // if ((value < _dspValMin) || (value > MAX_DISP_VALUE)) {
    if ((value < _dspValMin) || (value > _dspValMax)) {
        clear();
        displayable = false;
    }
    else {
        readOut = String(abs(value));
        if (rgtAlgn) {
            if (zeroPad)
                readOut = _zeroMask + readOut;
            else
                readOut = _spaceMask + readOut;


            if (value >= 0)
                readOut = readOut.substring(readOut.length() - _dspDigits);
            else
                readOut = readOut.substring(readOut.length() - (_dspDigits - 1));
        }
        if (value < 0)
            readOut = "-" + readOut;
        displayable = print(readOut);
    }

    return displayable;
}

bool SevenSeg74HC595::print(const double &value, const unsigned int &decPlaces, bool rgtAlgn, bool zeroPad){
    bool displayable{true};
    String readOut{""};
    String pad{""};
    int start{0};

    if (decPlaces == 0)
        displayable = print(int(value), rgtAlgn, zeroPad);
    else if ((value < _dspValMin) || (value > _dspValMax) || (decPlaces > _dspDigits)) {
        displayable = false;
        clear();
    }
    else if ((decPlaces + String(int(value)).length()) > (((value < 0) && (value > (-1))) ? (_dspDigits - 1) : _dspDigits)) {
        displayable = false;
        clear();
    }
    else {
        if (value < 0 && value > -1)
            readOut = "-";
        readOut += String(int(value)) + ".";
        start = String(value).indexOf('.') + 1;
        readOut += (String(value) + _zeroMask).substring(start, start + decPlaces);

        if (rgtAlgn) {
            if (readOut.length() < _dspDigits + 1) {
                if (value < 0)
                    pad += "-";
                if (zeroPad)
                    pad += _zeroMask;
                else
                    pad += _spaceMask;
                if (value < 0)
                    readOut = pad.substring(0, (_dspDigits+1) - (readOut.length()-1)) + readOut.substring(1);
                else
                    readOut = pad.substring(0, (_dspDigits+1) - (readOut.length())) + readOut;
                readOut = readOut.substring(0, _dspDigits+1);
            }
        }

        displayable = print(readOut);
    }

    return displayable;
}

void SevenSeg74HC595::refresh(){
   static int firstRefreshed {0};
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

    updBlinkState();
    updWaitState();

    if((_blinking == false)||(_blinkShowOn == true)){
        for (int i {0}; i < _dspDigits; i++){
            tmpDigToSend = *(_digitPtr+((i + firstRefreshed) % _dspDigits));
            send(tmpDigToSend, 1<<(i + firstRefreshed) % _dspDigits);
        }
    }
    if(_blinking && !_blinkShowOn){
      for(int i{0}; i < _dspDigits; i++)
         tmpLogic = tmpLogic && *(_blinkMaskPtr + i);
        if (!tmpLogic){   //At least one digit is set NOT TO BLINK
            for (int i {0}; i < _dspDigits; i++){
                if(!*(_blinkMaskPtr+((i + firstRefreshed) % _dspDigits))){
                    tmpDigToSend = *(_digitPtr+((i + firstRefreshed) % _dspDigits));
                    send(tmpDigToSend, 1<<(i + firstRefreshed) % _dspDigits);
                }
            }
        }
    }
    firstRefreshed++;
    if (firstRefreshed == _dspDigits)
        firstRefreshed = 0;

    return;
}

void SevenSeg74HC595::resetBlinkMask(){
   for (int i{0}; i < _dspDigits; i++)
      *(_blinkMaskPtr + i) = true;

   return;
}

void SevenSeg74HC595::send(const uint8_t &content){
    //Sends the byte value (char <=> unsigned short int) to the 4 7-segment display bit by bit
    //by using the shiftOut() function. The time added (or not) to send it bit is unknown, so the total time
    //consumed to shift an entire byte is unknown, issue that must be considered when the method is invoked 
    //from an ISR and multiple times depending on the qty of displays being used
    shiftOut(_dio, _sclk, MSBFIRST, content);
}

void SevenSeg74HC595::send(const uint8_t &segments, const uint8_t &port){
// Sends the character 'segments' to the digit 'port' of the display
// Content and Port must be sent in two sequencial parts, character first, port second
// so this overloaded two char send method uses the one char send method twice and then moves
// up the RCLK pin to present the content in the shift register. This method depends on the shiftOut() function
// so consumed time must be considered
    digitalWrite(_rclk, LOW);
    send(segments);
    send(port);
    digitalWrite(_rclk, HIGH);
}

void SevenSeg74HC595::setBlinkMask(const bool blnkPort[]){
   for (int i{0}; i < _dspDigits; i++)
      *(_blinkMaskPtr + i) = blnkPort[i];

    return;
}

bool SevenSeg74HC595::setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate){
    bool result {false};
    
    if ((newOnRate >= _minBlinkRate) && newOnRate <= _maxBlinkRate) {
        if ((newOffRate == 0) || ((newOffRate >= _minBlinkRate) && (newOffRate <= _maxBlinkRate))) {
            //if the new blinkRate is in the accepted range, set the blinkRate for symmetric rate
            if(_blinkOnRate != newOnRate)
               _blinkOnRate = newOnRate;
            if (newOffRate == 0)
                _blinkOffRate = newOnRate;
            else
                _blinkOffRate = newOffRate;
            result =  true;
        }
    }
    //The value was outside valid range, keep the existing rate and report the error by returning false
    
    return result;  
}

bool SevenSeg74HC595::setWaitChar (const char &newChar){
    bool result {false};
    int position {-1};

   if(_waitChar != newChar){
      position = _charSet.indexOf(newChar);
      if (position > -1) {
         _waitChar = _charLeds[position];
         result = true;
      }
   }
   else{
      result = true;
   }

   return result;
}

bool SevenSeg74HC595::setWaitRate(const unsigned long &newWaitRate){
    bool result {false};

    if ((newWaitRate >= _minBlinkRate) && newWaitRate <= _maxBlinkRate) {
        //if the new waitRate is within the accepted range, set it
        if(_waitRate != newWaitRate)
         _waitRate = newWaitRate;
        result =  true;        
    }
    //The value was outside valid range, keep the existing rate and report the error by returning false
    
    return result;
}

bool SevenSeg74HC595::stop() {
    //This object's pointer will be deleted from the arrays of pointers. If the array has no more valid pointers the timer will be stopped to avoid loosing processing time.
    bool pointersFound(false);
    bool result {false};

    for(uint8_t i {0}; i < MAX_PTR_ARRAY; i++)
        if (instancesList[i] == _dispInstance){
            instancesList[i] = nullptr;
            result = true;
        }
        else if (instancesList[i] != nullptr){
            // There are still objects pointers in the vector, so the refresh display services must continue active
            pointersFound = true;
        }

    if (!pointersFound){
        //There are no more display instances active, there's no point in keeping the ISR active, the timer is stopped and the interrupt service detached
      xTimerStop(_dspRfrshTmrHndl, portMAX_DELAY);
      xTimerDelete(_dspRfrshTmrHndl, portMAX_DELAY);
      _dspRfrshTmrHndl = nullptr;
    }   

    return result;
}

void SevenSeg74HC595::updBlinkState(){
   //Must be replaced with a xTimer that keeps flip-floping the _blinkShowOn value
   if (_blinking == true){
      if (_blinkShowOn == false) {
         if (_blinkTimer == 0){
            //turn off the digits by sending directly a blank to each port, without affecting the _digit[] buffer
            for (int i{0}; i < _dspDigits; i++)
               send(_space, 0b01 << i);
            _blinkTimer = xTaskGetTickCount() / portTICK_RATE_MS; //Starts the count for the blinkRate control
            
         }
         else if((xTaskGetTickCount() / portTICK_RATE_MS - _blinkTimer)> _blinkOffRate){
            _blinkTimer = 0;
            _blinkShowOn = true;
         }
      }
      else{
         if (_blinkTimer == 0){
            _blinkTimer = xTaskGetTickCount() / portTICK_RATE_MS;
         }
         else if((xTaskGetTickCount() / portTICK_RATE_MS - _blinkTimer) > _blinkOnRate){
            _blinkTimer = 0;
            _blinkShowOn = false;
         }
      }
   }

   return;
}

void SevenSeg74HC595::updWaitState(){
   if (_waiting == true){
      if (_waitTimer == 0){
         for (int i{0}; i < _dspDigits; i++)
            *(_digitPtr+i) = _space;
         _waitTimer = xTaskGetTickCount()/portTICK_RATE_MS;
      }
      else if((xTaskGetTickCount()/portTICK_RATE_MS - _waitTimer) > _waitRate){
         for (int i{_dspDigits - 1}; i >= 0; i--){

            if(( _dspDigits - i) <= _waitCount)
               *(_digitPtr+i) = _waitChar;
            else
               *(_digitPtr+i) = _space;
         }
         _waitCount++;
         if (_waitCount == (_dspDigits + 1))
            _waitCount = 0;
         _waitTimer = xTaskGetTickCount()/portTICK_RATE_MS;
      }
   }

   return;
}

bool SevenSeg74HC595::wait(const unsigned long &newWaitRate){
   bool result {true};
   
   if (_waiting == false){
      if (newWaitRate != 0){
         if ((newWaitRate >= _minBlinkRate) && (newWaitRate <= _maxBlinkRate)){
            if(_waitRate != newWaitRate)
               _waitRate = newWaitRate;         
         }
         else{
            result = false;
         }
      }
      if (result == true){
         _waitTimer = 0;
         _waitCount = 0;
         _waiting = true;
      }
   }
   else{
      result = false;
   }

   return result;
}

bool SevenSeg74HC595::write(const uint8_t &segments, const uint8_t &port){
    bool result {false};
    
    if (port < _dspDigits){
        *(_digitPtr+port) = segments;
        result = true;
    }
    
    return result;
}

bool SevenSeg74HC595::write(const String &character, const uint8_t &port){
    bool result {false};
    int position {-1};
    
    if (port < _dspDigits){
            position = _charSet.indexOf(character);
            if (position > -1) {
                // Character found for translation
                *(_digitPtr+port) = _charLeds[position];
                result = true;
            }
    }

    return result;
}

//============================================================> Class methods separator

ClickCounter::ClickCounter(uint8_t ccSclk, uint8_t ccRclk, uint8_t ccDio, bool rgthAlgn, bool zeroPad,  bool rollOver, bool commAnode)
:SevenSeg74HC595(ccSclk, ccRclk, ccDio, commAnode), _countRgthAlgn {rgthAlgn}, _countZeroPad {zeroPad}, _rollOver{rollOver}
{
    //Class constructor
}

bool ClickCounter::blink(){

    return SevenSeg74HC595::blink();
}

bool ClickCounter::blink(const unsigned long &onRate, const unsigned long &offRate){

    return SevenSeg74HC595::blink(onRate, offRate);
}

bool ClickCounter::countBegin(int startVal){
   bool result{false};

    if (SevenSeg74HC595::begin() == true){
        result = countRestart(startVal);
        if (result)
            _beginStartVal = startVal;
    }

   return result;
}

bool ClickCounter::countDown(int qty){
    bool result {false};
    qty = abs(qty);

    if((_count - qty) >= _dspValMin){
        _count -= qty;
        result = updDisplay();
    }

    return result;
}

bool ClickCounter::countReset(){

    return countRestart(_beginStartVal);
}
    
bool ClickCounter::countRestart(int restartValue){
   bool result{false};

   if ((restartValue >= _dspValMin) && (restartValue <= _dspValMax)){
      _count = restartValue;
      result = updDisplay();
   }

   return result;
}

bool ClickCounter::countStop(){
    
    return SevenSeg74HC595::stop();
}

bool ClickCounter::countToZero(int qty){
    bool result {false};

    if (_count > 0)
        result = countDown(qty);
    else if (_count < 0)
        result = countUp(qty);
    
    return result;
}

bool ClickCounter::countUp(int qty){
    bool result {false};
    qty = abs(qty);

    // if((_count + qty) <= MAX_DISP_VALUE){
    if((_count + qty) <= _dspValMax){
        _count += qty;
        result = updDisplay();
    }

    return result;
}

int ClickCounter::getCount(){

    return _count;
}

int ClickCounter::getStartVal(){

    return _beginStartVal;
}

bool ClickCounter::noBlink(){

    return SevenSeg74HC595::noBlink();
}

bool ClickCounter::setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate){

    return SevenSeg74HC595::setBlinkRate(newOnRate, newOffRate);
}

bool ClickCounter::updDisplay(){

    return print(_count, _countRgthAlgn, _countZeroPad);
}

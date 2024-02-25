#include "sevenSegDisplays.h"

uint8_t SevenSegDisplays::_displaysCount = 0;
uint8_t SevenSegDisplays::_dspPtrArrLngth = 10;
SevenSegDisplays** SevenSegDisplays::_instancesLstPtr = nullptr;

TimerHandle_t SevenSegDisplays::_blinkTmrHndl = nullptr;  //====>> For future use
TimerHandle_t SevenSegDisplays::_waitTmrHndl = nullptr;   //====>> For future use

SevenSegDisplays::SevenSegDisplays()
{
}

SevenSegDisplays::SevenSegDisplays(SevenSegDispHw dspUndrlHw)
:_dspUndrlHw{dspUndrlHw}
{
    _dspDigitsQty = dspUndrlHw.getDspDigits();
    _dspBuffPtr = new uint8_t[_dspDigitsQty];

    setAttrbts();
    clear();
}

SevenSegDisplays::~SevenSegDisplays(){
    // stop(); //Frees the slot in the pointers array for the refresh timer, and stops the timer if there are no valid pointers left
    delete [] _dspBuffPtr;  //Free the resources of the display digits buffer
    delete [] _dspAuxBuffPtr;   //Free the resources of the auxiliary display digits buffer (to keep a copy of the dspBuffer contents for blinking, waiting, etc.)
    delete [] _blinkMaskPtr;    //Free the resources of the blink mask buffer

    --_displaysCount;
}

bool SevenSegDisplays::blink(){
   bool result {false};

   if (!_blinking){
      _blinkTimer = 0;  //Start blinking

/*bool SevenSegDisplays::begin(){
    bool result {false};
    int frstFreeSlot{-1};
    BaseType_t tmrModResult {pdFAIL};

    //Verify if the timer interrupt service was started by checking if the timer Handle is valid (and there are displays added to the pointers vector)
    if (!_dspRfrshTmrHndl){
      //Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Max qty of digits * 30Hz)
        _dspRfrshTmrHndl = xTimerCreate(
            "Display Refresh",
            pdMS_TO_TICKS((int)(1000/(30 * MAX_DIGITS_DISPLAYS))),
            pdTRUE,  //Autoreload
            NULL,   //TimerID, data to be passed to the callback function
            SevenSegDisplays::tmrCbRefresh  //Callback function
        );
    }

    if (_dspRfrshTmrHndl != NULL){
        // Include the object's pointer to the array of pointers to be serviced by the timer Callback, 
        // If this is the first instance created, create the array of instances in Heap    
        if(_instancesLstPtr == nullptr){
            _instancesLstPtr = new SevenSegDisplays* [_dspPtrArrLngth];
            for(int i{0}; i < _dspPtrArrLngth; i++)
                *(_instancesLstPtr + i) = nullptr;
        }            
        //Look for a free slot in the array of pointers to displays to be refreshed or find if the display is already in the array
        for(uint8_t i {0}; i < _dspPtrArrLngth; i++){
            if (*(_instancesLstPtr + i) == nullptr){    //Save the first free slot of the list in case it's needed later
                if(frstFreeSlot == -1)
                    frstFreeSlot = i;
            }
            else if (*(_instancesLstPtr + i) == _dspInstance){
                result = true;  //The display was included in the list of instances to keep refreshed
                break;
            }        
        }
        if(!result){    // The pointer to this object wasn't in the list, so it must be added
            if(frstFreeSlot > -1){
                *(_instancesLstPtr + frstFreeSlot) = _dspInstance;
                result = true;
            }
        }
    }  
    if(result && (!xTimerIsTimerActive(_dspRfrshTmrHndl))){
        // The instance was added to the array, but the timer wasn't started, start the timer
        tmrModResult = xTimerStart(_dspRfrshTmrHndl, portMAX_DELAY);
        if (tmrModResult == pdPASS)
            result = true;
    }

    return result;
}*/
    _dspBuffPtr = new uint8_t[_dspDigitsQty];
    saveDspBuff();
      _blinkShowOn = false;
      _blinking = true;
      result = true;
   }

   return result;
}

bool SevenSegDisplays::blink(const unsigned long &onRate, const unsigned long &offRate){
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

unsigned long SevenSegDisplays::blinkTmrGCD(unsigned long blnkOnTm, unsigned long blnkOffTm){
    /*returning values:
        0: One of the input values was 0
        1: No GDC greater than 1
        Other: This value would make the blink timer save resources by checking the blink time as little as possible*/
    unsigned long result{ 0 };

    if ((blnkOnTm != 0) && (blnkOffTm != 0)) {
        if (blnkOnTm == blnkOffTm) {
            result = blnkOnTm;
        }
        else if ((blnkOnTm % blnkOffTm == 0) || (blnkOffTm % blnkOnTm == 0)) {
            result = (blnkOffTm < blnkOnTm)? blnkOffTm : blnkOnTm;
        }

        if (result == 0) {
            for (unsigned long int i{ (blnkOnTm < blnkOffTm) ? blnkOnTm : blnkOffTm }; i > 0; i--) {
                if ((blnkOnTm % i == 0) && (blnkOffTm % i == 0)) {
                    result = i;
                    break;
                }
            }
        }
    }

    return result;
}

void SevenSegDisplays::clear(){
   //Cleans the contents of the internal display buffer (All leds off for all digits)
   for (int i{0}; i < _dspDigitsQty; i++){
      if(*(_dspBuffPtr + i) != _space){
        *(_dspBuffPtr + i) = _space;
        _dspBuffChng = true;
      }
   }

   return;
}

bool SevenSegDisplays::doubleGauge(const int &levelLeft, const int &levelRight, char labelLeft, char labelRight){
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
        if(_dspDigitsQty > 4){
            for (int i{0}; i < (_dspDigitsQty - 4)/2; i++)
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

/*void SevenSegDisplays::fastRefresh(){
   bool tmpLogic {true};

   updBlinkState();
   updWaitState();
   if ((_blinking == false) || (_blinkShowOn == true)) {
    //   send(*(_dspBuffPtr + _firstRefreshed), uint8_t(1) << *(_digitPosPtr + _firstRefreshed));
   }
   else if(_blinking && !_blinkShowOn){
      for(uint8_t i{0}; i<_dspHwPtr->getDspDigits(); i++)
         tmpLogic = tmpLogic && *(_blinkMaskPtr + i);
      if (!tmpLogic){   //At least one digit is set NOT TO BLINK
         if(!*(_blinkMaskPtr + _firstRefreshed))
            // send(*(_dspBuffPtr + _firstRefreshed), uint8_t(1) << *(_digitPosPtr + _firstRefreshed));
      }
   }
   ++_firstRefreshed;
   if (_firstRefreshed == _dspHwPtr->getDspDigits())
      _firstRefreshed = 0;

   return;
}*/

bool SevenSegDisplays::gauge(const int &level, char label){
    bool displayable{true};
    String readOut{""};

    clear();
    if (((level < 0) || (level > 3)) || (_dspDigitsQty < 4)) {
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

bool SevenSegDisplays::gauge(const double &level, char label) {
    bool displayable{true};
    int intLevel{0};

    if (((level < 0.0) || (level > 1.0)) || (_dspDigitsQty < 4)) {
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

/*uint8_t SevenSegDisplays::getDigitsQty(){

    return _dspDigitsQty;
}*/

uint32_t SevenSegDisplays::getDspValMax(){

    return _dspValMax;
}

uint32_t SevenSegDisplays::getDspValMin(){

    return _dspValMin;
}

uint8_t SevenSegDisplays::getInstanceNbr(){

    return _dspInstNbr;
}

unsigned long SevenSegDisplays::getMaxBlinkRate(){
    
    return _maxBlinkRate;
}

unsigned long  SevenSegDisplays::getMinBlinkRate(){

    return _minBlinkRate;
}

bool SevenSegDisplays::isBlank(){
    uint8_t result{true};

    for (int i{0}; i < _dspDigitsQty; i++){
        if(*(_dspBuffPtr + i) != _space){
            result = false;
            break;
        }
    }

    return result;
}

bool SevenSegDisplays::isBlinking(){

   return _blinking;
}

bool SevenSegDisplays::isWaiting(){

    return _waiting;
}

bool SevenSegDisplays::noBlink(){
   bool result {false};

   if(_blinking){
      _blinking = false;
      _blinkTimer = 0;
      _blinkShowOn = true;
      result = true;
   }

   return result;
}

bool SevenSegDisplays::noWait(){
   bool result {false};

   if (_waiting){
      _waiting = false;
      _waitTimer = 0;
      clear();
      result = true;
   }

   return result;
}

bool SevenSegDisplays::print(String text){
    bool displayable{true};
    int position{-1};
    // uint8_t _dspDigitsQty{_dspHwPtr->getDspDigits()};

    String tempText{""};
    uint8_t temp7SegData[_dspDigitsQty];
    uint8_t tempDpData[_dspDigitsQty];

    for (int i{0}; i < _dspDigitsQty; i++){
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
            if(tempText.length() <= _dspDigitsQty)
                tempDpData[_dspDigitsQty - tempText.length()] = _dot;
        }
    }
    text = tempText;
    if (text.length() <= _dspDigitsQty){
        for (unsigned int i {0}; i < text.length(); ++i){
            position = _charSet.indexOf(text.charAt(i));
            if (position > -1) {
                // Character found for translation
                temp7SegData[(_dspDigitsQty - 1) - i] = _charLeds[position];
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
        for (uint8_t i{0}; i < _dspDigitsQty; ++i)
            *(_dspBuffPtr + i) = temp7SegData[i] & tempDpData[i];
    }

    return displayable;
}

bool SevenSegDisplays::print(const int32_t &value, bool rgtAlgn, bool zeroPad){
    bool displayable{true};
    String readOut{""};
    // uint8_t _dspDigitsQty{_dspHwPtr->getDspDigits()};

    if ((value < _dspValMin) || (value > _dspValMax)) {
        clear();
        displayable = false;
    }
    else {
        readOut = String(abs(value));
        if (rgtAlgn) {
            if (zeroPad)
                readOut = _zeroPadding + readOut;
            else
                readOut = _spacePadding + readOut;

            if (value >= 0)
                readOut = readOut.substring(readOut.length() - _dspDigitsQty);
            else
                readOut = readOut.substring(readOut.length() - (_dspDigitsQty - 1));
        }
        if (value < 0)
            readOut = "-" + readOut;
        displayable = print(readOut);
    }

    return displayable;
}

bool SevenSegDisplays::print(const double &value, const unsigned int &decPlaces, bool rgtAlgn, bool zeroPad){
    bool displayable{true};
    String readOut{""};
    String pad{""};
    int start{0};
    // uint8_t _dspDigitsQty{_dspHwPtr->getDspDigits()};

    if (decPlaces == 0)
        displayable = print(int(value), rgtAlgn, zeroPad);
    else if ((value < _dspValMin) || (value > _dspValMax) || (decPlaces > _dspDigitsQty)) {
        displayable = false;
        clear();
    }
    else if ((decPlaces + String(int(value)).length()) > (((value < 0) && (value > (-1))) ? (_dspDigitsQty - 1) : _dspDigitsQty)) {
        displayable = false;
        clear();
    }
    else {
        if (value < 0 && value > -1)
            readOut = "-";
        readOut += String(int(value)) + ".";
        start = String(value).indexOf('.') + 1;
        readOut += (String(value) + _zeroPadding).substring(start, start + decPlaces);
        if (rgtAlgn) {
            if (readOut.length() < _dspDigitsQty + 1) {
                if (value < 0)
                    pad += "-";
                if (zeroPad)
                    pad += _zeroPadding;
                else
                    pad += _spacePadding;
                if (value < 0)
                    readOut = pad.substring(0, (_dspDigitsQty+1) - (readOut.length()-1)) + readOut.substring(1);
                else
                    readOut = pad.substring(0, (_dspDigitsQty+1) - (readOut.length())) + readOut;
                readOut = readOut.substring(0, _dspDigitsQty + 1);
            }
        }
        displayable = print(readOut);
    }

    return displayable;
}

/*void SevenSegDisplays::refresh(){
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

    updBlinkState();
    updWaitState();

    if((_blinking == false) || (_blinkShowOn == true)){
        for (int i {0}; i < _dspDigitsQty; i++){
            tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
            // send(tmpDigToSend, uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));
        }
    }
    else if(_blinking && !_blinkShowOn){
      for(int i{0}; i < _dspDigitsQty; i++)
         tmpLogic = tmpLogic && *(_blinkMaskPtr + ((i + _firstRefreshed) % _dspDigitsQty));
        if (!tmpLogic){   //At least one digit is set NOT TO BLINK
            for (int i {0}; i < _dspDigitsQty; i++){
                if(!*(_blinkMaskPtr + ((i + _firstRefreshed) % _dspDigitsQty))){
                    tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
                    // send(tmpDigToSend, 1 << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));
                }
            }
        }
    }
    ++_firstRefreshed;
    if (_firstRefreshed == _dspDigitsQty)
        _firstRefreshed = 0;

    return;
}*/

void SevenSegDisplays::resetBlinkMask(){
   for (uint8_t i{0}; i < _dspDigitsQty; i++)
      *(_blinkMaskPtr + i) = true;

   return;
}

void SevenSegDisplays::restoreDspBuff(){
    // for (int i{0}; i < _dspDigitsQty; i++){
    //     (*(_dspAuxBuffPtr + i)) = (*(_dspBuffPtr + i));
    // }
    strncpy((char*)_dspBuffPtr, (char*)_dspAuxBuffPtr, _dspDigitsQty );

    return;
}

void SevenSegDisplays::saveDspBuff(){
    // for (int i{0}; i < _dspDigitsQty; i++){
    //     (*(_dspAuxBuffPtr + i)) = (*(_dspBuffPtr + i));
    // }
    strncpy((char*)_dspAuxBuffPtr, (char*)_dspBuffPtr, _dspDigitsQty );

    return;
}

void SevenSegDisplays::setAttrbts(){
   if (_dspDigitsQty > 1){ // Calculate the minimum integer value displayable with this display's available digits
      _dspValMin = 1;
      for (uint8_t i{0}; i < (_dspDigitsQty - 1); i++)
         _dspValMin *= 10;
      _dspValMin = -(--_dspValMin); //_dspValMin = (-1)*(_dspValMin - 1);
   }
   else
      _dspValMin = 0;

   _dspValMax = 1; // Calculate the maximum integer value displayable with this display's available digits, create a Zero and a Space padding string for right alignement
   for (uint8_t i{0}; i < _dspDigitsQty; i++)
   {
      _dspValMax *= 10;
      _zeroPadding += "0";
      _spacePadding += " ";
      *(_blinkMaskPtr + i) = true;
   }
   --_dspValMax;

   _dspInstNbr = _displaysCount++;
   _dspInstance = this;

   if (!_dspUndrlHw.getCommAnnode())
   {
      _waitChar = ~_waitChar;
      _space = ~_space;
      _dot = ~_dot;
      for (int i{0}; i < (int)_charSet.length(); i++)
         _charLeds[i] = ~_charLeds[i];
   }

   return;
}


void SevenSegDisplays::setBlinkMask(const bool* blnkMsk){
   for (int i{0}; i < _dspDigitsQty; i++)
      *(_blinkMaskPtr + i) = *(blnkMsk + i);

    return;
}

bool SevenSegDisplays::setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate){
    bool result {false};
    long unsigned tmpOffRate{newOffRate};

    if (tmpOffRate == 0)
        tmpOffRate = newOnRate;
    if ((newOnRate != _blinkOnRate) || (tmpOffRate != _blinkOffRate)) { //The new ON rate is in the valid range
        if ((newOnRate >= _minBlinkRate) && (newOnRate <= _maxBlinkRate)) { //The new ON rate is in the valid range
            if ((tmpOffRate >= _minBlinkRate) && (tmpOffRate <= _maxBlinkRate)) {    //The new OFF rate is in the valid range or is equat to 0 to set a symmetric blink
                if(_blinkOnRate != newOnRate)
                    _blinkOnRate = newOnRate;
                if(_blinkOffRate != tmpOffRate)
                    _blinkOffRate = tmpOffRate;
                _blinkRatesGCD = blinkTmrGCD(newOnRate, newOffRate);

                if(_blinking){
                    // If it's active and running modify the timer taking care of the blinking
                }
                result =  true;
            }
        }
    }
    else{
        result =  true; //There's no need to change the current values, but as those were valid, they are still valid
    }

    return result;  
}

/*bool SevenSegDisplays::setDigitsOrder(uint8_t* newOrderPtr){
    bool result{true};

    for(int i {0}; i < _dspDigitsQty; i++){
        if (*(newOrderPtr + i) >= _dspDigitsQty){
            result = false;
            break;
        }   
    }
    if (result){
        for(int i {0}; i < _dspDigitsQty; i++){
            *(_digitPosPtr + i) = *(newOrderPtr + i);
        }
    }

    return result;
}*/

bool SevenSegDisplays::setWaitChar (const char &newChar){
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

bool SevenSegDisplays::setWaitRate(const unsigned long &newWaitRate){
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

/*bool SevenSegDisplays::stop() {
    //This object's pointer will be deleted from the arrays of pointers. If the array has no more valid pointers the timer will be stopped to avoid loosing processing time.
    bool pointersFound(false);
    bool result {false};

    for(uint8_t i {0}; i < _dspPtrArrLngth; i++){
        if (*(_instancesLstPtr + i) == _dspInstance){
            *(_instancesLstPtr + i) = nullptr;
            result = true;
        }
        else if (*(_instancesLstPtr + i) != nullptr){
            // There are still objects pointers in the vector, so the refresh display services must continue active
            pointersFound = true;
            if(result)
                break;
        }
    }

    if (!pointersFound){
        //There are no more display instances active, there's no point in keeping the ISR active, the timer is stopped and the interrupt service detached
        delete [] _instancesLstPtr;
        _instancesLstPtr = nullptr;
        
        if(_dspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
            xTimerStop(_dspRfrshTmrHndl, portMAX_DELAY);
            xTimerDelete(_dspRfrshTmrHndl, portMAX_DELAY);
            _dspRfrshTmrHndl = nullptr;
        }
    }   

    return result;
}*/

void SevenSegDisplays::tmrCbWait(TimerHandle_t waitTmrCbArg){
/*   SevenSegDisplays **argObj = (SevenSegDisplays**)pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling each display's fastRefresh() method
    
    for(uint8_t i {0}; i < _dspPtrArrLngth; i++){
        // if (*(_instancesLstPtr + i) != nullptr)
            // (*(_instancesLstPtr + i)) -> fastRefresh();
    }    
*/
    return;
}

void SevenSegDisplays::tmrCbBlink(TimerHandle_t blinkTmrCbArg){
/*   SevenSegDisplays **argObj = (SevenSegDisplays**)pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling each display's fastRefresh() method
    
    for(uint8_t i {0}; i < _dspPtrArrLngth; i++){
        // if (*(_instancesLstPtr + i) != nullptr)
            // (*(_instancesLstPtr + i)) -> fastRefresh();
    }    
*/
    return;
}

void SevenSegDisplays::updBlinkState(){
   //The use of a xTimer that keeps flip-floping the _blinkShowOn value is better suited for symmetrical blinking, but not for assymetrical cases.
   if (_blinking == true){
      if (_blinkShowOn == false) {
         if (_blinkTimer == 0){
            //turn off the digits by sending directly a blank to each port, without affecting the _digit[] buffer
            for (int i{0}; i < _dspDigitsQty; i++)
            //    send(_space, 0b01 << i);
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

void SevenSegDisplays::updWaitState(){
   if (_waiting == true){
      if (_waitTimer == 0){
         for (int i{0}; i < _dspDigitsQty; i++)
            *(_dspBuffPtr + i) = _space;
         _waitTimer = xTaskGetTickCount()/portTICK_RATE_MS;
      }
      else if((xTaskGetTickCount()/portTICK_RATE_MS - _waitTimer) > _waitRate){
         for (int i{_dspDigitsQty - 1}; i >= 0; i--){

            if(( _dspDigitsQty - i) <= _waitCount)
               *(_dspBuffPtr + i) = _waitChar;
            else
               *(_dspBuffPtr + i) = _space;
         }
         _waitCount++;
         if (_waitCount == (_dspDigitsQty + 1))
            _waitCount = 0;
         _waitTimer = xTaskGetTickCount()/portTICK_RATE_MS;
      }
   }

   return;
}

bool SevenSegDisplays::wait(const unsigned long &newWaitRate){
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

bool SevenSegDisplays::write(const uint8_t &segments, const uint8_t &port){
    bool result {false};
    
    if (port < _dspDigitsQty){
        *(_dspBuffPtr + port) = segments;
        result = true;
    }
    
    return result;
}

bool SevenSegDisplays::write(const String &character, const uint8_t &port){
    bool result {false};
    int position {-1};
    
    if (port < _dspDigitsQty){
            position = _charSet.indexOf(character);
            if (position > -1) {
                // Character found for translation
                *(_dspBuffPtr + port) = _charLeds[position];
                result = true;
            }
    }

    return result;
}

//============================================================> Class methods separator

ClickCounter::ClickCounter(uint8_t ccSclk, uint8_t ccRclk, uint8_t ccDio, bool rgthAlgn, bool zeroPad, bool commAnode, const uint8_t dspDigits)
:_display()
// :_display(ccSclk, ccRclk, ccDio, commAnode, dspDigits), _countRgthAlgn {rgthAlgn}, _countZeroPad {zeroPad}
{
    //Class constructor
}

ClickCounter::~ClickCounter(){

}

bool ClickCounter::blink(){
    
    return _display.blink();
}

bool ClickCounter::blink(const unsigned long &onRate, const unsigned long &offRate){

    return _display.blink(onRate, offRate);
}

void ClickCounter::clear(){
    _display.clear();

    return;
}

bool ClickCounter::countBegin(int32_t startVal){
   bool result{false};

    if ((startVal >= _display.getDspValMin()) && (startVal <= _display.getDspValMax())){
        // if (_display.begin() == true){
            result = countRestart(startVal);
            if (result)
                _beginStartVal = startVal;
        // }
    }
   return result;
}

bool ClickCounter::countDown(int32_t qty){
    bool result {false};
    qty = abs(qty);

    if((_count - qty) >= _display.getDspValMin()){
        _count -= qty;
        result = updDisplay();
    }

    return result;
}

bool ClickCounter::countReset(){

    return countRestart(_beginStartVal);
}
    
bool ClickCounter::countRestart(int32_t restartValue){
   bool result{false};

   if ((restartValue >= _display.getDspValMin()) && (restartValue <= _display.getDspValMax())){
      _count = restartValue;
      result = updDisplay();
   }

   return result;
}

bool ClickCounter::countStop(){
    
    // return _display.stop();
    return true;
}

bool ClickCounter::countToZero(int32_t qty){
    bool result {false};

    if (_count > 0)
        result = countDown(qty);
    else if (_count < 0)
        result = countUp(qty);
    
    return result;
}

bool ClickCounter::countUp(int32_t qty){
    bool result {false};
    qty = abs(qty);

    if((_count + qty) <= _display.getDspValMax()){
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

    return _display.noBlink();
}

bool ClickCounter::setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate){

    return _display.setBlinkRate(newOnRate, newOffRate);
}

bool ClickCounter::updDisplay(){

    return _display.print(_count, _countRgthAlgn, _countZeroPad);
}

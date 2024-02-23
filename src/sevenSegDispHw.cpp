#include "Arduino.h"
#include "sevenSegDispHw.h"

const uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4};
const uint8_t noName4Bits[4] {0, 1, 2, 3};

TimerHandle_t SevenSegDynamic::_dspRfrshTmrHndl = nullptr;

SevenSegDispHw::SevenSegDispHw() {}

SevenSegDispHw::SevenSegDispHw(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
: _dspDigitsQty {dspDigits}, _commAnode {commAnode}
{
}

SevenSegDispHw::~SevenSegDispHw(){}

bool SevenSegDispHw::getCommAnnode(){

    return _commAnode;
}

uint8_t SevenSegDispHw::getDspDigits(){

    return _dspDigitsQty;
}

bool SevenSegDispHw::setDigitsOrder(uint8_t* newOrderPtr){
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

bool SevenSegDynamic::begin(){
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
}


void SevenSegDynamic::refresh(){
   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

    // updBlinkState();
    // updWaitState();

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
}

bool SevenSegDynamic::stop() {
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
}

void SevenSegDynamic::tmrCbRefresh(TimerHandle_t waitTmrCbArg){
/*   SevenSegDisplays **argObj = (SevenSegDisplays**)pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling each display's fastRefresh() method
    
    for(uint8_t i {0}; i < _dspPtrArrLngth; i++){
        // if (*(_instancesLstPtr + i) != nullptr)
            // (*(_instancesLstPtr + i)) -> fastRefresh();
    }    
*/
    return;
}


//============================================================> Class methods separator

// SevenSegStatic::SevenSegStatic() {}

SevenSegStatic::~SevenSegStatic() {}

//============================================================> Class methods separator

SevenSegHC595Dyn::SevenSegHC595Dyn(uint8_t* ioPins, uint8_t dspDigits, bool commAnode)
:_ioPins{ioPins}
{    
    SevenSegDispHw(ioPins, dspDigits, commAnode);
    pinMode(*(_ioPins + _sclk), OUTPUT);
    pinMode(*(_ioPins + _rclk), OUTPUT);
    pinMode(*(_ioPins + _dio), OUTPUT);
}

SevenSegHC595Dyn::~SevenSegHC595Dyn(){}



void SevenSegHC595Dyn::fastSend(uint8_t content){
    for (int i {7}; i >= 0; i--){   //Send each of the 8 bits representing the character
        if (content & 0x80)
            digitalWrite(*(_ioPins + _dio), HIGH);
        else
            digitalWrite(*(_ioPins + _dio), LOW);
        content <<= 1;
        digitalWrite(*(_ioPins + _sclk), LOW);
        digitalWrite(*(_ioPins + _sclk), HIGH);
    }

    return;

}

void SevenSegHC595Dyn::fastSend(const uint8_t &segments, const uint8_t &port){

    digitalWrite(*(_ioPins + _rclk), LOW);
    fastSend(segments);
    fastSend(port);
    digitalWrite(*(_ioPins + _rclk), HIGH);

   return;
}

//============================================================> Class methods separator

// SevenSegTM1637::SevenSegTM1637() {}

SevenSegTM1637::~SevenSegTM1637() {}

//============================================================> Class methods separator

// SevenSegHC595Stat::SevenSegHC595Stat() {}

SevenSegHC595Stat::~SevenSegHC595Stat() {}

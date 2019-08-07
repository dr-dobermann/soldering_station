#include <TFT.h>  // Arduino LCD library
#include <SPI.h>

#include "encoder.h"


const uint8_t 
    // encoder pins
    PBTN  = 12, 
    PENCA = A2, 
    PENCB = A3,
    // mode button
    PMODE_BTN = 2, 
    // SPI TFT 160x128 display pins
    PDC   = 7,
    PCS   = 10,
    PRST  = 8,
    // soldering station active pins
    P_IRON       = 5,
    P_FAN        = 6,
    P_HEATER     = 3,
    // soldering station control pins
    P_HTR_TEMP   = A1,  // heater fan temp
    P_FSTD_SENS  = 4,   // heater gercon switch
    P_TEMP_CHECK = A0,  // iron temp
    P_T12_CHK = A6,     // iron stick checker
    P_T12_SW  = 9;      // iron vibration sensor
//---------------------------------------------------------------------

void setup() {
    Serial.begin(9600);
}
//---------------------------------------------------------------------

void loop() {
    //enc.tick();
}
//---------------------------------------------------------------------

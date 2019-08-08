#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

#include <Fonts/FreeMonoBold12pt7b.h>

// 'fan', 16x16px
const uint8_t fan_icon[] = {0xf0, 0x7f, 0xc0, 0x3f, 0x80, 0x1f, 0x80, 0x3f, 0x83, 0xe1, 0x84, 0x00, 0xc8, 0x20, 0xb8, 0x20,
                            0x08, 0x20, 0x00, 0x00, 0x03, 0xc1, 0x00, 0xc1, 0x80, 0x43, 0x80, 0xe7, 0xc0, 0xff, 0xf1, 0xff};

#include "encoder.h"
#include "button.h"

const uint8_t 
    // encoder pins
    PBTN  = 12, 
    PENCA = A2, 
    PENCB = A3,

    // mode button
    PMODE_BTN = 2, 
    
    // SPI TFT 160x128 display pins
    // it also uses MOSI(11), SCK(13) Arduino pins
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

Adafruit_ST7735 tft = Adafruit_ST7735(PCS, PDC, PRST);

//---------------------------------------------------------------------
void setup() {
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    
    Serial.begin(9600);
}
//---------------------------------------------------------------------

void loop() {
    //enc.tick();
}
//---------------------------------------------------------------------

void show_stat() {
    
}
//---------------------------------------------------------------------

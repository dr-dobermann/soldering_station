#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

#include "fan.h"
#include "iron.h"
#include "encoder.h"
#include "button.h"

namespace ss = sstation;

// 'fan', 16x16px
const uint8_t fan_icon[] = {0xf0, 0x7f, 0xc0, 0x3f, 0x80, 0x1f, 0x80, 0x3f, 0x83, 0xe1, 0x84, 0x00, 0xc8, 0x20, 0xb8, 0x20,
                            0x08, 0x20, 0x00, 0x00, 0x03, 0xc1, 0x00, 0xc1, 0x80, 0x43, 0x80, 0xe7, 0xc0, 0xff, 0xf1, 0xff};

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
    P_T12_CHK = A6,     // iron stick check
    P_T12_SW  = 9;      // iron vibration sensor

const int64_t SHOW_DELAY = 40;  // max 25 fps

//---------------------------------------------------------------------

typedef enum {
    smOff,
    smIron,
    smFan,
    smConfig
} StationMode;

typedef struct {
    uint16_t temp;
    uint16_t sel_temp;
    ss::ToolState state;
    ss::ToolSubState sstate;
    ss::ToolPowerLevel power;
    long time_left;
} IronState;
//---------------------------------------------------------------------

Adafruit_ST7735 tft = Adafruit_ST7735(PCS, PDC, PRST);
ss::Fan fan = ss::Fan(P_HEATER, P_FAN, P_HTR_TEMP, P_FSTD_SENS);
ss::Iron iron = ss::Iron(P_IRON, P_TEMP_CHECK, P_T12_SW, P_T12_CHK);

encdr::Encoder enc = encdr::Encoder(PENCA, PENCB);
dbtn::Button enc_btn = dbtn::Button(PBTN);
dbtn::Button mode_btn = dbtn::Button(PMODE_BTN);

StationMode mode = smIron;
//---------------------------------------------------------------------

void cfg(int enc_value, dbtn::BtnStatus enc_btn) {}
//---------------------------------------------------------------------

void setup() {
    
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    
    Serial.begin(9600);
}
//---------------------------------------------------------------------

void loop() {

    // update controls
    enc.tick();
    enc_btn.tick();
    mode_btn.tick();

    // check mode button
    // and update mode if needed

    switch (mode) {
        case smOff:
            iron.off(ss::tsOff);
            fan.off(ss::tsOff);
            break;
            
        case smIron:
            iron.tick(enc.get_value(), enc_btn.get_status());
            fan.tick();
            break;
            
        case smFan:
            fan.tick(enc.get_value(), enc_btn.get_status());
            iron.tick();
            break;
            
        smConfig:
            fan.tick();
            iron.tick();
            cfg(enc.get_value(), enc_btn.get_status());
            break;
    }

    show_stat();
}
//---------------------------------------------------------------------

void show_stat() {

    static StationMode prevMode;
    static int64_t last_show_time;
    static IronState istate;

    if ( millis() - last_show_time < SHOW_DELAY )
        return;
        
    if ( prevMode != mode ) {
        prevMode = mode;
        tft.fillScreen(ST77XX_BLACK);
    }
    
    switch ( mode ) {
        case smOff:
            tft.setFont(&FreeMonoBold12pt7b);
            tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
            tft.setCursor(10, 60);
            tft.print("Station is");            
            tft.setCursor(60, 80);
            tft.print("OFF");
            break;
            
        case smIron:
        case smFan:
            // show active tool
            if ( mode == smIron )
                tft.drawRect(0, 0, 160, 64, ST77XX_CYAN);
            else if ( mode == smFan ) 
                tft.drawRect(0, 64, 160, 64, ST77XX_CYAN);

            //////////////////////////////////////////////////////
            // show iron state
            if ( iron.state == ss::tsOff ) {
                if (istate.state != ss::tsOff ) 
                    tft.fillRect(1, 1, 158, 62, ST77XX_BLACK);
                tft.setFont(&FreeMonoBold18pt7b);
                tft.setTextColor(ST77XX_CYAN);
                tft.setCursor(50, 45);
                tft.print("OFF");
                
                istate.state = iron.state;
                break;
            }
            if ( istate.temp != iron.curr_temp ) { // current temp
                tft.setFont(&FreeMonoBold18pt7b);
                tft.setTextColor(ST77XX_BLACK);
                tft.setCursor(50, 45);
                tft.print(istate.temp);
                tft.setTextColor(ST77XX_CYAN);
                tft.setCursor(50, 45);
                tft.print(iron.curr_temp);
                
                istate.temp = iron.curr_temp;
            }
            if ( istate.sel_temp != iron.sel_temp ) { // selected temp
                tft.setFont();
                tft.setTextColor(ST77XX_BLACK);
                tft.setCursor(10, 10);
                tft.print("set: ");
                tft.print(istate.sel_temp);
                tft.setTextColor(ST77XX_CYAN);
                tft.setFont();
                tft.setCursor(10, 10);
                tft.print("set: ");
                tft.print(iron.sel_temp);

                istate.sel_temp = iron.sel_temp;
            }
            if ( istate.state != iron.state ) { // iron state
                if ( iron.state == ss::tsIdle ||
                     iron.state == ss::tsStandBy ) {
                    tft.setFont();
                    tft.setTextColor(ST77XX_BLACK);
                    tft.setCursor(10, 30);
                    tft.print(iron.state == ss::tsIdle ? "IDL" : "SBY");
                    tft.setTextColor(ST77XX_CYAN);
                    tft.setFont();
                    tft.setCursor(10, 30);
                    tft.print(iron.state == ss::tsIdle ? "IDL" : "SBY");
                }
                else
                    if ( istate.state == ss::tsIdle ||
                         istate.state == ss::tsStandBy ) {
                        tft.setFont();
                        tft.setTextColor(ST77XX_BLACK);
                        tft.setCursor(10, 30);
                        tft.print(iron.state == ss::tsIdle ? "IDL" : "SBY");
                    }
                istate.state = iron.state;                
            }
//            if ( istate.time_left != istate.time_left ) { // timer
                tft.setFont();
                tft.setTextColor(ST77XX_BLACK);
                tft.setCursor(10, 50);
                tft.print("-");
                uint16_t mins = istate.time_left / 60;
                tft.print(mins);
                tft.print(":");
                uint16_t secs = istate.time_left/1000 % 60;
                if ( secs < 10 )
                    tft.print("0");
                tft.print(secs);
                
                tft.setTextColor(ST77XX_CYAN);
                tft.setCursor(10, 50);
                tft.print("- ");
                mins = iron.time_left / 60;
                tft.print(mins);
                tft.print(":");
                secs = iron.time_left % 60;
                if ( secs < 10 )
                    tft.print("0");
                tft.print(secs);

                istate.time_left = iron.time_left;
//            }
            
            //////////////////////////////////////////////////////
            // show fan state


            break;
            
        case smConfig:
            break;
    }
    
    last_show_time = millis();
}
//---------------------------------------------------------------------

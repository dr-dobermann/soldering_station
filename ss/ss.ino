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
const unsigned char fan_icon[] PROGMEM = {
    0x0f, 0x80, 0x3f, 0xc0, 0x3f, 0xe0, 0x7f, 0xc0, 0x7c, 0x1e, 0x7b, 0xbf, 0x37, 0xdf, 0x47, 0xdf, 
    0xf7, 0xdf, 0xff, 0xff, 0xfc, 0x3e, 0xff, 0x3e, 0x7f, 0xbc, 0x7f, 0x18, 0x3f, 0x00, 0x0e, 0x00,
};

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

typedef struct {
    ss::ToolState state;
    ss::ToolSubState sstate;
    uint16_t temp;
    uint16_t sel_temp;
    ss::ToolPowerLevel speed;
    ss::ToolPowerLevel power;
    uint16_t time_left;
} FanState;
//---------------------------------------------------------------------

Adafruit_ST7735 tft = Adafruit_ST7735(PCS, PDC, PRST);
ss::Fan fan = ss::Fan(P_HEATER, P_FAN, P_HTR_TEMP, P_FSTD_SENS);
ss::Iron iron = ss::Iron(P_IRON, P_TEMP_CHECK, P_T12_SW, P_T12_CHK);

encdr::Encoder enc = encdr::Encoder(PENCA, PENCB);
dbtn::Button enc_btn = dbtn::Button(PBTN);
dbtn::Button mode_btn = dbtn::Button(PMODE_BTN);

StationMode mode = smFan; // TODO: change to smOff;

//int16_t pwr_ind_colors[] = {ST77XX_BLUE, ST77XX_GREEN, ST77XX_YELLOW, ST77XX_RED};

char *SPD_NAMES[] = {"OFF", "LOW", "MED", "HIGH", "FULL"};
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

            show_iron_info();
            
            show_fan_info();


            break;
            
        case smConfig:
            break;
    }
    
    last_show_time = millis();
}
//---------------------------------------------------------------------

void show_iron_info() {

    static IronState istate;

    if ( iron.state == ss::tsOff ) {
        if (istate.state != ss::tsOff ) 
            tft.fillRect(1, 1, 158, 62, ST77XX_BLACK);
        tft.setFont(&FreeMonoBold18pt7b);
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(50, 45);
        tft.print("OFF");
        
        istate.state = iron.state;
        return;
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
        tft.print("SET: ");
        tft.print(istate.sel_temp);
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(10, 10);
        tft.print("SET: ");
        tft.setTextColor(ST77XX_YELLOW);
        tft.print(iron.sel_temp);

        istate.sel_temp = iron.sel_temp;
    }
    if ( istate.state != iron.state ) { // iron state
        if ( iron.state == ss::tsIdle ||
             iron.state == ss::tsStandBy ) {
            tft.setFont();
            tft.setTextColor(ST77XX_BLACK);
            tft.setCursor(10, 25);
            tft.print(iron.state == ss::tsIdle ? "IDLE" : "STNBY");
            tft.setTextColor(ST77XX_CYAN);
            tft.setCursor(10, 25);
            tft.print(iron.state == ss::tsIdle ? "IDLE" : "STNBY");
        }
        else
            if ( istate.state == ss::tsIdle ||
                 istate.state == ss::tsStandBy ) {
                tft.setFont();
                tft.setTextColor(ST77XX_BLACK);
                tft.setCursor(10, 25);
                tft.print(iron.state == ss::tsIdle ? "IDLE" : "SNDBY");
            }
        istate.state = iron.state;                
    }
    if ( istate.time_left != iron.time_left ) { // timer
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(10, 35);
        tft.print("-");
        tft.print(istate.time_left / 60);
        tft.print(":");
        uint16_t secs = istate.time_left/1000 % 60;
        if ( secs < 10 )
            tft.print("0");
        tft.print(secs);
        
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(10, 35);
        tft.print("-");
        tft.print(iron.time_left / 60);
        tft.print(":");
        secs = iron.time_left % 60;
        if ( secs < 10 )
            tft.print("0");
        tft.print(secs);

        istate.time_left = iron.time_left;
    }

    if ( istate.power != iron.power ) { // iron power
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(125, 40);
        tft.print(istate.power * 25);
        tft.print("%");
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(125, 40);
        tft.print(iron.power * 25);
        tft.print("%");

        for ( int l = 1; l <= istate.power; l++ ) {
            tft.drawRect(125, 30 - (l - 1)*8, 25, 3, ST77XX_BLACK);
            tft.drawRect(125, 30 - (l - 1)*8 + 4, 25, 3, ST77XX_BLACK);
        }

        for ( int l = 1; l <= iron.power; l++ ) {
            tft.drawRect(125, 30 - (l - 1)*8, 25, 3, ST77XX_CYAN);
            tft.drawRect(125, 30 - (l - 1)*8 + 4, 25, 3, ST77XX_CYAN);
//            tft.drawRect(125, 30 - (l - 1)*8, 25, 3, pwr_ind_colors[l - 1]);
//            tft.drawRect(125, 30 - (l - 1)*8 + 4, 25, 3, pwr_ind_colors[l - 1]);
        }
                    
        istate.power = iron.power;
    }

    if ( istate.sstate != iron.sstate ) {  // substate
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(7, 52);
        tft.print("[OFF] [STND-BY] [CNCL]");
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(7, 52);
        switch ( iron.sstate ) {
            case ss::tssWaitToStandBy:
                tft.print(" OFF  [STND-BY]  CANCEL ");
                break;
            case ss::tssWaitToOff:
                tft.print("[OFF]  STND-BY   CANCEL ");
                break;
            case ss::tssWaitToCancel:
                tft.print(" OFF   STND-BY  [CANCEL]");
                break;
        }
        
        istate.sstate = iron.sstate;
    }
}
//---------------------------------------------------------------------

void show_fan_info() {
    
    static FanState fstate;

    if ( fan.state == ss::tsOff ) {
        if (fstate.state != ss::tsOff ) 
            tft.fillRect(1, 65, 158, 62, ST77XX_BLACK);
        tft.setFont(&FreeMonoBold18pt7b);
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(50, 109);
        tft.print("OFF");
        
        fstate.state = fan.state;
        return;
    }
    if ( fstate.temp != fan.curr_temp ) { // current temp
        tft.setFont(&FreeMonoBold18pt7b);
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(50, 109);
        tft.print(fstate.temp);
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(50, 109);
        tft.print(fan.curr_temp);
        
        
        fstate.temp = fan.curr_temp;
    }
    if ( fstate.sel_temp != fan.sel_temp ) { // selected temp
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(10, 74);
        tft.print("SET: ");
        tft.print(fstate.sel_temp);
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(10, 74);
        tft.print("SET: ");
        tft.setTextColor(ST77XX_YELLOW);
        tft.print(fan.sel_temp);

        fstate.sel_temp = fan.sel_temp;
    }
    if ( fstate.speed != fan.speed ) { // speed
        tft.drawBitmap(70, 70, fan_icon, 16, 16, ST77XX_CYAN);
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(90, 74);
        tft.print(SPD_NAMES[fstate.speed]);
        tft.setCursor(90, 74);
        tft.setTextColor(ST77XX_YELLOW);
        tft.print(SPD_NAMES[fan.speed]);

        fstate.speed = fan.speed;
    }
    if ( fstate.state != fan.state ) { // fan state
        if ( fan.state == ss::tsIdle ||
             fan.state == ss::tsStandBy ) {
            tft.setFont();
            tft.setTextColor(ST77XX_BLACK);
            tft.setCursor(10, 89);
            tft.print(fan.state == ss::tsIdle ? "IDLE" : "STNBY");
            tft.setTextColor(ST77XX_CYAN);
            tft.setCursor(10, 89);
            tft.print(fan.state == ss::tsIdle ? "IDLE" : "STNBY");
        }
        else
            if ( fstate.state == ss::tsIdle ||
                 fstate.state == ss::tsStandBy ) {
                tft.setFont();
                tft.setTextColor(ST77XX_BLACK);
                tft.setCursor(10, 89);
                tft.print(fan.state == ss::tsIdle ? "IDLE" : "SNDBY");
            }
        fstate.state = fan.state;                
    }
    if ( fstate.time_left != fan.time_left ) { // timer
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(10, 99);
        tft.print("-");
        tft.print(fstate.time_left / 60);
        tft.print(":");
        uint16_t secs = fstate.time_left/1000 % 60;
        if ( secs < 10 )
            tft.print("0");
        tft.print(secs);
        
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(10, 99);
        tft.print("-");
        tft.print(fan.time_left / 60);
        tft.print(":");
        secs = fan.time_left % 60;
        if ( secs < 10 )
            tft.print("0");
        tft.print(secs);

        fstate.time_left = fan.time_left;
    }

    if ( fstate.power != fan.power ) { // fan power
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(125, 104);
        tft.print(fstate.power * 25);
        tft.print("%");
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(125, 104);
        tft.print(fan.power * 25);
        tft.print("%");

        for ( int l = 1; l <= fstate.power; l++ ) {
            tft.drawRect(125, 94 - (l - 1)*8, 25, 3, ST77XX_BLACK);
            tft.drawRect(125, 94 - (l - 1)*8 + 4, 25, 3, ST77XX_BLACK);
        }

        for ( int l = 1; l <= fan.power; l++ ) {
            tft.drawRect(125, 94 - (l - 1)*8, 25, 3, ST77XX_CYAN);
            tft.drawRect(125, 94 - (l - 1)*8 + 4, 25, 3, ST77XX_CYAN);
//            tft.drawRect(125, 94 - (l - 1)*8, 25, 3, pwr_ind_colors[l - 1]);
//            tft.drawRect(125, 94 - (l - 1)*8 + 4, 25, 3, pwr_ind_colors[l - 1]);
        }
                    
        fstate.power = fan.power;
    }

    if ( fstate.sstate != fan.sstate ) {  // substate
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(7, 116);
        tft.print("[OFF] [STND-BY] [CNCL]");
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(7, 116);
        switch ( fan.sstate ) {
            case ss::tssWaitToStandBy:
                tft.print(" OFF  [STND-BY]  CANCEL ");
                break;
            case ss::tssWaitToOff:
                tft.print("[OFF]  STND-BY   CANCEL ");
                break;
            case ss::tssWaitToCancel:
                tft.print(" OFF   STND-BY  [CANCEL]");
                break;
        }
        
        fstate.sstate = fan.sstate;
    }    
}

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

typedef enum {
    ssmNormal = 0,
    ssmWaitForCfg,      // Station configuration selected, wait for approval
    ssmWaitForOff,      // Station OFF selected, wait for approval
    ssmWaitForCancel,   // Cancel of Station Off is waiting for approval
    ssmTurningOff       // Shutdown initiated and station waits for tools to turn off
} StationSubMode;

typedef struct {
    uint16_t temp;
    uint16_t sel_temp;
    ss::ToolState state;
    ss::ToolMenuState mstate;
    ss::ToolPowerLevel power;
    long time_left;
} IronState;

typedef struct {
    ss::ToolState state;
    ss::ToolMenuState mstate;
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

StationMode mode = smOff;
StationSubMode smode = ssmNormal;

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

    static uint64_t appr_time; // approval deadline time
                               // if there is no approval in a given time
                               // station returns into ssmNormal submode

    if ( mode_btn.get_btn_pressed() > 0 ) {
        if ( mode != smOff ) {
            switch ( smode ) {
                case ssmNormal:
                    mode = (mode == smFan ? smIron : smFan);
                    break;

                case ssmWaitForCfg:
                    mode = smConfig;
                    smode = ssmNormal;
                    break;
                    
                case ssmWaitForOff:
                    mode = smOff; 
                    smode = ssmTurningOff; 
                    break;
                    
                case ssmWaitForCancel:
                    smode = ssmNormal;
                    break;
            }            
        } 
        else 
            if ( mode == smOff ) {
                mode = smIron;
                smode = ssmNormal;
            }
    }

    if ( mode != smOff && mode != smConfig && mode_btn.get_lbtn_pressed() > 0 )
        smode = ssmWaitForCfg;
        
    // if an user select station config or shut down, check encoder to approve or cancel it
    if ( mode != smOff and (smode == ssmWaitForCfg ||
                            smode == ssmWaitForOff || 
                            smode == ssmWaitForCancel) ) {
        int eval = enc.get_value();
        if ( eval > 0 ) {
            smode = smode + 1;
            if ( smode > ssmWaitForCancel )
                smode = ssmWaitForOff;
        }
        else 
            if ( eval < 0 ) {
                smode = smode - 1;
                if ( smode < ssmWaitForCfg )
                    smode = ssmWaitForCancel;
            }
        
    
        if ( enc_btn.get_btn_pressed() > 0 )
            switch ( smode ) {
                case ssmWaitForCfg:
                    mode = smConfig;
                    smode = ssmNormal;
                    break;
                    
                case ssmWaitForOff:
                    mode = smOff;
                    smode = ssmTurningOff; 
                    break;

                case ssmWaitForCancel:
                    smode = ssmNormal;
                    break;
            }
    }

    switch ( mode ) {
        case smOff:
            iron.off(ss::tsOff);
            fan.off(ss::tsOff);
            if ( smode == ssmTurningOff &&     // if station is shutting down,
                 iron.state == ss::tsOff &&    // check actual tools' states
                 fan.state == ss::tsOff )      // and if they're off, then off the station
                smode = ssmNormal;
            break;
            
        case smIron:
            iron.tick(enc.get_value(), enc_btn.get_status());
            fan.tick();
            break;
            
        case smFan:
            fan.tick(enc.get_value(), enc_btn.get_status());
            iron.tick();
            break;
            
        case smConfig:
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
    static StationSubMode prevSMode;
    static int64_t last_show_time;

    bool updateTools = false;

    if ( millis() - last_show_time < SHOW_DELAY )
        return;
        
    if ( prevMode != mode ) {
        prevMode = mode;
        updateTools = true;
        tft.fillScreen(ST77XX_BLACK);
    }
    
    switch ( mode ) {
        case smOff:
            tft.setFont(&FreeMonoBold12pt7b);
            tft.setTextColor(ST77XX_CYAN);

            if ( prevSMode != smode && prevSMode == ssmTurningOff ) {   // clear last line to prevent screen flicking
                tft.fillScreen(ST77XX_BLACK);
                prevSMode = smode;
            }

            if ( prevSMode != smode && smode == ssmTurningOff ) {
                tft.setCursor(10, 60);
                tft.print("Station is");
                tft.setCursor(20, 80);
                tft.print("shutting");
                tft.setCursor(30, 100);
                tft.print("down...");
                
                prevSMode = smode;
            }

            if ( smode == ssmNormal ) {
                tft.setCursor(10, 60);
                tft.print("Station is");
                tft.setCursor(60, 80);
                tft.print("OFF");
            }
            break;
            
        case smIron:
        case smFan:
            // show active tool
//            if ( smode == ssmNormal && prevSMode != smode ) {
//                prevSMode = smode;
//                tft.fillScreen(ST77XX_BLACK);
//            }
            if ( smode == ssmNormal )
                if ( mode == smIron ) {
                    tft.drawRect(0, 64, 160, 64, ST77XX_BLACK);
                    tft.drawRect(0, 0, 160, 64, ST77XX_CYAN);
                }
                else { 
                    tft.drawRect(0, 0, 160, 64, ST77XX_BLACK);
                    tft.drawRect(0, 64, 160, 64, ST77XX_CYAN);
                }

            show_iron_info(updateTools);
            
            show_fan_info(updateTools);

            if ( prevSMode != smode && (smode == ssmWaitForOff || 
                                        smode == ssmWaitForCancel ||
                                        smode == ssmWaitForCfg) ) {
                tft.fillRect(20, 50, 140, 30, ST77XX_BLACK);
                tft.drawRect(22, 52, 136, 26, ST77XX_CYAN);
                tft.setFont();
                tft.setTextColor(ST77XX_BLACK);
                tft.setCursor(24, 64);
                tft.print("[CFG] [OFF] [CANCEL]");
                tft.setTextColor(ST77XX_YELLOW);
                tft.setCursor(24, 64);
                if ( smode == ssmWaitForOff )
                    tft.print(" CFG  [OFF]  CANCEL ");
                else if ( smode == ssmWaitForCancel )
                    tft.print(" CFG   OFF  [CANCEL]");
                else
                    tft.print("[CFG]  OFF   CANCEL ");
                    
                prevSMode = smode;
            }
            break;
            
        case smConfig:
            break;
    }
    
    last_show_time = millis();
}
//---------------------------------------------------------------------

void show_iron_info(bool updateTool) {

    static IronState istate;

    if ( updateTool )
        memset(&istate, 0, sizeof(IronState));

    if ( iron.state != istate.state && (iron.state == ss::tsOff || istate.state == ss::tsOff ) ) {
        tft.fillRect(1, 1, 158, 62, ST77XX_BLACK);
        if ( iron.state == ss::tsOff )
            memset(&istate, 0, sizeof(IronState));
    }

    if ( iron.state == ss::tsOff ) {
        tft.setFont(&FreeMonoBold18pt7b);
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(50, 45);
        tft.print("OFF");
        
        return;
    }
    
    if ( istate.temp != iron.curr_temp ) { // current temp
        tft.setFont(&FreeMonoBold18pt7b);
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(55, 45);
        tft.print(istate.temp);
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(55, 45);
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
        if ( iron.state == ss::tsStandBy || iron.state == ss::tsRun ) {
            tft.setFont();
            tft.setTextColor(ST77XX_BLACK);
            tft.setCursor(10, 25);
            tft.print(istate.state == ss::tsStandBy ? "SNDBY" : "NORM");
            tft.setTextColor(ST77XX_CYAN);
            tft.setCursor(10, 25);
            tft.print(iron.state == ss::tsStandBy ? "SNDBY" : "NORM");
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
        uint16_t secs = istate.time_left % 60;
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

    if ( istate.mstate != iron.mstate ) {  // substate
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(7, 52);
        tft.print("[OFF] [STND-BY] [CANCEL]");
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(7, 52);
        switch ( iron.mstate ) {
            case ss::tmsWaitForStandBy:
                tft.print(" OFF  [STND-BY]  CANCEL ");
                break;
            case ss::tmsWaitForOff:
                tft.print("[OFF]  STND-BY   CANCEL ");
                break;
            case ss::tmsWaitForCancel:
                tft.print(" OFF   STND-BY  [CANCEL]");
                break;
        }
        
        istate.mstate = iron.mstate;
    }
}
//---------------------------------------------------------------------

void show_fan_info(bool updateTool) {
    
    static FanState fstate;

    if ( updateTool )
        memset(&fstate, 0, sizeof(IronState));

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
        tft.drawBitmap(70, 68, fan_icon, 16, 16, ST77XX_CYAN);
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
        if ( fan.state == ss::tsStandBy ) {
            tft.setFont();
            tft.setTextColor(ST77XX_BLACK);
            tft.setCursor(10, 89);
            tft.print(fan.state == ss::tsStandBy ? "STNBY" : "NORM");
            tft.setTextColor(ST77XX_CYAN);
            tft.setCursor(10, 89);
            tft.print(fan.state == ss::tsStandBy ? "STNBY" : "NORM");
        }
        else
            if ( fstate.state == ss::tsStandBy ) {
                tft.setFont();
                tft.setTextColor(ST77XX_BLACK);
                tft.setCursor(10, 89);
                tft.print(fan.state == ss::tsStandBy ? "SNDBY" : "NORM");
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

    if ( fstate.mstate != fan.mstate ) {  // substate
        tft.setFont();
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(7, 116);
        tft.print("[OFF] [STND-BY] [CNCL]");
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(7, 116);
        switch ( fan.mstate ) {
            case ss::tmsWaitForStandBy:
                tft.print(" OFF  [STND-BY]  CANCEL ");
                break;
            case ss::tmsWaitForOff:
                tft.print("[OFF]  STND-BY   CANCEL ");
                break;
            case ss::tmsWaitForCancel:
                tft.print(" OFF   STND-BY  [CANCEL]");
                break;
        }
        
        fstate.mstate = fan.mstate;
    }    
}

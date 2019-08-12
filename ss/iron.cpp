#include "iron.h"

using namespace sstation;

Iron::Iron(uint8_t iron_pin, 
           uint8_t temp_pin, 
           uint8_t ssensor_pin,
           uint8_t stick_pin
          ) :
           // iron pins
           piron(iron_pin),
           ptemp(temp_pin),
           pssens(ssensor_pin),
           pstchk(stick_pin),
           // on-line properties
           state(tsOff),
           sstate(tssNormal),
           curr_temp(0),
           sel_temp(0),
           wrk_temp(0),
           sby_temp(0),
           power(tplOff),
           time_left(IDLE_TOUT),
           // iron core values
           st(tsOff),
           sst(tssNormal),
           ctemp(0),
           stemp(0),
           wrktemp(IRON_STD_TEMP),
           sbytemp(IRON_SBY_TEMP),
           pwr(tplOff),
           idle_tout(IDLE_TOUT),
           sby_tout(SBY_TOUT),
           appr_tout(APPR_TOUT),
           next_tout(IDLE_TOUT),
           heat_start_time(0),
           sw_prev_state(0),
           sw_last_time(0)
{
    pinMode(piron, OUTPUT);
    pinMode(ptemp, INPUT);
    pinMode(pssens, INPUT_PULLUP);
    pinMode(pstchk, INPUT);

    digitalWrite(piron, 0);
}     
//-----------------------------------------------------------------

void Iron::tick(int enc_value, dbtn::BtnStatus enc_btn) {
}
//-----------------------------------------------------------------

void Iron::tick() {
    
    if ( st == tsOff )
        return;

    // stop heating if time is up
    if ( sst == tssHeat && micros() - heat_start_time > MAX_HEATING_TIME ) {
        digitalWrite(piron, 0);
        sst = tssNormal;
    }

    // check shake sensor
    if ( st != tsOff && millis() - sw_last_time > dbtn::DEBOUNCE_BTN ) {
        if ( digitalRead(pssens) != sw_prev_state) {
            sw_last_time = millis();
            sw_prev_state = digitalRead(pssens);
            next_tout = idle_tout;
            stemp = wrktemp;
            st = tsRun;
        }
        else {
            if ( millis() - sw_last_time > next_tout )
                switch ( tsRun ) {
                    case tsRun:
                        st = tsStandBy;
                        next_tout = sby_tout;
                        stemp = sbytemp;
                        break;

                    case tsStandBy:
                        st = tsOff;
                        stemp = 0;
                        break;
                }
             else
                time_left = (next_tout - (millis() - sw_prev_state)) / 1000;
        }
    }
        
    // check temp
    if ( sst != tssNormal ) // DO NOT check temp until it's heating!
        return;
        
    ctemp = map(analogRead(ptemp), 0, 850, 0, 480);
    curr_temp = ctemp;
    if ( ctemp < stemp ) {
        sst = tssHeat;
        uint16_t diff = stemp - ctemp;
        if ( diff > 150 )
            pwr = tplFull;
        else if ( diff > 50 )
            pwr = tplHigh;
        else if ( diff > 15 )
            pwr = tplMedium;
        else if ( diff > 1 )
            pwr = tplLow;
    }
    else
        pwr = tplOff;
        
    if ( sst == tssHeat ) {
        heat_start_time = micros();
        analogWrite(piron, pwr * 63); // multiplying on 63 instead of 64 to keep max under 255
    }
        
}
//-----------------------------------------------------------------

void Iron::off(ToolState off_state) {
    
    if ( st == tsOff ) 
        return;

    switch ( st ) {
        case tsRun:
            if ( off_state == tsOff ) {
                digitalWrite(piron, 0);
                st = tsOff;
            }
            else if ( off_state == tsStandBy ) {
                st = tsStandBy;
                stemp = sbytemp;
                sel_temp = stemp;
                next_tout = millis() + sby_tout;
                time_left = next_tout;
            }
            break;
            
        case tsStandBy:
            if ( off_state == tsOff ) {
                digitalWrite(piron, 0);
                st = tsOff;
            }
            break;
    }
}
//-----------------------------------------------------------------

void Iron::set_temp(uint16_t temp) {}
//-----------------------------------------------------------------

void Iron::set_timeout(TimeoutType type, uint64_t timeout) {}
//-----------------------------------------------------------------

void Iron::set_sby_temp(uint16_t new_sby_temp) {}
//-----------------------------------------------------------------

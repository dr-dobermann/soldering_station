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
           mstate(tmsNone),
           curr_temp(0),
           sel_temp(0),
           wrk_temp(0),
           sby_temp(0),
           power(tplOff),
           time_left(IDLE_TOUT),
           // iron core values
           st(tsOff),
           sst(tssNormal),
           mst(tmsNone),
           ctemp(0),
           stemp(0),
           wrktemp(IRON_STD_TEMP),
           sbytemp(IRON_SBY_TEMP),
           pwr(tplOff),
           idle_tout(IDLE_TOUT * 1000),
           sby_tout(SBY_TOUT * 1000),
           appr_tout(APPR_TOUT *1000),
           next_tout(IDLE_TOUT * 1000),
           heat_start_time(0),
           sw_prev_state(0),
           sw_last_time(0),
           //kf(50, 0.0005)
           w(4)
{
    pinMode(piron, OUTPUT);
    pinMode(ptemp, INPUT);
    pinMode(pssens, INPUT_PULLUP);
    pinMode(pstchk, INPUT);

    digitalWrite(piron, 0);
}     
//-----------------------------------------------------------------

void Iron::tick(int enc_value, dbtn::BtnStatus enc_btn) {

    if ( enc_value != 0 || enc_btn.bpressed != 0 || enc_btn.lpressed  != 0 ) {
        Serial.print("Iron::tick(");
        Serial.print(enc_value); Serial.print(',');
        Serial.print(enc_btn.bpressed); Serial.print(',');
        Serial.print(enc_btn.lpressed);
        Serial.println(')');
    }
    
    // check encoder long button press
    if ( enc_btn.lpressed > 0 )
        if ( st != tsOff && mst == tmsNone ) {
            mst = tmsWaitForStandBy;
            mstate = mst;
            next_tout = appr_tout;
        }
    
    // check encoder button press
    if ( mst != tmsNone ) {
        if ( enc_btn.bpressed > 0 ) {
            switch ( mst ) {
                case tmsWaitForStandBy:
                    mst = tmsNone;
                    this->off(tsStandBy);
                    break;
                    
                case tmsWaitForOff:
                    mst = tmsNone;
                    this->off(tsOff);
                    break;
                    
                case tmsWaitForCancel:
                    mst = tmsNone;
                    this->on();
                    break;
            }
            mstate = mst;
        }
    }
    else 
        if ( enc_btn.bpressed > 0 ) {
            Serial.print("Iron::check state:"); 
            Serial.println(st);   
            switch ( st ) {
                case tsOff:
                case tsStandBy:
                    this->on();
                    break;
                    
                case tsRun:
                    // TODO: Start turbo mode
                    // or switch between increment/decrtment encoder step (1, 5, 10)
                    break;
            }
        }

    // check encoder rotation
    if ( enc_value != 0 ) {
        if ( mst != tmsNone ) {
            switch (mst) {
                case tmsWaitForCancel:
                    if ( enc_value > 0 )
                        mst = tmsWaitForOff;
                    else if ( enc_value < 0 )
                        mst = tmsWaitForStandBy;
                    break;
            
                case tmsWaitForOff:
                    if ( enc_value > 0 )
                        mst = tmsWaitForStandBy;
                    else if ( enc_value < 0 )
                        mst = tmsWaitForCancel;
                    break;
    
                case tmsWaitForStandBy:
                    if ( enc_value > 0 )
                        mst = tmsWaitForCancel;
                    else if ( enc_value < 0 )
                        mst = tmsWaitForOff;
                    break;
            }
            next_tout = appr_tout;
            mstate = mst;
        }
        else {
            switch ( st ) {
                case tsOff:
                    break;
    
                case tsRun:
                    this->set_temp(tmpWork, stemp + enc_value);
                    stemp = wrktemp;
                    sel_temp = stemp;
                    break;
    
                case tsStandBy:
                    this->on();
                    break;
            }
        }
    }
    if ( enc_value != 0 || enc_btn.bpressed != 0 || enc_btn.lpressed != 0 )
        sw_last_time = millis();

    // run standard routine
    this->tick();
}
//-----------------------------------------------------------------

void Iron::tick() {
    
    if ( st == tsOff )
        return;

    // stop heating if time is up (MICRO!!!)
    if ( sst == tssHeat && micros() - heat_start_time > MAX_HEATING_TIME ) {
        digitalWrite(piron, 0);
        sst = tssNormal;
    }

    // check shake sensor
    if ( st != tsOff && millis() - sw_last_time > dbtn::DEBOUNCE_BTN ) {
        int nssens = digitalRead(pssens);
        if ( nssens != sw_prev_state) {
            sw_prev_state = nssens;
            this->on();
        }
    }
    
    if ( millis() - sw_last_time > next_tout )
        switch ( st ) {
            case tsRun:
                if ( mst != tmsNone )
                    this->off(tsStandBy);
                else 
                    this->on();
                break;

            case tsStandBy:
                this->off(tsOff);
                break;
        }
    else    
        time_left = (uint16_t)((next_tout - (millis() - sw_last_time)) / 1000);
    
        
    // check temp
    if ( sst == tssHeat ) // DO NOT check temp until it's heating!
        return;

    // make 4 temperature measurements in a row
    // and get an average from them    
    uint64_t temp = 0;
    for ( int i = 0; i < 4; i++ ) {
        temp += analogRead(ptemp);
        delay(25);
    }
    temp >>= 2;
    ctemp = map((uint16_t)temp, 0, 1024, 27, 480);
    //Serial.println(ctemp);
    //ctemp = (uint16_t)kf.filter(temp);
    ctemp = w.filter(ctemp);
    curr_temp = ctemp;
    if ( ctemp < stemp ) {
        sst = tssHeat;
        uint16_t diff = stemp - ctemp;
        if ( diff > 75 )
            pwr = tplFull;
        else if ( diff > 25 )
            pwr = tplHigh;
        else if ( diff > 10 )
            pwr = tplMedium;
        else if ( diff > 1 )
            pwr = tplLow;
    }
    else
        pwr = tplOff;

    power = pwr;
        
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
                state = st;
                sst = tssNormal;
                Serial.println("Iron::off");
            }
            else if ( off_state == tsStandBy ) {
                st = tsStandBy;
                state = st;
                stemp = sbytemp;
                sel_temp = stemp;
                next_tout = sby_tout;
                time_left = (uint16_t)(sby_tout / 1000);
                Serial.println("Iron::stand-by");
            }
            break;
            
        case tsStandBy:
            if ( off_state == tsOff ) {
                digitalWrite(piron, 0);
                st = tsOff;
                state = st;
            }
            break;
    }
}
//-----------------------------------------------------------------

void Iron::set_temp(TempType type, uint16_t temp) {
    
    if ( temp > IRON_MAX_TEMP )
        temp = IRON_MAX_TEMP;

    if ( type == tmpWork ) {
        wrktemp = temp;
        wrk_temp = temp;
    }
    else {
        sbytemp = temp;
        sby_temp = temp;
    }
}
//-----------------------------------------------------------------

void Iron::set_timeout(TimeoutType type, uint64_t timeout) {}
//-----------------------------------------------------------------

void Iron::on() {
    
    // TODO: add check iron procedure
    if ( st != tsRun ) {
        stemp = wrktemp;
        sel_temp = stemp;
        st = tsRun;
        state = st;
    }
    mst = tmsNone;
    mstate = mst;
    next_tout = idle_tout;
    time_left = (uint16_t)(idle_tout / 1000);
    sw_last_time = millis();
    
    Serial.println("Iron::on");
}
//-----------------------------------------------------------------

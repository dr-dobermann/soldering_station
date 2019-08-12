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
           sby_temp(0),
           power(tplOff),
           time_left(IDLE_TOUT),
           // iron core values
           st(tsOff),
           sst(tssNormal),
           ctemp(0),
           stemp(IRON_STD_TEMP),
           sbytemp(IRON_SBY_TEMP),
           pwr(tplOff),
           idle_tout(IDLE_TOUT),
           sby_tout(SBY_TOUT),
           appr_tout(APPR_TOUT),
           next_tout(IDLE_TOUT)
{
    pinMode(piron, OUTPUT);
    pinMode(ptemp, INPUT);
    pinMode(pssens, INPUT_PULLUP);
    pinMode(pstchk, INPUT);

    digitalWrite(piron, 0);
}     
//-----------------------------------------------------------------

void Iron::tick(int enc_value, dbtn::BtnStatus enc_btn) {}
//-----------------------------------------------------------------

void Iron::tick() {
}
//-----------------------------------------------------------------

void Iron::on() {}
//-----------------------------------------------------------------

void Iron::off(ToolState off_state) {
    
    if ( st == tsOff ) 
        return;

    switch ( st ) {
        case tsRun:
        case tsIdle:
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

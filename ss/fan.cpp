#include "fan.h"

using namespace sstation;

Fan::Fan(uint8_t heater_pin,
         uint8_t fan_pin, 
         uint8_t temp_pin, 
         uint8_t msensor_pin
        ) :
         // fan pins
         phtr(heater_pin),
         pfan(fan_pin),
         ptemp(temp_pin),
         pmsens(msensor_pin),
         // properties
         state(tsRun),
         sstate(tssWaitToStandBy),
         curr_temp(275),
         sel_temp(350),
         speed(tplFull),
         power(tplMedium),
         time_left(138),
         // fan core values
         st(tsOff),
         sst(tssNormal),
         ctemp(0),
         stemp(0),
         spd(tplOff),
         pwr(tplOff),
         idle_tout(IDLE_TOUT),
         sby_tout(SBY_TOUT),
         appr_tout(APPR_TOUT),
         next_tout(0)
         
{
    pinMode(phtr, OUTPUT);
    pinMode(pfan, OUTPUT);
    pinMode(ptemp, INPUT);
    pinMode(pmsens, INPUT_PULLUP);

    digitalWrite(phtr, 0);
    digitalWrite(pfan, 0);
}
//-----------------------------------------------------------------

void Fan::tick(int enc_value, dbtn::BtnStatus enc_btn) {
    
}
//-----------------------------------------------------------------

void Fan::tick() {
    
}
//-----------------------------------------------------------------

void Fan::on() {
    
}
//-----------------------------------------------------------------

void Fan::off(ToolState off_state) {
    
}
//-----------------------------------------------------------------

void Fan::set_temp(uint16_t temp) {
    
}
//-----------------------------------------------------------------

void Fan::set_speed(ToolPowerLevel speed) {
    
}
//-----------------------------------------------------------------

void Fan::set_timeout(TimeoutType type, uint64_t timeout) {
    
}
//-----------------------------------------------------------------

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
           iron_state(tsOff),
           iron_sstate(tssNormal),
           curr_temp(0),
           sel_temp(0),
           iron_power(tplOff),
           time_left(0),
           // iron core values
           state(tsOff),
           sstate(tssNormal),
           ctemp(0),
           stemp(0),
           pwr(tplOff),
           idle_tout(IDLE_TOUT),
           sby_tout(SBY_TOUT),
           appr_tout(APPR_TOUT),
           next_tout(0)
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

void Iron::on() {}
//-----------------------------------------------------------------

void Iron::off(ToolState off_state) {}
//-----------------------------------------------------------------

void Iron::set_temp(uint16_t temp) {}
//-----------------------------------------------------------------

void Iron::set_timeout(TimeoutType type, int64_t timeout) {}
//-----------------------------------------------------------------

#include "encoder.h"

using namespace encdr;
Encoder::Encoder(byte encA, byte encB) :
            pa(encA),
            pb(encB),
            value(0),
            epos(0),
            enc_change_time(0)
{
    pinMode(pa, INPUT_PULLUP);
    pinMode(pb, INPUT_PULLUP);    
}
//-----------------------------------------------------------------
            
void Encoder::tick() {
    
    if ( millis() - enc_change_time > DEBOUNCE_ENC ) {
        uint8_t new_epos = (digitalRead(pa) << 1) | digitalRead(pb);
        value += ECDR_SHIFTS[new_epos * 4 + epos];
        epos = new_epos;

        enc_change_time = millis();
    }
}
//-----------------------------------------------------------------

int Encoder::get_value() {
                 
    int val = value;
    value = 0;
            
    return val;
};
//-----------------------------------------------------------------

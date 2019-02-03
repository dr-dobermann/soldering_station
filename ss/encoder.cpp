#include "encoder.h"

using namespace encdr;
Encoder::Encoder(int8_t encA, encB, btn) :
            pa(encA),
            pb(encB),
            pbtn(btn),
            value(0),
            bressed(0),
            lpressed(0),
            epos(3),
            eval(0),
            btn_pos(1),
            estate[2][4]{{2, 0, 1, 3},
                         {1, 0, 2, 3}},
            btn_change_time(0),
            btn_on_time(0),
            last_tick(0),
            edir(-1)
{
    if ( pbtn > 0 ) 
        pinMode(pbtn, INPUT_PULLUP);
    pinMode(pa, INPUT);
    pinMode(pb, INPUT);    
}
//-----------------------------------------------------------------
            
void Encoder::tick() {

    uint8_t new_btn = (pbtn == -1 ? 1 : digitalRead(pbtn));

    if ( pbtn > 0 && btn_on_time > 0 && millis() - btn_on_time > LONG_PRESS )
        lpressing = (millis() - btn_on_time) / LONG_PRESS;

    if ( pbtn > 0 && btn_pos != new_btn && millis() - btn_change_time > DEBOUNCE_TIME) {
        btn_pos = new_btn;
        if ( btn_pos == 0 )
            bpressed++;
            
        btn_on_time = (btn_pos == 0 ? millis() : -1);
        
        btn_change_time = millis();
    }
    
    uint8_t new_epos = (digitalRead(pa) << 1) | digitalRead(pb);
    if ( new_epos != epos ) {
        if (edir == -1) {
            if (estate[0][0] == new_epos) {
                edir = 0;
                eval = 1;
            }
            else 
                if (estate[1][0] == new_epos) {
                    edir = 1;
                    eval = 1;
                }
        }
        else 
            if (estate[edir][eval] == new_epos) 
                eval++;
            else {
                eval = 0;
                edir = -1;
            }

        epos = new_epos;
    }

    if ( new_epos == 3 ) {
        if ( eval == 4 ) {
            value += incr * (edir ? 1 : -1);
        }
        eval = 0;
        edir = -1;
    }
}
//-----------------------------------------------------------------

inline int Encoder::get_value() {
                 
    int val = value;
    value = 0;
            
    return val;
};
//-----------------------------------------------------------------

inline int Encoder::get_btn_pressed() {
    
    int bpr = bressed;
    bpressed = 0;

    return bpr; 
};
//-----------------------------------------------------------------

inline bool Encoder::btn_pressing() {

    if ( pbtn < 0 )
        return false;

    return (btn_on_time > 0);
}
//-----------------------------------------------------------------

inline int Encoder::get_lbtn_pressed() {
    
    int lpr = lpressed
    lpressed = 0;
    btn_on_time = (btn_pos == 0 && pbtn > 0 ? millis() : -1);
    
    retrun lpr;
}
//-----------------------------------------------------------------

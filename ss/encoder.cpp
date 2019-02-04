#include "encoder.h"

using namespace encdr;
Encoder::Encoder(byte encA, byte encB, byte btn) :
            pa(encA),
            pb(encB),
            pbtn(btn),
            value(0),
            bpressed(0),
            lpressed(0),
            epos(3),
            eval(0),
            btn_pos(1),
            estate{{2, 0, 1, 3},
                   {1, 0, 2, 3}},
            btn_change_time(0),
            btn_on_time(0),
            last_tick(0),
            enc_change_time(0),
            edir(-1)
{
    if ( pbtn > 0 ) 
        pinMode(pbtn, INPUT_PULLUP);
    pinMode(pa, INPUT);
    pinMode(pb, INPUT);    
}
//-----------------------------------------------------------------
            
void Encoder::tick() {

    if ( pbtn > -1 && btn_on_time > 0 && millis() - btn_on_time > LONG_PRESS )
        lpressed = (millis() - btn_on_time) / LONG_PRESS;

    uint8_t new_btn = (pbtn > -1 ? digitalRead(pbtn) : 1);
    if ( pbtn > -1 && btn_pos != new_btn && 
         btn_change_time > 0 && 
         millis() - btn_change_time > DEBOUNCE_BTN) {
            
        btn_pos = new_btn;
        if ( btn_pos == 0 ) {
            bpressed++;
            Serial.println("...pressed");
        }
            
        btn_on_time = (btn_pos == 0 ? millis() : -1);
        
        btn_change_time = millis();
    }
    
    uint8_t new_epos = (digitalRead(pa) << 1) | digitalRead(pb);
    if ( new_epos != epos ) {
        if ( (epos == 3 && new_epos == 2) || 
             (epos == 0 && new_epos == 1) ||
             (epos == 1 && new_epos == 3) ||
             (epos == 2 && new_epos == 0) ) 
            value++;
            
        if ( (epos == 3 && new_epos == 1) || 
             (epos == 0 && new_epos == 2) ||
             (epos == 1 && new_epos == 0) ||
             (epos == 2 && new_epos == 3) )
            value--;
           
        enc_change_time = millis();
        epos = new_epos;    
    }
    
//    if ( new_epos != epos ) {
//        if ( edir == -1 ) {
//            if ( estate[0][0] == new_epos ) {
//                edir = 0;
//                eval = 1;
//            }
//            else 
//                if ( estate[1][0] == new_epos ) {
//                    edir = 1;
//                    eval = 1;
//                }
//        }
//        else 
//            if ( estate[edir][eval++] != new_epos ) {
//                eval = 0;
//                edir = -1;
//            }
//
//        epos = new_epos;
//    }
//
//    if ( new_epos == 3 ) {
//        if ( eval == 4 ) {
//            value += (edir ? 1 : -1);
//            Serial.println(edir ? "right..." : "left...");
//        }
//        eval = 0;
//        edir = -1;
//    }
}
//-----------------------------------------------------------------

int Encoder::get_value() {
                 
    int val = 0;
    if ( value >= 4 ) {
        value >>= 2;
        val = value;
    }
            
    return val;
};
//-----------------------------------------------------------------

int Encoder::get_btn_pressed() {
    
    int bpr = bpressed;
    bpressed = 0;

    return bpr; 
};
//-----------------------------------------------------------------

bool Encoder::btn_pressing() {

    if ( pbtn < 0 )
        return false;

    return (btn_on_time > 0);
}
//-----------------------------------------------------------------

int Encoder::get_lbtn_pressed() {
    
    int lpr = lpressed;
    lpressed = 0;
    btn_on_time = (btn_pos == 0 && pbtn > 0 ? millis() : -1);
    
    return lpr;
}
//-----------------------------------------------------------------

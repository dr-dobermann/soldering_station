#include "button.h"

using namespace dbtn;

Button::Button(uint8_t btn_pen) :
    pbtn(btn_pen),
    btn_pos(1),
    btn_change_time(0),
    btn_on_time(0),
    bpressed(0),
    lpressed(0)
{
    pinMode(pbtn, INPUT_PULLUP);
}
//-----------------------------------------------------------------
            
void Button::tick() {
    
    uint8_t new_btn = digitalRead(pbtn);
    if ( btn_pos != new_btn &&  
         millis() - btn_change_time > DEBOUNCE_BTN) {
            
        btn_pos = new_btn;
        if ( btn_pos == 0 && lpressed == 0 ) 
            bpressed++;                      
            
        btn_on_time = (btn_pos == 0 ? millis() : -1);
        
        btn_change_time = millis();
    }
    
    if ( btn_on_time > 0 && millis() - btn_on_time > LONG_PRESS )
        lpressed = (millis() - btn_on_time) / LONG_PRESS;
}
//-----------------------------------------------------------------

int Button::get_btn_pressed() {
    
    int bpr = bpressed;
    bpressed = 0;

    return bpr; 
};
//-----------------------------------------------------------------

bool Button::btn_pressing() {

    return (btn_on_time > 0);
}
//-----------------------------------------------------------------

int Button::get_lbtn_pressed() {
    
    int lpr = lpressed;
    // clearing collected data
    btn_on_time = (lpressed > 0 && btn_on_time > 0 ? millis() : btn_on_time);
    lpressed = 0;
        
    return lpr;
}
//-----------------------------------------------------------------

BtnStatus Button::get_status() {
    BtnStatus bs = {bpressed, 
                    lpressed,
                    btn_on_time > 0};

    // clearing collected data
    bpressed = 0;
    btn_on_time = (lpressed > 0 && btn_on_time > 0 ? millis() : btn_on_time);
    lpressed = 0;

    return bs;
}
//-----------------------------------------------------------------

#ifndef encoder_h
#define encoder_h

#include <Arduino.h>
namespace encdr {
    
const long 
    DEBOUNCE_ENC = 20,
    DEBOUNCE_BTN = 80,
    LONG_PRESS = 500;

class Encoder {
    public:
        Encoder(byte encA, byte encB, byte btn);
        
        // should be called to update encoder state 
        void tick();
        
        // return accumulated value and clears value
        int get_value();

        // return count of button's pressings
        int get_btn_pressed();
        bool btn_pressing();

        int get_lbtn_pressed();
                
    protected:
        // encoder's pins button could be -1 what means it's not presented
        int8_t pa, pb, pbtn;

        // encoder value accumulator
        int value;

        // button pressing counter
        int bpressed;

        // button long pressing counter
        int lpressed;
        
        uint8_t epos, eval, btn_pos;
        uint8_t estate[2][4] = {{2, 0, 1, 3},
                                {1, 0, 2, 3}};
                        
        long btn_change_time, 
             btn_on_time,
             last_tick,
             enc_change_time;
        
        int8_t edir;        
};


};

#endif // encoder_h

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <Arduino.h>

namespace dbtn {

    const long
        LONG_PRESS = 400,
        DEBOUNCE_BTN = 50;

    typedef struct {
            int bressed;
            int lpressed;
            bool pressed;
        } BtnStatus;

    class Button {
        public:
            Button(uint8_t btn_pin);

            // update button status
            void tick();
            
            // return count of button's pressings
            int get_btn_pressed();
            int get_lbtn_pressed();      
                  
            // if button is still pressed
            bool btn_pressing();

            // get and clear all data of the button
            BtnStatus get_status();

        private:
            uint8_t pbtn; // button pin

            // button pressing counter
            int bpressed;

            // button long pressing counter
            int lpressed;
        
            uint8_t btn_pos;

            long btn_change_time, 
                 btn_on_time;            
    };
    
}; // dbtn


#endif // _BUTTON_H_

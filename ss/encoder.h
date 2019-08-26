#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <Arduino.h>

namespace encdr {
    
    const uint64_t
        MAX_CHECK_TIME = 20, 
        DEBOUNCE_ENC = 100; // in MICROseconds

    const int8_t ECDR_SHIFTS[] = { 0,  1, -1,  0,  // previous state is on X 
                                  -1,  0,  0,  1,  // the new state is on Y
                                   1,  0,  0, -1,
                                   0, -1,  1,  0};
                                   
    class Encoder {
        public:
            Encoder(byte encA, byte encB);
            
            // should be called to update encoder state 
            void tick();
            
            // return accumulated value and clears value
            int get_value();
                    
        protected:
            // encoder's pins
            int8_t pa, pb;
    
            // encoder value accumulator
            int value;
    
            uint8_t epos;
                            
            uint64_t enc_change_time;  
            
    };
    
}; // encdr

#endif // _ENCODER_H_

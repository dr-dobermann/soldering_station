#ifndef _IRON_H_
#define _IRON_H_

#include <Arduino.h>

#include "tool.h"
#include "button.h"

namespace sstation {
        
    class Iron {
        public:
            Iron(uint8_t iron_pin, 
                 uint8_t temp_pin, 
                 uint8_t ssensor_pin, // shake sensor
                 uint8_t stick_pin);  // stick check line

            // run one cycle for the tool
            void tick(int enc_value, dbtn::BtnStatus enc_btn);

            void on();
            void off(ToolState off_state); // could be ttOff or ttStandBy
            void set_temp(uint16_t temp);
            void set_timeout(TimeoutType type, int64_t timeout);

            // configuration tool properties
            inline int64_t get_timeout(TimeoutType type, int64_t timeout) {
                switch (type) {
                    case ttIdle:    return idle_tout;
                    case ttStandBy: return sby_tout;
                    case ttApprove: return appr_tout;
                }
                return -1; // invalid timeout type
            }; // get_timeout 

            // tool properties. Only for reading
            // Rewriting has no effect on the tool
            ToolState iron_state;
            ToolSubState iron_sstate;
            
            uint16_t curr_temp,
                     sel_temp;
                     
            ToolPowerLevel iron_power;
                           
            int64_t time_left;   // time left to idle, stand-by, off state or for approving 
                    
        private:
            uint8_t piron,
                    ptemp,
                    pssens,
                    pstchk;
                    
            ToolState state;
            ToolSubState sstate;
            
            uint16_t ctemp,
                     stemp;
                     
            ToolPowerLevel pwr;

            int64_t idle_tout,
                    sby_tout,
                    appr_tout,
                    next_tout;   // timelimit to next state or substate

    }; // class Iron
    
}; // namespace sstation

#endif // _IRON_H_

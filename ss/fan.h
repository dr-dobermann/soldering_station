#ifndef _FAN_H_
#define _FAN_H_

#include <Arduino.h>

#include "tool.h"
#include "button.h"

namespace sstation {

    class Fan {
        public:
            Fan(uint8_t heater_pin,
                uint8_t fan_pin, 
                uint8_t temp_pin, 
                uint8_t msensor_pin);  // magnetic sensor

            // run one cycle for the tool
            void tick(int enc_value, dbtn::BtnStatus enc_btn);

            void on();
            void off(ToolState off_state); // could be ttOff or ttStandBy
            void set_temp(uint16_t temp);
            void set_fan_speed(ToolPowerLevel speed);
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
            
            // on-line tool properties. Only for reading
            // Rewriting has no effect on the tool
            ToolState htr_state;
            ToolSubState htr_sstate;
            
            uint16_t curr_temp,
                     sel_temp;
                     
            ToolPowerLevel fan_speed,
                           heater_rate;
                           
            int64_t time_left;   // time left to idle, stand-by, off state or for approving 
                    
        private:
            uint8_t phtr,
                    pfan,
                    ptemp,
                    pmsens;
                    
            ToolState state;
            ToolSubState sstate;
            
            uint16_t ctemp,
                     stemp;
                     
            ToolPowerLevel fspeed,
                           hrate;

            int64_t idle_tout,
                    sby_tout,
                    appr_tout,
                    next_tout;   // timelimit to next state or substate 
            
    }; // class Fan
    
}; // namespace sstation
#endif // _FAN_H_

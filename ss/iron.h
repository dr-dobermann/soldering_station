#ifndef _IRON_H_
#define _IRON_H_

#include <Arduino.h>

#include "tool.h"
#include "button.h"

namespace sstation {

    const uint16_t IRON_STD_TEMP = 250,
                   IRON_MAX_TEMP = 480,
                   IRON_SBY_TEMP = 150;

    const uint64_t MAX_HEATING_TIME = 1; // MICROseconds
        
    class Iron {
        public:
            Iron(uint8_t iron_pin, 
                 uint8_t temp_pin, 
                 uint8_t ssensor_pin, // shake sensor
                 uint8_t stick_pin);  // stick check line

            // run one cycle for the tool
            void tick(int enc_value, dbtn::BtnStatus enc_btn);
            void tick(); // background call

            void off(ToolState off_state); // could be ttOff or ttStandBy
            void set_temp(TempType type, uint16_t temp);
            void set_timeout(TimeoutType type, uint64_t timeout);

            // configuration tool properties
            inline uint64_t get_timeout(TimeoutType type) {
                switch (type) {
                    case ttIdle:    return idle_tout;
                    case ttStandBy: return sby_tout;
                    case ttApprove: return appr_tout;
                }
                return -1; // invalid timeout type
            }; // get_timeout 

            // tool properties. Only for reading
            // Rewriting has no effect on the tool
            ToolState state;
            ToolMenuState mstate;
            
            uint16_t curr_temp,
                     sel_temp,
                     wrk_temp,
                     sby_temp;
                     
            ToolPowerLevel power;
                           
            uint16_t time_left;   // time left in seconds to idle, 
                                  // stand-by, off state or for approving 
                    
        private:
            uint8_t piron,
                    ptemp,
                    pssens,
                    pstchk;
                    
            ToolState st;
            ToolSubState sst;
            ToolMenuState mst;
            
            uint16_t ctemp,
                     stemp,
                     wrktemp,
                     sbytemp;
                     
            ToolPowerLevel pwr;

            uint64_t idle_tout,
                     sby_tout,
                     appr_tout,
                     next_tout;   // timelimit to next state or substate

            uint64_t heat_start_time; // heating starting time in MICROseconds

            uint8_t sw_prev_state;   // iron shake sensor previous state;
            uint64_t sw_last_time;   // last time shake sensor check

            //Kalman kf; // kalman filter for iron temperature
            WMA w;

            void on();

    }; // class Iron
    
}; // namespace sstation

#endif // _IRON_H_

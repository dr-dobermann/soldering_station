#ifndef _TOOL_H_
#define _TOOL_H_

#include <Arduino.h>

namespace sstation {

    const uint32_t 
        IDLE_TOUT = 10 * 60,   // default timeout to switch to stand-by mode
        SBY_TOUT = 3 * 60,    // default timeout to switch off
        APPR_TOUT = 5;     // default approving timeout

    const int8_t MAX_WMA_SIZE = 16;
            
    typedef enum {
        tplOff,
        tplLow,
        tplMedium,
        tplHigh,
        tplFull
    } ToolPowerLevel;

    typedef enum {
        tsOff,
        tsShuttingDown, // only used by fan since it should cool down the heater
        tsStandBy,
        tsRun
    } ToolState;

    typedef enum {
        tssNormal,          // heater not run
        tssHeat,          
    } ToolSubState;

    typedef enum {
        tmsNone,
        tmsWaitForStandBy,  // user select to switch tool to stand-by mode and should approve it
        tmsWaitForOff,      // user select to switch off the tool and should approve it
        tmsWaitForCancel    // user don't want to turn the tool off or stand it by
    } ToolMenuState;

    typedef enum {
        ttIdle,
        ttStandBy,
        ttApprove       // timeout to approve to switch tool off or send it to stand-by mode
    } TimeoutType;

    typedef enum {
        tmpWork,
        tmpStandBy,
        tmpCooled       // heater gun maximum temp on turning off
    } TempType;
//
//    class Kalman {  // Kalman filter
//        public:
//            Kalman( float vv, float pp);
//
//            float filter(float value);
//
//        private:
//        
//            float varVolt;    // average dispersion
//            float varProcess; // reaction speed
//            float   Pc,
//                    G,
//                    P,
//                    Xp,
//                    Zp,
//                    Xe;
//    };

    class WMA {  // weighted moving average
        public:
            WMA(uint8_t m_count);

            uint16_t filter(uint16_t value);

            ~WMA();
            
        private:
            uint16_t* m;  // measurements array

            int8_t pos;       // current position to insert a new value
            uint8_t cap, len; // capacity and current length of m
    }; // WMA
}; // namespace sstation

#endif // _TOOL_H_

#ifndef _TOOL_H_
#define _TOOL_H_

namespace sstation {

    const int64_t 
        IDLE_TOUT = 3 * 60 * 1000,   // default timeout to switch to stand-by mode
        SBY_TOUT = 5 * 60 * 1000,    // default timeout to switch off
        APPR_TOUT = 5 * 1000;        // default approving timeout
            
    typedef enum {
        tplOff,
        tplLow,
        tplMedium,
        tplHigh,
        tplFull
    } ToolPowerLevel;

    typedef enum {
        tsOff,
        tsStandBy,
        tsIdle,
        tsRun,
        tsCheck,        // only used by iron
        tsTempCheck     // only used by iron
    } ToolState;

    typedef enum {
        tssNormal,          
        tssWaitToStandBy,   // user select to switch tool to stand-by mode and should approve it
        tssWaitToOff,       // user select to switch off the tool and should approve it
        tssWaitToCancel     // user don't want to turn the tool off or stand it by
    } ToolSubState;

    typedef enum {
        ttIdle,
        ttStandBy,
        ttApprove       // timeout to approve to switch tool off or send it to stand-by mode
    } TimeoutType;
    
}; // namespace sstation

#endif // _TOOL_H_

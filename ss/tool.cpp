#include "tool.h"

using namespace sstation;

Kalman::Kalman( float vv = 0.25, float vp = 0.05) :
    Pc(0.0),
    G(0.0),
    P(1.0),
    Xp(0.0),
    Zp(0.0),
    Xe(0.0)
{
    varProcess = vp;
    varVolt = vv;
}
//-----------------------------------------------------------------

float Kalman::filter(float value) {
//    Pc = P + varProcess;
//    G = Pc / (Pc + varVolt);
//    P = (1 - G) * Pc;
//    Xp = Xe;
//    Zp = Xp;
//    Xe = G * (value - Zp) + Xp;

  Xe = varProcess * value + (1 - varProcess) * Xe;
  
  return Xe; 
}
//-----------------------------------------------------------------

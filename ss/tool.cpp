#include "tool.h"

using namespace sstation;

//Kalman::Kalman( float vv = 0.25, float vp = 0.05) :
//    Pc(0.0),
//    G(0.0),
//    P(1.0),
//    Xp(0.0),
//    Zp(0.0),
//    Xe(0.0)
//{
//    varProcess = vp;
//    varVolt = vv;
//}
////-----------------------------------------------------------------
//
//float Kalman::filter(float value) {
////    Pc = P + varProcess;
////    G = Pc / (Pc + varVolt);
////    P = (1 - G) * Pc;
////    Xp = Xe;
////    Zp = Xp;
////    Xe = G * (value - Zp) + Xp;
//
//  Xe = varProcess * value + (1 - varProcess) * Xe;
//  
//  return Xe; 
//}
//-----------------------------------------------------------------

WMA::WMA(uint8_t m_count) :
    cap(m_count),
    len(0),
    pos(0)
{
    if ( cap > MAX_WMA_SIZE )
        cap = MAX_WMA_SIZE;
    if ( cap == 0 )
        cap = 4; 
        
    m = new uint16_t[cap];
}
//-----------------------------------------------------------------

uint16_t WMA::filter(uint16_t value) {

    m[pos++] = value;
    if ( pos > cap - 1 )
        pos = 0;
    if ( len < cap )
        len++;

    uint64_t w = 0, f = 0;
    for ( int n = len, p = pos - 1; n > 0; n-- ) {
        if ( p < 0 )
            p = cap - 1;
        w += m[p--] * n;
        f += n;
    }
    
    return (uint16_t)(w/f);
}
//-----------------------------------------------------------------

WMA::~WMA() {
    if ( m != NULL )
        delete[] m;    
}
//-----------------------------------------------------------------

/*
    Analog sound emulation library

    Copyright (c) 2004 Alessandro Scotti
*/
#ifndef ASE_TIMER555_LINEAR_RAMP_H_
#define ASE_TIMER555_LINEAR_RAMP_H_

#include "ase.h"

/*
    555 timer in astable/linear-ramp configuration.

    Although this is still an astable setup, here the capacitor doesn't follow the
    usual charge/discharge path using resistors but is charged thru a current
    source (typically, a transistor).
*/
class ATimer555LinearRamp : public AChannel
{
public:
    ATimer555LinearRamp( AFloat c );

    void setVcc( AFloat vcc );

    void setCurrent( AChannel * current );

    void dischargeCapacitor() {
        c_voltage_ = 0;
    }

protected:
    virtual void updateBuffer( AFloat * buf, unsigned len, unsigned ofs );

private:
    void setCapacitorCoefficients();

    AFloat c_;
    AFloat vcc_;
    AFloat threshold_hi_;
    AFloat threshold_lo_;
    AFloat c_dt_;

    AChannel * current_;

    int flipflop_;

    AFloat c_voltage_;  // Voltage across capacitor
};

#endif // ASE_TIMER555_LINEAR_RAMP_H_

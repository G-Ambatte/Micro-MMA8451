#ifndef _CUSTOM8451_H_
#define _CUSTOM8451_H_

#include <Adafruit_MMA8451.h>

#define MMA8451_INT_SRC           0x0C
#define MMA8451_PULSE_CFG         0x21
#define MMA8451_PULSE_SRC         0x22
#define MMA8451_PULSE_THSX        0x23
#define MMA8451_PULSE_THSY        0x24
#define MMA8451_PULSE_THSZ        0x25
#define MMA8451_PULSE_TMLT        0x26
#define MMA8451_PULSE_LTCY        0x27
#define MMA8451_PULSE_WIND        0x28
// Defined in Adafruit_MMA8451.h
//#define MMA8451_REG_CTRL_REG1     0x2A
//#define MMA8451_REG_CTRL_REG2     0x2B
//#define MMA8451_REG_CTRL_REG4     0x2D
//#define MMA8451_REG_CTRL_REG5     0x2E

class Custom_MMA8451 : public Adafruit_MMA8451 {
  public:
    bool initializeTap(void) {
      writeRegister8(MMA8451_REG_CTRL_REG1, 0x10); //200 Hz, Standby Mode
      writeRegister8(MMA8451_PULSE_CFG, 0x3F); // 0x7F - Single and Double tap
      writeRegister8(MMA8451_PULSE_THSX, 0x20); // X Threshold set to 2G (0x20)
      writeRegister8(MMA8451_PULSE_THSY, 0x20); // Y Threshold set to 2G (0x20)
      writeRegister8(MMA8451_PULSE_THSZ, 0x40); // Z Threshold set to 4G (0x40)
      writeRegister8(MMA8451_PULSE_TMLT, 0x18); // Pulse time limit set to 60 ms (0x18)
      writeRegister8(MMA8451_PULSE_LTCY, 0x28); // Pulse latency set to 200ms (0x28)
      writeRegister8(MMA8451_PULSE_WIND, 0x3C); // Pulse window set to 300ms (0x3C)
      writeRegister8(MMA8451_REG_CTRL_REG4, 0x08); //Enable Pulse Interrupt Block in System CTRL_REG4
      writeRegister8(MMA8451_REG_CTRL_REG5, 0x08); //Route Pulse Interrupt Block to INT1 hardware Pin
      uint8_t CTRL_REG1_Data = readRegister8(MMA8451_REG_CTRL_REG1); //Read contents of control register
      CTRL_REG1_Data |= 0x01;  //Change the value in the register to Active Mode.
      writeRegister8(MMA8451_REG_CTRL_REG1, CTRL_REG1_Data);  //Write in the updated value to put the device in Active Mode
      
      return 1;
    };
    uint8_t getTapEvents(void) {
      if (readRegister8(MMA8451_INT_SRC & 0x08 == 0x08)) {
        return (uint8_t)(readRegister8(MMA8451_PULSE_SRC));
      } else {
        return 0;
      }
    };
};


#endif

#ifndef AXS15231B_Touch_H
#define AXS15231B_Touch_H

#include <Arduino.h>
#include <Wire.h>

class AXS15231B_Touch {
public:
    uint8_t scl, sda, int_pin, addr, rotation;
    AXS15231B_Touch(uint8_t scl, uint8_t sda, uint8_t int_pin, uint8_t addr, uint8_t rotation) {
        this->scl = scl;
        this->sda = sda;
        this->int_pin = int_pin;
        this->addr = addr;
        this->rotation = rotation;
    }
    bool begin();
    bool touched();
    void setRotation(uint8_t);
    void readData(uint16_t *, uint16_t *);
    void enOffsetCorrection(bool);
    void setOffsets(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);

protected:
    volatile bool touch_int = false;

private:
    bool en_offset_correction = false;
    uint16_t point_X = 0, point_Y = 0;
    uint16_t x_real_min, x_real_max, y_real_min, y_real_max;
    uint16_t x_ideal_max, y_ideal_max;

    bool update();
    void correctOffset(uint16_t *, uint16_t *);
    static void isrTouched();
    static AXS15231B_Touch* instance;
};


#define AXS_GET_GESTURE_TYPE(buf)  (buf[0])
#define AXS_GET_FINGER_NUMBER(buf) (buf[1])
#define AXS_GET_EVENT(buf)         ((buf[2] >> 0) & 0x03)
#define AXS_GET_POINT_X(buf)       (((uint16_t)(buf[2] & 0x0F) <<8) + (uint16_t)buf[3])
#define AXS_GET_POINT_Y(buf)       (((uint16_t)(buf[4] & 0x0F) <<8) + (uint16_t)buf[5])

#ifndef ISR_PREFIX
  #if defined(ESP8266)
    #define ISR_PREFIX ICACHE_RAM_ATTR
  #elif defined(ESP32)
    #define ISR_PREFIX IRAM_ATTR
  #else
    #define ISR_PREFIX
  #endif
#endif
#endif

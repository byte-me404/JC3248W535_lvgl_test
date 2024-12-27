#include "AXS15231B_touch.h"



AXS15231B_Touch* AXS15231B_Touch::instance = nullptr;



bool AXS15231B_Touch::begin() {
    instance = this;

    // Attach interrupt. Interrupt -> display touched
    attachInterrupt(digitalPinToInterrupt(int_pin), isrTouched, FALLING);

    // Start I2C
    return (Wire.begin(sda, scl));
}

ISR_PREFIX
void AXS15231B_Touch::isrTouched() {
    // This ISR gets executed if the display reports a touch interrupt
    if (instance) {
        instance->touch_int = true;
    }
}

void AXS15231B_Touch::setRotation(uint8_t rot) {
    rotation = rot;
}

bool AXS15231B_Touch::touched() {
    // Check if the display is touched / got touched
    return update();
}

void AXS15231B_Touch::readData(uint16_t *x, uint16_t *y) {
    // Return the latest data points
    *x = point_X;
    *y = point_Y;
}

void AXS15231B_Touch::enOffsetCorrection(bool en) {
    // Enable offset correction
    en_offset_correction = en;
}

void AXS15231B_Touch::setOffsets(uint16_t x_real_min, uint16_t x_real_max, uint16_t x_ideal_max, uint16_t y_real_min, uint16_t y_real_max, uint16_t y_ideal_max) {
    // Offsets used for offset correction if enabled
    // Offsets should be determinded with rotation = 0
    this->x_real_min = x_real_min;
    this->x_real_max = x_real_max;
    this->y_real_min = y_real_min;
    this->y_real_max = y_real_max;
    this->x_ideal_max = x_ideal_max;
    this->y_ideal_max = y_ideal_max;
}

void AXS15231B_Touch::correctOffset(uint16_t *x, uint16_t *y) {
    // Map values to correct for offset
    *x = map(*x, x_real_min, x_real_max, 0, x_ideal_max);
    *y = map(*y, y_real_min, y_real_max, 0, y_ideal_max);
}

bool AXS15231B_Touch::update() {
    // Check if interrupt occured, if there was an interrupt get data from touch controller and clear flag
    if (!touch_int) {
        return false;
    } else {
        touch_int = false;
    }

    uint8_t tmp_buf[8] = {0};
    // Command to read data from controller
    // This commands configures the controller to read only the position of the first finger
    static const uint8_t read_touchpad_cmd[8] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08};

    // Send command to controller
    Wire.beginTransmission(addr);
    Wire.write(read_touchpad_cmd, sizeof(read_touchpad_cmd));
    Wire.endTransmission();

    // Read response from controller
    Wire.requestFrom(addr, sizeof(tmp_buf));
    for(int i = 0; i < sizeof(tmp_buf) && Wire.available(); i++) {
        tmp_buf[i] = Wire.read();
    }

    // Extract X and Y coordinates from response
    uint16_t raw_X = AXS_GET_POINT_X(tmp_buf);
    uint16_t raw_Y = AXS_GET_POINT_Y(tmp_buf);

    // Validate data
    if (point_X || point_Y) {
        if (raw_X > x_real_max) raw_X = x_real_max;
        if (raw_X < x_real_min) raw_X = x_real_min;
        if (raw_Y > y_real_max) raw_Y = y_real_max;
        if (raw_Y < y_real_min) raw_Y = y_real_min;
    }

    // Correct offset if enabled
    uint16_t x_max, y_max;
    if (en_offset_correction) {
        correctOffset(&raw_X, &raw_Y);
        x_max = x_ideal_max;
        y_max = y_ideal_max;
    } else {
        x_max = x_real_max;
        y_max = y_real_max;
    }

    // Align X and Y according to rotation
    switch (rotation) {
        case 0:
            point_X = raw_X;
            point_Y = raw_Y;
            break;
        case 1:
            point_X = raw_Y;
            point_Y = x_max - raw_X;
            break;
        case 2:
            point_X = x_max - raw_X;
            point_Y = y_max - raw_Y;
            break;
        case 3:
            point_X = y_max - raw_Y;
            point_Y = raw_X;
            break;
        default:
            break;
    }

    return true;
}
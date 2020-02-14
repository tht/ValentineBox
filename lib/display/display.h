#include <Arduino.h>

// Display modes
enum DD {
    DD_BOOT,
    DD_DAYS_SINCE
};

// Function definitions
void show_display(enum DD mode, uint16_t days = 0);
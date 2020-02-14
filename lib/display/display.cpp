#include "display.h"
/*******************************************************************************************
    Pinbelegung:
        Display:      ESP32 Lolin32:
        BUSY          4
        RST           16
        DC            17
        CS            5   (SS)
        CLK           18  (SCK)
        DIN           23  (MOSI)
        GND           GND
        3.3V          3V
*/

// Include libraries for ePaper Display
#include <GxEPD.h>
#include <GxGDEP015OC1/GxGDEP015OC1.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

// Include Fonts and Images
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include "img_heart.h"
#include "img_boot.h"

// Configure display interface
GxIO_Class io(SPI, SS, 17, 16);  //SPI,SS,DC,RST
GxEPD_Class display(io, 16, 4);  //io,RST,BUSY

/**
 * draw_display_boot()
 * Draws the full-screen (200x200px) boot image from img_boot
 */
void draw_display_boot() {
    display.drawBitmap(img_boot, 0, 0, 200 , 200, GxEPD_BLACK);
}


/**
 * draw_display_days(days)
 * Draws some text and the number of days since a specific date
 * on the screen.
 */
void draw_display_days(uint16_t days) {
    // Clear display and set colors
    display.setTextColor(GxEPD_BLACK);
    display.fillRect(0, 0, 200, 200, GxEPD_WHITE);

    // Draw first line
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(0, 25);
    display.print("Ich liebe dich");

    // Draw two hearts from bitmaps
    display.drawBitmap(img_heart, 15, 42, 32 , 29, GxEPD_BLACK);
    display.drawBitmap(img_heart, 155, 50, 32 , 29, GxEPD_BLACK);

    // Draw the number of days using a bigger font
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor((days >= 1000 ? 45 : 57), 110);
    display.print(days);

    // back to smaller font, write lines below number
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(28, 137);
    display.print("tolle Tage");
    display.setCursor(13, 162);
    display.print("seit unserem");
    display.setCursor(21, 187);
    display.print("ersten Kuss");
}


/**
 * show_display(mode, days)
 * Updates the eInk display to show screen selected by `mode`.
 * Some screens need the number of `days` since an event.
 */
void show_display(enum DD mode, uint16_t days) {
    Serial.println("Updating display...");

    // Init display if not yet done
    static bool first_run = true;
    if (first_run) {
        first_run = false;
        display.init();
        display.fillScreen(GxEPD_WHITE);  // Display Weiss füllen
        display.setRotation(1);
        display.update();
    }

    // Draw to display according to choosen mode
    switch (mode){
        case (DD_BOOT): draw_display_boot(); break;
        case (DD_DAYS_SINCE): draw_display_days(days); break;
    }

    // Finally update area
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
}
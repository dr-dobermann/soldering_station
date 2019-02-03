#include <TFT.h>  // Arduino LCD library
#include <SPI.h>

const uint8_t 
    PBTN = 2, 
    PENCA = 3, 
    PENCB = 4,
    PDC  = 5,
    PCS  = 7,
    PRST = 6;

int value = -1;
int incr  = 1;


// create an instance of the library
TFT TFTscreen = TFT(PCS, PDC, PRST);
char encdrPrintout[5];

void setNewValue(int new_val) {
    if (new_val == value)
        return;
        
    String(value).toCharArray(encdrPrintout, 5);
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(encdrPrintout, 0, 20);
    value = new_val;
    String(value).toCharArray(encdrPrintout, 5);
    TFTscreen.stroke(255, 255, 0);
    TFTscreen.text(encdrPrintout, 0, 20);
    Serial.println(value);
}

void setup() {
    TFTscreen.begin();
    TFTscreen.setRotation(1);
    // clear the screen with a black background
    TFTscreen.background(0, 0, 0);
    // write the static text to the screen
    // set the font color to white
    TFTscreen.stroke(255, 0, 255);
    // set the font size
    TFTscreen.setTextSize(2);
    // write the text to the top left corner of the screen
    TFTscreen.text("Encdr Value :\n ", 0, 0);
    // ste the font size very large for the loop
    TFTscreen.setTextSize(4);



    Serial.begin(9600);
    setNewValue(0);
}

void loop() {


    if (btn_on_time > 0 && millis() - btn_on_time > LONG_PRESS) {
        btn_on_time = -1;
        incr = 1;
        setNewValue(0);
    }

    if (btn_pos != new_btn && millis() - btn_change_time > DEBOUNCE_TIME) {
        btn_pos = new_btn;
        if (btn_pos == 0)
            switch (incr) {
                case 1:
                    incr = 5;
                    break;
                case 5:
                    incr = 10;
                    break;
                case 10:
                    incr = 1;
                    break;
            }
        btn_on_time = btn_pos == 0 ? millis() : -1;
        
        btn_change_time = millis();
    }
    
    if ( new_epos != epos ) {
        if (edir == -1) {
            if (estate[0][0] == new_epos) {
                edir = 0;
                eval = 1;
            }
            else 
                if (estate[1][0] == new_epos) {
                    edir = 1;
                    eval = 1;
                }
        }
        else 
            if (estate[edir][eval] == new_epos) 
                eval++;
            else {
                eval = 0;
                edir = -1;
            }

        epos = new_epos;
    }

    if ( new_epos == 3 ) {
        if ( eval == 4 ) {
            int new_val = value + incr * (edir ? 1 : -1);
            if (new_val > 999)
                new_val = 999;
            if (new_val < -999)
                new_val = -999;
            setNewValue(new_val);
        }
        eval = 0;
        edir = -1;
    }
}

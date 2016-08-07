/**************************************************************************
 *                                                                     	*
 *   NeoPixel Ring Clock                                               	*
 *   by Andy Doro (mail@andydoro.com)                                 	*
 *   http://andydoro.com/ringclock/                                   	*
 *                                                                      *
 **************************************************************************
 *
 *
 * Revision History
 *
 * Date 	 By    What
 * 20140320 	AFD 	First draft
 * 20160105   AFD   faded arcs
 */

// include the library code:
#include <Wire.h>
#include "RTClib.h"
#include "Adafruit_NeoPixel.h"

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

// define pins
#define NEOPIN 6
#define BUTTON1 10
#define BUTTON2 11

#define BRIGHTNESS 64 // set max brightness

/****************************************************************************
 * New Button Class
 */
class Button {
  public:
    Button( int myPin = 2 ) {
      pin = myPin;
      pinMode( pin, INPUT_PULLUP );
      currentVal = false;
      lastVal = false;
    }

    boolean togglePush() {
      if ( readButton() == true && lastVal == false ) {
        return true;
      }
      if ( readButton() == false && lastVal == true ) {
        return false;
      }
      return false;
    }

    boolean isPushed() {
      if ( readButton() == true && lastVal == false ) {
        return true;
      }
      return false;
    }

    boolean isUnPushed() {
      if ( readButton() == false && lastVal == true ) {
        return true;
      }
      return false;
    }

    boolean isOn() {
      if ( readButton() == true ) {
        return true;
      }
      return false;
    }

    boolean isOff() {
      if ( readButton() == false ) {
        return true;
      }
      return false;
    }

  private:
    int pin;
    boolean currentVal;
    boolean lastVal;
    
    boolean readButton() {
      lastVal = currentVal;
      int sensorVal = digitalRead( pin );
      if( sensorVal == HIGH ) {
        currentVal = false;
      } else {
        currentVal = true;
      }
      return currentVal;
    }
};
//****************************************************************************


RTC_DS1307 RTC; // Establish clock object
DateTime Clock; // Holds current clock time

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, NEOPIN, NEO_GRB + NEO_KHZ800); // strip object

byte yearval, monthval, dayval;     // holds the date
byte hourval, minuteval, secondval; // holds the time

byte pixelColorRed, pixelColorGreen, pixelColorBlue; // holds color values

int selectMode = 0;

Button button1 = Button( BUTTON1 );
Button button2 = Button( BUTTON2 );


void setup () {
  //Serial.begin(9600);
  Serial.begin(57600);
  Serial.println("START: setup");

  Wire.begin();  // Begin I2C
  RTC.begin();   // begin clock

  // set pinmodes
  pinMode(NEOPIN, OUTPUT);

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  strip.begin();
  //strip.show(); // Initialize all pixels to 'off'

  strip.setBrightness(BRIGHTNESS); // set brightness

  // startup sequence
  delay(500);
  colorWipe(strip.Color(255, 0, 0), 20); // Red
  colorWipe(strip.Color(0, 255, 0), 20); // Green
  colorWipe(strip.Color(0, 0, 255), 20); // Blue
  delay(500);
  Serial.println("END: setup");
}

void loop () {

  // get time
  Clock = RTC.now(); // get the RTC time
  boolean blink = false;

  yearval   = Clock.year();    // get year
  monthval  = Clock.month();   // get month
  dayval    = Clock.day();     // get day
  secondval = Clock.second();  // get seconds
  minuteval = Clock.minute();  // get minutes
  hourval   = Clock.hour();    // get hours
  if (hourval > 11) hourval -= 12; // This clock is 12 hour, if 13-23, convert to 0-11

  hourval = (hourval * 60 + minuteval) / 12; //each red dot represent 24 minutes.

  // arc mode
  for (uint8_t i = 0; i < strip.numPixels(); i++) {

    if (tailBand( i, secondval, 15.0 )) {
      // calculates a faded arc from low to maximum brightness
      //pixelColorBlue = (i + 1) * (255 / (secondval + 1));
      if( selectMode = 1 && blink ) {
        pixelColorBlue = 0;
      } else {
        pixelColorBlue = 255;
      }
      pixelColorBlue = (i + 1) * (255 / (secondval + 1));
      //pixelColorBlue = 255;
    }
    else {
      pixelColorBlue = 0;
    }

    if (deadBand( i, minuteval, 0.9 )) {
      pixelColorGreen = 255;
      pixelColorGreen = (i + 1) * (255 / (minuteval + 1));
      //pixelColorGreen = 255;
    }
    else {
      pixelColorGreen = 0;
    }

    if (deadBand( i, hourval, 2.5 )) {
      pixelColorRed = 255;
      pixelColorRed = (i + 1) * (255 / (hourval + 1));
      //pixelColorRed = 255;
    }
    else {
      pixelColorRed = 0;
    }

    strip.setPixelColor(i, strip.Color(pixelColorRed, pixelColorGreen, pixelColorBlue));
  }

  /*
  // for serial debugging
  
   Serial.print(hourval, DEC);
   Serial.print(':');
   Serial.print(minuteval, DEC);
   Serial.print(':');
   Serial.println(secondval, DEC);
   /* */

  //display
  strip.show();

  // wait
  delay(100);
  blink = ! blink;
  if( button1.togglePush() ) {
    selectMode = cycleMode( selectMode );
  }
  if( button2.togglePush() ) {
     if( selectMode == 1 ) {
        hourval   = Clock.hour() + 1;
     }
     if( selectMode == 2 ) {
        minuteval   = Clock.minute() + 1;
     }
     if( selectMode == 3 ) {
        secondval   = Clock.second() + 1;
     }
     RTC.adjust(DateTime( yearval, monthval, dayval, hourval, minuteval, secondval ) );
     if (hourval > 11) hourval -= 12;           // This clock is 12 hour, if 13-23, convert to 0-11
     hourval = (hourval * 60 + minuteval) / 12; //each red dot represent 24 minutes.
  }
}

// Select setup mode
int cycleMode( int selectMode ) {
  selectMode = selectMode + 1;
  if ( selectMode > 3 ) { selectMode = 0; }
  return selectMode;
}
// Boolean test for clock hand width
boolean deadBand( uint8_t countVal, byte actualVal, float deadBand ) {
     if( ( countVal >  ( actualVal - deadBand) ) and ( countVal < ( actualVal + deadBand ) ) ) {
       return true;
     }
     if( actualVal - deadBand < 0 ) {
       if( ( countVal >  ( 60 + actualVal - deadBand) ) and ( countVal < actualVal ) ) {
         return true;
       }
     }
     if( actualVal + deadBand > 60 ) {
       if( ( countVal >  ( actualVal - deadBand) ) and ( countVal < actualVal + deadBand - 60 ) ) {
         return true;
       }
     }
     return false;
}

// Boolean test for clock hand width
boolean tailBand( uint8_t countVal, byte actualVal, float deadBand ) {
     if( ( countVal >  ( actualVal - deadBand) ) and ( countVal < actualVal ) ) {
       return true;
     }
     if( actualVal - deadBand < 0 ) {
       if( countVal >  ( 60 + actualVal - deadBand) ) {
         return true;
       }
     }
     return false;
}
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}



/*
This is a sketch for Arduinos using the Atmega328P controller - the Uno,
the Nano or the Pro Mini.  It works for a Magic Chef model HNN1110W
microwave oven purchased in 2020.

The code monitors signals being sent to the oven's display, and electronically
presses the Stop key twice when the countdown reaches one second left.  This
eliminates the five annoying beeps that the oven produces - even after opening
the door - while preserving the basic beep function on a keypress.

The digit lines are connected to D2 - D5.  Four of the seven segment lines are
connected to A0 - A3 after voltage translation.  The Stop key press is controlled
from D6.
*/

byte digitVal[8];                         // 4 digits, 2 cycles (ABCD,EFG:)
byte cycle = 0;                           // zero or 4
byte digit = 0;                           // digit currently being driven
bool matchFlag;                           // first of two required matches
byte i, digitPins, oldPins, lastValid;
byte pattern[4] = {0, 7, 7, 2};           // values for ' 001'. Only ABDG tested
byte digID[16] ={0, 0, 0, 0, 0, 0, 0, 4,  // value of the 4 digit lines as an index
                 0, 0, 0, 3, 0, 2, 1, 0}; //   identifies the digit being driven (1-4)

void setup() {
  for (i = 0; i < 4; i++) {
    pinMode((2 + i), INPUT);              // D2 - D5
    pinMode((14 + i), INPUT_PULLUP);      // A0 - A3
  }
  digitalWrite(6, LOW);                   // mosfet gate drive
  pinMode(6, OUTPUT);
  TIMSK0 = 0;                             // turn off millis() interrupt
}

void loop() {
  digitPins = PIND & 0x3C;                // digit pins on D2 - D5
  if (digitPins != oldPins) {             // have they changed?
    digit = digID[(digitPins >> 2)];      // the digit being driven, if any
    if (digit) {                          // one digit is being driven
      digit--;                            // shift to zero-based digit(0-3)
      if (digit != lastValid) {           // different from last digit
        lastValid = digit;
        digitVal[digit + cycle] = PINC & 0xF; // read segments for this digit
                                              // A0-A3 = segments ABDG
        if (digit == 3) {                 // end of a cycle
          cycle ^= 4;                     // switch between ABCD and EFG:
          if (cycle == 0) {               // end of EFG: cycle?
            if (testOne()) {              // does the pattern match ' 001'
              if (matchFlag) {            // two successive matches required
                Stopit();                 // if second match, shut it down
                matchFlag = false;
              }
              else matchFlag = true;      // if first match, just set flag
            }                             // did pattern match?
            else matchFlag = false;       // if match fails, reset flag
          }                               // end of EFG: cycle
        }                                 // end of a cycle
      }                                   // was digit same as last valid
    }                                     // was change to a valid digit
    oldPins = digitPins;
  }                                       // any change in digit lines
}

bool testOne() {                          // i 0-3 = ABCD cycle, 4-7 = EFG: cycle
  bool Match = true;
  for (i = 0; i < 4; i++) {
    if ((digitVal[i] | digitVal[i + 4]) != pattern[i]) Match = false;
  }
  return Match;
}

void Stopit () {
  TIMSK0 = 1;                             // turn millis() interrupt on
  delay(800);
  digitalWrite(6, HIGH);                  // turn on mosfet
  delay(100);
  digitalWrite(6, LOW);
  delay(300);
  digitalWrite(6, HIGH);
  delay(100);
  digitalWrite(6, LOW);
  delay(500);
  TIMSK0 = 0;                              // turn millis() interrupt off
}

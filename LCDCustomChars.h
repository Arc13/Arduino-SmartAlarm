#ifndef LCDCUSTCHARS_H
#define LCDCUSTCHARS_H

enum customChars {
  CHAR_DEGREECELSIUS = 0,
  CHAR_DEGREEFAHRENHEIT = 1,
};

byte char_degreeCelsius[8] = {
  B11000,
  B11000,
  B00000,
  B00000,
  B00011,
  B00100,
  B00100,
  B00011,
};

byte char_degreeFahrenheit[8] = {
  B11000,
  B11000,
  B00000,
  B00000,
  B00111,
  B00100,
  B00110,
  B00100,
};

#endif

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include "Arduino.h"

class ButtonController {
  public:
    // Buttons enum
    enum Buttons {
      BUTTON_NONE = 0,

      BUTTON_UP = 1,
      BUTTON_DOWN,
      BUTTON_LEFT,
      BUTTON_RIGHT,

      BUTTON_MAIN,
    };
    
    ButtonController(int buttonValue);

    void update(int buttonValue);

    Buttons getLastPressedButton();
    Buttons getHeldButton();

  private:
    int m_buttonValue;
    int m_buttonValueOld;

    unsigned long m_timePressedButton;
    Buttons m_pressedButton;
    Buttons m_heldButton;
    Buttons m_lastPressedButton;
};

#endif

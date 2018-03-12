#include "ButtonController.h"

/*
  SELECT=639
  LEFT=407
  DOWN=256
  UP=99
  RIGHT=0
*/

// ButtonController constructor
ButtonController::ButtonController(int buttonValue)
  : m_buttonValue(buttonValue)
  , m_buttonValueOld(buttonValue)
  , m_pressedButton(BUTTON_NONE)
  , m_heldButton(BUTTON_NONE)
  , m_lastPressedButton(BUTTON_NONE)
  , m_timePressedButton(millis())
{

}

// Update the buttons state
void ButtonController::update(int buttonValue) {
  // Check if the actually pressed button was pressed for at least 500ms, to flag it as held
  if (m_pressedButton != BUTTON_NONE) {
    if (millis() >= m_timePressedButton + 500) {
      m_heldButton = m_pressedButton;
    }
  }

  // To check a new button, no button must be pressed
  if (m_buttonValue < 1000 && buttonValue < 1000)
    return;

  // m_buttonValue holds the A0 voltage
  m_buttonValue = buttonValue;

  // Check if the button isn't the same since the last time and that 30ms passed since the last check
  if ((m_buttonValue <= m_buttonValueOld - 300 || m_buttonValue >= m_buttonValueOld + 300) && millis() >= m_timePressedButton + 30) {
    // Transform the voltage into a Buttons var
    if (m_buttonValue >= 0 && m_buttonValue < 20)
      m_pressedButton = BUTTON_RIGHT;
    else if (m_buttonValue >= 80 && m_buttonValue < 120)
      m_pressedButton = BUTTON_UP;
    else if (m_buttonValue >= 230 && m_buttonValue < 280)
      m_pressedButton = BUTTON_DOWN;
    else if (m_buttonValue >= 380 && m_buttonValue < 420)
      m_pressedButton = BUTTON_LEFT;
    else if (m_buttonValue >= 620 && m_buttonValue < 660)
      m_pressedButton = BUTTON_MAIN;
    else
      m_pressedButton = BUTTON_NONE;

    // Reset vars
    m_timePressedButton = millis();
    m_heldButton = BUTTON_NONE;
    m_lastPressedButton = m_pressedButton;
  }

  // Store the actual value for later check
  m_buttonValueOld = m_buttonValue;
}

// getLastPressedButton() is a one-shot function, it reset the var when called
ButtonController::Buttons ButtonController::getLastPressedButton() {
  Buttons lastPressedButton = m_lastPressedButton;
  m_lastPressedButton = BUTTON_NONE;

  return lastPressedButton;
}

// Get if a button is being hold
ButtonController::Buttons ButtonController::getHeldButton() {
  return m_heldButton;
}


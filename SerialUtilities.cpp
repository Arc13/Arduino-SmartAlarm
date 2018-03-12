#include "SerialUtilities.h"

SerialUtils::SerialUtils()
: m_isAvailable(0)
{
  
}

void SerialUtils::update() {
  if (m_isAvailable)
    return;
    
  if (Serial.available()) {
    for (int i = 0; i < Serial.available(); i++) {
      char currentChar = Serial.read();

      if (currentChar == '\n')
        m_isAvailable += 1;
   
      m_actualMessage += currentChar;
    }
  }
}

String SerialUtils::receive() {
  if (m_isAvailable == 0)
    return;

  String message;
  m_isAvailable -= 1;

  message = m_actualMessage.substring(0, m_actualMessage.indexOf('\n') + 1);
  m_actualMessage = m_actualMessage.substring(m_actualMessage.indexOf('\n') + 1, m_actualMessage.length());
  
  return message;
}

int SerialUtils::available() {
  return m_isAvailable;
}

void SerialUtils::begin(long baudrate) {
  m_communicationSpeed = baudrate;
  Serial.begin(baudrate);
}


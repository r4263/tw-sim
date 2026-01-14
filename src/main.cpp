#include <Arduino.h>

#define OUTPUT_PIN 42

struct ToothWheelConfig
{
  const char *name;
  int teethCount;
  int missingTeeth;
};

enum CommandType {
  CMD_SET_RPM,
  CMD_SET_WHEEL,
  CMD_ENABLE,
  CMD_DISABLE,
  CMD_INVALID
};

const ToothWheelConfig wheels[] = {
    {"36-1", 36, 1},
    {"60-2", 60, 2},
    {"24-1", 24, 1},
    {"12-2", 12, 2},
    {"12-0", 12, 0}};

const int wheelCount = sizeof(wheels) / sizeof(wheels[0]);

// Global variables
volatile bool signalEnabled = false;
volatile int currentRPM = 1000;
volatile int selectedWheel = 0;
volatile int currentTooth = 0;
volatile bool highState = false;
volatile int intervalMicros = 0;

hw_timer_t *timer = NULL;

inline void IRAM_ATTR setPinHigh(int pin)
{
  if (pin < 32)
    GPIO.out_w1ts = (1 << pin);
  else
    GPIO.out1_w1ts.val = (1 << (pin - 32));
}

inline void IRAM_ATTR setPinLow(int pin)
{
  if (pin < 32)
    GPIO.out_w1tc = (1 << pin);
  else
    GPIO.out1_w1tc.val = (1 << (pin - 32));
}

int calculatePulseInterval(const ToothWheelConfig &wheel)
{
  int totalTime = (60 * 1000000) / currentRPM;

  return totalTime / wheel.teethCount;
}

void IRAM_ATTR generateSignal()
{
  if (!signalEnabled)
  {
    setPinLow(OUTPUT_PIN);
    return;
  }

  const ToothWheelConfig &wheel = wheels[selectedWheel];

  int presentTeeth = wheel.teethCount - wheel.missingTeeth;
  bool isPresentTooth = (currentTooth < presentTeeth);

  if (isPresentTooth)
  {
    highState = !highState;
    if (highState)
      setPinHigh(OUTPUT_PIN);
    else
      setPinLow(OUTPUT_PIN);
  }
  else
  {
    setPinLow(OUTPUT_PIN);
  }

  if (!highState)
  {
    currentTooth++;
    if (currentTooth >= wheel.teethCount)
    {
      currentTooth = 0;
    }
  }

  int pulseInterval = calculatePulseInterval(wheel);
  intervalMicros = isPresentTooth ? pulseInterval / 2 : pulseInterval;
  timerAlarmWrite(timer, intervalMicros, true);
}

void setup()
{
  Serial.begin(115200);

  pinMode(OUTPUT_PIN, OUTPUT);
  setPinLow(OUTPUT_PIN);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &generateSignal, true);
  timerAlarmWrite(timer, 500, true);
  timerAlarmEnable(timer);

  Serial.println("Trigger wheel signal generator/simulator.");
  Serial.println("Available commands:");
  Serial.println("set rpm <value> - Adjusts RPM");
  Serial.println("set wheel <index> - Changes the trigger wheel pattern");
  Serial.println("enable - Enables the output signal");
  Serial.println("disable - Disables the output signal");
}

CommandType parseCommand(const String &command) {
  if (command.startsWith("set rpm ")) {
    return CMD_SET_RPM;
  }
  if (command.startsWith("set wheel ")) {
    return CMD_SET_WHEEL;
  }
  if (command == "enable") {
    return CMD_ENABLE;
  }
  if (command == "disable") {
    return CMD_DISABLE;
  }
  return CMD_INVALID;
}

void handleSerialInput()
{
  if (!Serial.available()) return;

  String command = Serial.readStringUntil('\n');
  command.trim();

  switch (parseCommand(command))
  {
    case CMD_SET_RPM:
      currentRPM = command.substring(8).toInt();
      Serial.printf("RPM adjusted to: %d\n", currentRPM);
      break;

    case CMD_SET_WHEEL: {
      int index = command.substring(10).toInt();
      if (index >= 0 && index < wheelCount)
      {
        selectedWheel = index;
        Serial.printf(
          "Trigger wheel pattern set to: %s (Tooth: %d, Absent tooth: %d)\n",
          wheels[selectedWheel].name,
          wheels[selectedWheel].teethCount,
          wheels[selectedWheel].missingTeeth
        );
      }
      else
      {
        Serial.println("Trigger wheel pattern invalid.");
      }
      break;
    }

    case CMD_ENABLE:
      signalEnabled = true;
      Serial.println("Output signal enabled.");
      break;

    case CMD_DISABLE:
      signalEnabled = false;
      Serial.println("Output signal disabled.");
      break;

    default:
      Serial.println("Invalid command. Available:");
      Serial.println("set rpm <value> - Adjusts RPM");
      Serial.println("set wheel <index> - Changes the trigger wheel pattern");
      Serial.println("enable - Enables the output signal");
      Serial.println("disable - Disables the output signal");
      break;
  }
}

void loop()
{
  handleSerialInput();
}

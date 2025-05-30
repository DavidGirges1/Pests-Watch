// Pin Definitions
const int PIR_PIN_1 = 2;
const int PIR_PIN_2 = 3;
const int MOISTURE_SENSOR_PIN = A0;
const int THERMISTOR_PIN = A1;
const int RELAY_PIN_1 = 4;
const int RELAY_PIN_2 = 5;

// Sensor & System Parameters
const float MOISTURE_THRESHOLD_PERCENT = 80.0;
const float TEMP_LOW_THRESHOLD_C = 15.0;
const float TEMP_HIGH_THRESHOLD_C = 20.0;
const float NOMINAL_RESISTANCE = 10000.0;
const float NOMINAL_TEMPERATURE_C = 25.0;
const float BETA_COEFFICIENT = 3950.0;
const float SERIES_RESISTOR = 10000.0;
const unsigned long SPRAY_DURATION_MS = 10000;
const unsigned long COOLDOWN_PERIOD_MS = 3UL * 60UL * 1000UL;

unsigned long lastSprayTime = 0;
bool systemInCooldown = false;

// Setup Function: Initializes hardware and serial communication
void setup()
{
  Serial.begin(9600);
  Serial.println("System Initializing...");

  pinMode(PIR_PIN_1, INPUT);
  pinMode(PIR_PIN_2, INPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);

  digitalWrite(RELAY_PIN_1, HIGH);
  digitalWrite(RELAY_PIN_2, HIGH);

  Serial.println("System Ready.");
  Serial.print("Cooldown period (testing/production): ");
  Serial.print(COOLDOWN_PERIOD_MS / 1000 / 60);
  Serial.println(" minutes / 3 hours.");
}

// Main Loop: Continuously reads sensors and controls sprayers
void loop()
{
  unsigned long currentTime = millis();

  if (systemInCooldown && (currentTime - lastSprayTime >= COOLDOWN_PERIOD_MS))
  {
    systemInCooldown = false;
    Serial.println("Cooldown period ended. System active.");
  }

  bool motionDetected1 = digitalRead(PIR_PIN_1) == HIGH;
  bool motionDetected2 = digitalRead(PIR_PIN_2) == HIGH;
  float moisturePercent = readMoisturePercentage();
  float temperatureC = readTemperatureCelsius();

  Serial.print("PIR1: ");
  Serial.print(motionDetected1);
  Serial.print(" | PIR2: ");
  Serial.print(motionDetected2);
  Serial.print(" | Moisture: ");
  Serial.print(moisturePercent, 1);
  Serial.print("%");
  Serial.print(" | Temp: ");
  Serial.print(temperatureC, 1);
  Serial.println(" C");

  bool triggerSpray = false;

  if (!systemInCooldown)
  {
    if (motionDetected1 || motionDetected2)
    {
      Serial.println("Pest motion detected!");
      triggerSpray = true;
    }

    if (moisturePercent > MOISTURE_THRESHOLD_PERCENT &&
        temperatureC > TEMP_LOW_THRESHOLD_C &&
        temperatureC < TEMP_HIGH_THRESHOLD_C)
    {
      Serial.println("Fungal growth conditions detected!");
      triggerSpray = true;
    }

    if (triggerSpray)
    {
      activateSprayers();
      lastSprayTime = currentTime;
      systemInCooldown = true;
      Serial.println("Spraying initiated. Cooldown started.");
    }
  }
  else
  {
    Serial.println("System in cooldown...");
  }

  delay(1000);
}

// Helper Function: Reads and calculates soil moisture percentage
float readMoisturePercentage()
{
  int rawValue = analogRead(MOISTURE_SENSOR_PIN);
  float percent = 100.0 - ((float)rawValue * 100.0 / 1023.0);
  return constrain(percent, 0.0, 100.0);
}

// Helper Function: Reads NTC thermistor and calculates temperature in Celsius
float readTemperatureCelsius()
{
  int adcValue = analogRead(THERMISTOR_PIN);

  if (adcValue == 0 || adcValue >= 1023)
  {
    Serial.println("Warning: Thermistor reading out of range!");
    return -100.0;
  }

  float R_thermistor = SERIES_RESISTOR * (1023.0 / (float)adcValue - 1.0);
  if (R_thermistor <= 0)
  {
    Serial.println("Warning: Calculated thermistor resistance invalid!");
    return -102.0;
  }

  float steinhart;
  steinhart = R_thermistor / NOMINAL_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= BETA_COEFFICIENT;
  steinhart += 1.0 / (NOMINAL_TEMPERATURE_C + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;

  return steinhart;
}

// Helper Function: Activates and deactivates spray pumps
void activateSprayers()
{
  Serial.println("Turning pumps ON...");
  digitalWrite(RELAY_PIN_1, LOW);
  digitalWrite(RELAY_PIN_2, LOW);
  delay(SPRAY_DURATION_MS);
  digitalWrite(RELAY_PIN_1, HIGH);
  digitalWrite(RELAY_PIN_2, HIGH);
  Serial.println("Pumps OFF.");
}

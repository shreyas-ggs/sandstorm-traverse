// Task 4 - Sandstorm Traverse (very simple version)
// Potentiometer on pin 34 = motor current (0 V = 0 A, 3.3 V = 30 A)
// Serial keys: w = forward, s = stop, a = turn left, d = turn right

const int POT = 34;
const int LEFT_PIN = 25;    // left motor (green LED)
const int RIGHT_PIN = 26;   // right motor (green LED)
const int RED = 2;          // red LED = stall recovery

float v = 0.6;              // forward speed
float w = 0.0;              // turning rate
float amps = 0, leftSpeed = 0, rightSpeed = 0;

bool counting = false;      // current is above 20 A, timing it
bool reversing = false;     // doing the 200 ms backwards pulse
bool limp = false;          // running at 20% power
unsigned long overStart = 0, reverseStart = 0, lastFast = 0, lastSlow = 0;
int ticks = 0;

void setup() {
  Serial.begin(115200);
  pinMode(RED, OUTPUT);
  Serial.println("Keys: w=forward s=stop a=left d=right");
}

void loop() {
  unsigned long now = millis();

  // FAST LOOP - every 10 ms (100 times a second)
  if (now - lastFast >= 10) {
    lastFast = now;
    amps = analogRead(POT) / 4095.0 * 30.0;

    // steering math (wheel width = 0.5)
    leftSpeed  = v - (w * 0.5 / 2);
    rightSpeed = v + (w * 0.5 / 2);

    if (reversing) {
      leftSpeed = -0.2;                 // go backwards for 200 ms
      rightSpeed = -0.2;
      if (now - reverseStart >= 200) reversing = false;
    } else {
      if (amps > 20) {                  // too much current
        if (!counting) {
          counting = true;
          overStart = now;
        } else if (now - overStart > 350) {   // for more than 350 ms = STALL
          counting = false;
          limp = true;                  // power drops to 20%
          reversing = true;             // start backwards pulse
          reverseStart = now;
          Serial.println("STALL ACTIVE");
        }
      } else {
        counting = false;
      }

      if (amps < 10) limp = false;      // safe again, back to full power

      if (limp) {
        leftSpeed *= 0.2;
        rightSpeed *= 0.2;
      }
    }

    analogWrite(LEFT_PIN, abs(leftSpeed) * 255);
    analogWrite(RIGHT_PIN, abs(rightSpeed) * 255);
    digitalWrite(RED, limp);
  }

  // SLOW LOOP - every 50 ms (20 times a second)
  if (now - lastSlow >= 50) {
    lastSlow = now;

    while (Serial.available()) {        // read keys, never waits
      char c = Serial.read();
      if (c == 'w') { v = 0.6; w = 0; }
      if (c == 's') { v = 0;   w = 0; }
      if (c == 'a') { w = -1; }
      if (c == 'd') { w = 1; }
    }

    if (++ticks >= 10) {                // report twice a second
      ticks = 0;
      Serial.print("Current: ");
      Serial.print(amps);
      Serial.print(" A  Left: ");
      Serial.print(leftSpeed);
      Serial.print("  Right: ");
      Serial.println(rightSpeed);
    }
  }
}

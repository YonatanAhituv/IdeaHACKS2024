int flashPin = 4;

void setup() {
    pinMode(flashPin, OUTPUT);
}

void loop() {
    digitalWrite(flashPin, HIGH);
    delay(200);
    digitalWrite(flashPin, LOW);
    delay(200);
}
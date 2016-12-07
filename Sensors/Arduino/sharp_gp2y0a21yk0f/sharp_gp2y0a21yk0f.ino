void setup() {
    Serial.begin(9600);
}

int read_average(int pin) {
    long ans = 0;
    int i;
    for (i = 0; i < 100; i++) {
        ans = ans + analogRead(pin);
    }
    return ans / 100;
}

int atoc(int v) {
    if (v >= 340) return 10;
    if (v >= 200) return 20;
    if (v >= 145) return 30;
    if (v >= 126) return 40;
    if (v >= 105) return 50;
    if (v >= 85)  return 60;
    if (v >= 65)  return 70;
    if (v >= 55)  return 80;
    if (v >= 45)  return 90;
    return 100;
}

void loop() {
    int v;
    v = read_average(0);
    Serial.println(atoc(v));
    delay(500);
}
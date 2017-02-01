long error = 0;
int fps = 40;
long idealSleep = (1000L << 16) / fps;
long oldTime, newTime;

void setup() {
    Serial.begin(9600);
    newTime = millis() << 16;
}

int read_average(int pin) {
    long ans = 0;
    int i;
    for (i = 0; i < 10; i++) {
        ans = ans + analogRead(pin);
    }
    return ans / 10;
}

void loop() {
    int v;
    oldTime = newTime;

    v = read_average(0);
    Serial.println(v);

    newTime = millis() << 16;
    long sleepTime = idealSleep - (newTime - oldTime) - error;
    if (sleepTime < (2 << 16)) {
      sleepTime = 2 << 16;
    }
    oldTime = newTime;
    delay(sleepTime >> 16);
    newTime = millis() >> 16;
    error = newTime - oldTime - sleepTime;
}
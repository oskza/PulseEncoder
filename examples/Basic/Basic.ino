#include <Arduino.h>
#include <PulseEncoder.h>

PulseEncoder encoder(32);
bool ready = false;
uint32_t previousCount = 0;

void setup() {
    Serial.begin(115200);
    ready = encoder.begin();
    if (!ready)
        Serial.println("Encoder setup failed");
}

void loop() {
    if (!ready)
        return;
    const uint32_t count = encoder.getCount();
    if (count != previousCount) {
        previousCount = count;
        Serial.println(count);
    }
}

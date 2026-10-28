#ifndef PULSE_ENCODER_H
#define PULSE_ENCODER_H
#include <Arduino.h>

class PulseEncoder {
public:
    enum class InputMode : uint8_t {
        FLOATING,
        PULL_UP,
        PULL_DOWN
    };
    enum class Edge : uint8_t {
        FALLING_EDGE,
        RISING_EDGE,
        ANY_EDGE
    };
    explicit PulseEncoder(
        uint8_t pin,
        InputMode inputMode = InputMode::PULL_UP,
        Edge edge = Edge::FALLING_EDGE
    );
    PulseEncoder(const PulseEncoder &) = delete;
    PulseEncoder &operator=(const PulseEncoder &) = delete;
    PulseEncoder(PulseEncoder &&) = delete;
    PulseEncoder &operator=(PulseEncoder &&) = delete;
    bool begin();
    uint32_t getCount() const;
private:
    uint8_t _pin;
    InputMode _inputMode;
    Edge _edge;
    bool _initialized;
    volatile uint32_t _count;
    mutable portMUX_TYPE _countMux = portMUX_INITIALIZER_UNLOCKED;
    bool _validateConfig() const;
    bool _configureInput();
    bool _attachInterrupt();
    static void IRAM_ATTR _onPulse(void *arg);
};
#endif

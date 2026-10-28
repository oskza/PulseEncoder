#include "PulseEncoder.h"
#include <driver/gpio.h>

PulseEncoder::PulseEncoder(uint8_t pin, InputMode inputMode, Edge edge)
    : _pin(pin),
    _inputMode(inputMode),
    _edge(edge),
    _initialized(false),
    _count(0) {}

bool PulseEncoder::_validateConfig() const {
    return _pin < GPIO_NUM_MAX
        && GPIO_IS_VALID_GPIO(_pin)
        && (_inputMode == InputMode::FLOATING
            || _inputMode == InputMode::PULL_UP
            || _inputMode == InputMode::PULL_DOWN)
        && (_edge == Edge::FALLING_EDGE
            || _edge == Edge::RISING_EDGE
            || _edge == Edge::ANY_EDGE)
        && (_inputMode == InputMode::FLOATING
            || GPIO_IS_VALID_OUTPUT_GPIO(_pin));
}

bool PulseEncoder::_configureInput() {
    gpio_config_t config{};
    config.pin_bit_mask = uint64_t{1} << _pin;
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = (_inputMode == InputMode::PULL_UP)
        ? GPIO_PULLUP_ENABLE
        : GPIO_PULLUP_DISABLE;
    config.pull_down_en = (_inputMode == InputMode::PULL_DOWN)
        ? GPIO_PULLDOWN_ENABLE
        : GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;
    return gpio_config(&config) == ESP_OK;
}

bool PulseEncoder::_attachInterrupt() {
    const esp_err_t serviceResult = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    if (serviceResult != ESP_OK && serviceResult != ESP_ERR_INVALID_STATE)
        return false;
    const auto pin = static_cast<gpio_num_t>(_pin);
    if (gpio_isr_handler_add(pin, _onPulse, this) != ESP_OK)
        return false;
    const gpio_int_type_t interruptType = (_edge == Edge::FALLING_EDGE)
        ? GPIO_INTR_NEGEDGE
        : (_edge == Edge::RISING_EDGE)
            ? GPIO_INTR_POSEDGE
            : GPIO_INTR_ANYEDGE;
    if (gpio_set_intr_type(pin, interruptType) != ESP_OK || gpio_intr_enable(pin) != ESP_OK) {
        gpio_intr_disable(pin);
        gpio_isr_handler_remove(pin);
        return false;
    }
    return true;
}

void IRAM_ATTR PulseEncoder::_onPulse(void *arg) {
    auto &encoder = *static_cast<PulseEncoder*>(arg);
    portENTER_CRITICAL_ISR(&encoder._countMux);
    ++encoder._count;
    portEXIT_CRITICAL_ISR(&encoder._countMux);
}

bool PulseEncoder::begin() {
    if (!_initialized)
        _initialized = _validateConfig()
            && _configureInput()
            && _attachInterrupt();
    return _initialized;
}

uint32_t PulseEncoder::getCount() const {
    portENTER_CRITICAL(&_countMux);
    const uint32_t count = _count;
    portEXIT_CRITICAL(&_countMux);
    return count;
}

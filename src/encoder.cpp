#include <encoder.hpp>
using namespace arduino;
void basic_encoder::do_move(basic_encoder& rhs) {
    m_pin_data = rhs.m_pin_data;
    m_pin_clk = rhs.m_pin_clk;
    m_state = rhs.m_state;
    m_position = rhs.m_position;
}
basic_encoder::basic_encoder(basic_encoder&& rhs) {
    do_move(rhs);
}
basic_encoder& basic_encoder::operator=(basic_encoder&& rhs) {
    do_move(rhs);
    return *this;
}
basic_encoder::basic_encoder(uint8_t pin_data, uint8_t pin_clk, bool pull_up) : m_pin_data(pin_data), m_pin_clk(pin_clk), m_pull_up(pull_up), m_state(-1) {
}
bool basic_encoder::initialized() const {
    return m_state != -1;
}
void basic_encoder::initialize() {
    if (m_state == -1) {
#if defined(INPUT_PULLUP)
        if(m_pull_up) {
            pinMode(m_pin_data,INPUT_PULLUP);
            pinMode(m_pin_clk,INPUT_PULLUP);
        } else {
            pinMode(m_pin_data,INPUT_PULLUP);
            digitalWrite(m_pin_data,HIGH);
            pinMode(m_pin_clk,INPUT_PULLUP);
            digitalWrite(m_pin_clk,HIGH);
        }
#else
        pinMode(m_pin_data, INPUT);
        digitalWrite(m_pin_data, HIGH);
        pinMode(m_pin_clk, INPUT);
        digitalWrite(m_pin_clk, HIGH);
#endif
        delay(2);
        int s = 0;
        if (digitalRead(m_pin_data)) s |= 1;
        if (digitalRead(m_pin_clk)) s |= 2;
        m_state = s;
        m_position = 0;
    }
}
void basic_encoder::deinitialize() {
    m_state = -1;
}
long long basic_encoder::position() {
    return m_position;
}
void basic_encoder::position(long long value) {
    m_position = value;
}
void basic_encoder::update() {
    int s = m_state & 3;
    if (digitalRead(m_pin_data)) {
        s |= 4;
    }
    if (digitalRead(m_pin_clk)) {
        s |= 8;
    }
    switch (s) {
        case 0:
        case 5:
        case 10:
        case 15:
            break;
        case 1:
        case 7:
        case 8:
        case 14:
            ++m_position;
            break;
        case 2:
        case 4:
        case 11:
        case 13:
            --m_position;
            break;
        case 3:
        case 12:
            m_position += 2;
            break;
        default:
            m_position -= 2;
            break;
    }
    m_state = (s >> 2);
}
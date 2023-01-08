#pragma once
#include <Arduino.h>
namespace arduino {
class encoder {
public:
    // indicates whether the encoder was intialized
    virtual bool initialized() const = 0;
    // initializes the encoder
    virtual void initialize()=0;
    virtual void deinitialize()=0;
    virtual long long position();
    virtual void position(long long value)=0;
};
class basic_encoder final : encoder {
    uint8_t m_pin_data, m_pin_clk;
    bool m_pull_up;
    int m_state;
    int m_position;
    void do_move(basic_encoder& rhs);
    basic_encoder(const basic_encoder& rhs)=delete;
    basic_encoder& operator=(const basic_encoder& rhs)=delete;
public:
    basic_encoder(uint8_t pin_data, uint8_t pin_clk, bool pull_up=false);
    basic_encoder(basic_encoder&& rhs);
    basic_encoder& operator=(basic_encoder&& rhs);
    virtual bool initialized() const;
    virtual void initialize();
    virtual void deinitialize();
    virtual long long position();
    virtual void position(long long value);
    void update();
    
};
template<uint8_t PinData, uint8_t PinClk, bool PullUp=false>
class int_encoder final : encoder {
public:
    using type = int_encoder;
    constexpr static const uint8_t pin_data = PinData;
    constexpr static const uint8_t pin_clk = PinClk;
    constexpr static const bool pull_up = PullUp;
private:
    static type* m_this;
    volatile int m_state;
    volatile long long m_position;
#if defined(IRAM_ATTR)
    IRAM_ATTR
#endif
    void update() {
        int s = m_state & 3;
        if (digitalRead(pin_data)) {
            s |= 4;
        }
        if (digitalRead(pin_clk)) {
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
    static
#if defined(IRAM_ATTR)
    IRAM_ATTR
#endif
    void update_thunk() {
        m_this->update();
    }
    void do_move(int_encoder& rhs) {
        m_this = this;
        m_state = rhs.m_state;
        m_position = rhs.m_position;
    }
    int_encoder(const int_encoder& rhs)=delete;
    int_encoder& operator=(const int_encoder& rhs)=delete;
public:
    int_encoder() : m_state(-1) {
    }
    int_encoder(int_encoder&& rhs) {
        do_move(rhs);
    }
    int_encoder& operator=(int_encoder&& rhs) {
        do_move(rhs);
        return *this;
    }
    virtual bool initialized() const {
        return m_state != -1;
    }
    virtual void initialize() {
        if (m_state == -1) {
#if defined(INPUT_PULLUP)
            if(pull_up) {
                pinMode(pin_data,INPUT_PULLUP);
                pinMode(pin_clk,INPUT_PULLUP);
            } else {
                pinMode(pin_data,INPUT_PULLUP);
                digitalWrite(pin_data,HIGH);
                pinMode(pin_clk,INPUT_PULLUP);
                digitalWrite(pin_clk,HIGH);
            }
#else
            pinMode(pin_data, INPUT);
            digitalWrite(pin_data, HIGH);
            pinMode(pin_clk, INPUT);
            digitalWrite(pin_clk, HIGH);
#endif
            delay(2);
            int s = 0;
            if (digitalRead(pin_data)) s |= 1;
            if (digitalRead(pin_clk)) s |= 2;
            m_state = s;
            m_position = 0;
            m_this = this;
            attachInterrupt(pin_data,update_thunk, CHANGE);
            attachInterrupt(pin_clk,update_thunk, CHANGE);
        }
    }
    virtual void deinitialize() {
        if(m_state!=-1) {
            detachInterrupt(pin_data);
            detachInterrupt(pin_clk);
            m_state = -1;
        }
    }
    virtual long long position() {
        noInterrupts();
        update();
        long long ret =  m_position;
        interrupts();
        return ret;
    }
    virtual void position(long long value) {
        noInterrupts();
        m_position = value;
        interrupts();
    }
};
template<uint8_t PinData, uint8_t PinClk, bool PullUp>
int_encoder<PinData,PinClk,PullUp>* int_encoder<PinData,PinClk,PullUp>::m_this = nullptr;

}
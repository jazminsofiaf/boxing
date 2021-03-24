#include <stdio.h>
#include "i2c.h"


// I2C driver for ESP32 written for use with esp-idf
// With calling overhead, we end up at ~100kbit/s
#define CLK_HALF_PERIOD_US (1)

#define CLK_STRETCH  (10)

static bool started;
static uint8_t g_scl_pin;
static uint8_t g_sda_pin;

static const char *TAG = "i2c";

/*!Actively drive SCL signal low */
static inline void clear_scl(void){
    gpio_set_level(g_scl_pin, 0);
}

/*!Actively drive SDA signal low */
static inline void clear_sda(void){
    gpio_set_level(g_sda_pin, 0);
}

/*!busy-wait a specified number of uS */
static inline void i2c_delay(void){
    ets_delay_us(CLK_HALF_PERIOD_US);
}

/*!Set SDA as input, allowing it to float high,
   and return current level of line, 0 or 1 */
static inline bool read_sda(void){
    gpio_set_level(g_sda_pin, 1);
    i2c_delay(); // Without this delay we get arbitration lost in i2c_stop
    return gpio_get_level(g_sda_pin); // Clock high, valid ACK
}

/*!Set SCL as input, allowing it to float high,
        and return current level of line, 0 or 1 */
static inline bool read_scl(void){
    //gpio_write(g_scl_pin, 1);
    gpio_set_level(g_scl_pin,1);
    return gpio_get_level(g_scl_pin); // Clock high, valid ACK
}

/*!Write a bit to I2C bus */
static void i2c_write_bit(bool bit){
    uint32_t clk_stretch = CLK_STRETCH;
    if (bit) {
        (void) read_sda();
    } else {
        clear_sda();
    }
    i2c_delay();
    // Clock stretching
    while (read_scl() == 0 && clk_stretch--) ;
    // SCL is high, now data is valid
    // If SDA is high, check that nobody else is driving SDA
    if (bit && read_sda() == 0) {
        ESP_LOGE(TAG,"arbitration lost in i2c_write_bit");
    }
    i2c_delay();
    clear_scl();
}

/*!Read a bit from I2C bus */
static bool i2c_read_bit(void){
    uint32_t clk_stretch = CLK_STRETCH;
    bool bit;
    // Let the slave drive data
    (void) read_sda();
    i2c_delay();
    // Clock stretching
    while (read_scl() == 0 && clk_stretch--) ;
    // SCL is high, now data is valid
    bit = read_sda();
    i2c_delay();
    clear_scl();
    return bit;
}



void i2c_init(uint8_t scl_pin, uint8_t sda_pin){
    started = false;
    g_scl_pin = scl_pin;
    g_sda_pin = sda_pin;

    /*!Pull up a gpio (general purpose input/output)
    While the button is not pressed, the GPIO will register a 1.
    When you pressed the button, the GPIO will be 0  */
    gpio_set_pull_mode(g_scl_pin,GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(g_sda_pin,GPIO_PULLUP_ONLY);

    /*!< GPIO mode: output and input mode */
    gpio_set_direction(g_scl_pin,GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(g_sda_pin,GPIO_MODE_INPUT_OUTPUT);


    // I2C bus idle state.
    gpio_set_level(g_scl_pin,1); //Set pin to open drain to ground
    gpio_set_level(g_sda_pin,1); //Set pin to open drain to ground
}

bool i2c_slave_write_with_reg(uint8_t slave_addr,uint8_t reg_addr, uint8_t data){
    bool success = false;
   do {
        i2c_start();
        if (!i2c_write(slave_addr << 1))
            break;
        if (!i2c_write(reg_addr))
            break;
        if (!i2c_write(data))
            break;
        i2c_stop();
        success = true;
    } while(0);
    if (!success) {
        ESP_LOGE(TAG,"write error");
    }
    return success;
}


// Output start condition
void i2c_start(void){
    uint32_t clk_stretch = CLK_STRETCH;
    if (started) { // if started, do a restart cond
        // Set SDA to 1
        (void) read_sda();
        i2c_delay();
        while (read_scl() == 0 && clk_stretch--) ;
        // Repeated start setup time, minimum 4.7us
        i2c_delay();
    }
    if (read_sda() == 0) {
        ESP_LOGE(TAG,"arbitration lost in i2c_start");
    }
    // SCL is high, set SDA from 1 to 0.
    clear_sda();
    i2c_delay();
    clear_scl();
    started = true;
}

// Output stop condition
void i2c_stop(void){
    uint32_t clk_stretch = CLK_STRETCH;
    // Set SDA to 0
    clear_sda();
    i2c_delay();
    // Clock stretching
    while (read_scl() == 0 && clk_stretch--) ;
    // Stop bit setup time, minimum 4us
    i2c_delay();
    // SCL is high, set SDA from 0 to 1
    if (read_sda() == 0) {
        ESP_LOGE(TAG,"arbitration lost in i2c_stop");
    }
    i2c_delay();
    started = false;
}


bool i2c_write(uint8_t byte){
    bool nack = true;
    uint8_t bit;
    for (bit = 0; bit < 8; bit++) {
        i2c_write_bit((byte & 0x80) != 0);
        byte <<= 1;
    }
    nack = i2c_read_bit();
    return !nack;
}






// code to get the serial ID of a flash chips on the OBC
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/spi.h"

#include "flash.h"
static uint cs_pin;

void setup(uint cs){
    cs_pin = cs;

        // set up
    spi_init(Flash_SPI, Flash_Baud);
    gpio_set_function(Flash_MISO,GPIO_FUNC_SPI);
    gpio_set_function(Flash_MOSI,GPIO_FUNC_SPI);
    gpio_set_function(Flash_SCK,GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(cs_pin);
    gpio_put(cs_pin, 1);
    gpio_set_dir(cs_pin, GPIO_OUT);
}

// code to get the serial ID
void get_id() {
    uint8_t ID_BUF[5];
    uint8_t SixFour_ID[64];
    gpio_put(Flash_CS, 0); // pull cs pin low to start transmission
    ID_BUF[0] = Read_ID;
    ID_BUF[1] = Dummy_byte;
    ID_BUF[2] = Dummy_byte;
    ID_BUF[3] = Dummy_byte;
    ID_BUF[4] = Dummy_byte;

    spi_write_blocking(Flash_SPI, ID_BUF, 5);
    spi_read_blocking(Flash_SPI, 0 ,SixFour_ID, 64);

    for ( int i = 0; i < 64; i++) {
        printf("%02x", SixFour_ID[i]);
    }


    gpio_put(Flash_CS, 1); // pull cs pin high to end transmission

}

int main() {
    stdio_init_all();
    sleep_ms(12000); // time to get minicom up
    printf("SPI flash example\n");
    sleep_ms(100);
    setup(Flash_CS);
    sleep_ms(100);
    get_id();

}


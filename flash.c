// code to get the serial ID of a flash chips on the OBC
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/spi.h"

#include "flash.h"

static uint cs_pin; // to drive high or low 
uint8_t page_buf[FLASH_PAGE_SIZE]; // length of data
const uint32_t target_addr = 0; // address of the first data that is to be read

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

void flash_write_enable() {
    gpio_put(Flash_CS, 0); // pull cs pin low to start transmission
    uint8_t buf[1];

    buf[0] = Write_Enable;
    spi_write_blocking(Flash_SPI, buf, 1);

    gpio_put(Flash_CS, 1); // pull cs pin high to end transmission
}

// used to determine things like if the chip is busy or completed a read/write
//      can be used at any time 
// can be modified to get the 2nd or 3rd status registers
void flash_read_register() {
    gpio_put(Flash_CS, 0); // pull cs pin low to start transmission
    uint8_t buf[1];
    uint8_t reg[8];

    buf[0] = Read_Register2;
    spi_write_blocking(Flash_SPI, buf, 1);
    spi_read_blocking(Flash_SPI, 0, reg, 8);

    gpio_put(Flash_CS, 1); // pull cs pin high to end transmission
}

void flash_read_data(uint32_t addr, uint8_t *buf, size_t len) {
    static uint8_t read_dat[3];
    read_dat[0] = Read_Data; // command to read
    read_dat[1] = addr >> 16;
    read_dat[2] = addr >> 8;
    read_dat[3] = addr; // set to 0 if reading a full page 

    gpio_put(Flash_CS, 0); // pull cs pin low to start transmission
    spi_write_blocking(Flash_SPI, read_dat, 4);
    spi_read_blocking(Flash_SPI, 0, buf, len);

    gpio_put(Flash_CS, 1); // pull cs pin high to end transmission
}

// almost the same as read data with some extra stuff
void flash_write_data(uint32_t addr, uint8_t *buf, size_t len) {

    static uint8_t write_dat[3];
    write_dat[0] = Write_Data; // command to write
    write_dat[1] = addr >> 16;
    write_dat[2] = addr >> 8;
    write_dat[3] = addr;

    //flash_write_enable(); // enable write 

    gpio_put(Flash_CS, 0); // pull cs pin low to start transmission
    spi_write_blocking(Flash_SPI, write_dat, 4);
    spi_write_blocking(Flash_SPI, buf, len);

    gpio_put(Flash_CS, 1); // pull cs pin high to end transmission

}

//void get_id(uint64_t ID) {
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

    // print to serial the unique ID
    for ( int i = 0; i < 64; i++) {
        printf("%02x", SixFour_ID[i]);
    }
    gpio_put(Flash_CS, 1); // pull cs pin high to end transmission
}

void flash_erase_4k(uint32_t addr) {
    static uint8_t erase_dat4[3];
    erase_dat4[0] = Erase_4; 
    erase_dat4[1] = addr >> 16;
    erase_dat4[2] = addr >> 8;
    erase_dat4[3] = addr;

    flash_write_enable();
    gpio_put(Flash_CS, 0);
    spi_write_blocking(Flash_SPI, erase_dat4, 4);
    gpio_put(Flash_CS, 1);
}
void flash_erase_32k(uint32_t addr) {
    static uint8_t erase_dat32[3];
    erase_dat32[0] = Erase_32; 
    erase_dat32[1] = addr >> 16;
    erase_dat32[2] = addr >> 8;
    erase_dat32[3] = addr;

    flash_write_enable();
    gpio_put(Flash_CS, 0);
    spi_write_blocking(Flash_SPI, erase_dat32, 4);
    gpio_put(Flash_CS, 1);
}
void flash_erase_64k(uint32_t addr) {
    static uint8_t erase_dat64[3];
    erase_dat64[0] = Erase_64; 
    erase_dat64[1] = addr >> 16;
    erase_dat64[2] = addr >> 8;
    erase_dat64[3] = addr;
    
    flash_write_enable();
    gpio_put(Flash_CS, 0);
    spi_write_blocking(Flash_SPI, erase_dat64, 4);
    gpio_put(Flash_CS, 1);
}
void flash_erase_chip() {
    static uint8_t erase_chip[1];
    erase_chip[0] = Erase_Chip; 

    gpio_put(Flash_CS, 0);
    spi_write_blocking(Flash_SPI, erase_chip, 1);
    gpio_put(Flash_CS, 1);
}

int main() {
    stdio_init_all();
    sleep_ms(2000); 
    printf("SPI flash example\n");
    sleep_ms(100);
    setup(Flash_CS);
    sleep_ms(100);
    //get_id();
    //flash_write_enable();
    //flash_erase_chip();
    sleep_ms(100);
    //flash_read_register();

    flash_read_data(target_addr, page_buf, FLASH_PAGE_SIZE); // read data of all 0s
    sleep_ms(100);

    // make some random data 
    uint8_t Random_Data[Random_Data_Size];

    for (int i = 0; i < Random_Data_Size; i++) {
        Random_Data[i] = i;
    }
    flash_write_enable();
    flash_write_data(target_addr, Random_Data, Random_Data_Size); // write some data
    sleep_ms(100);
    flash_read_data(target_addr, page_buf, FLASH_PAGE_SIZE); // read the new data
}



// code to get the serial ID of a flash chips on the OBC
// code also reads and writes data from flash over SPI
// DMA code doesnt work copied from pico_examples
// TODO: make some DMA code that works 

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/spi.h"

#include "flash.h"

static uint cs_pin; // to drive high or low 
static uint dma_chan;

uint8_t page_buf[FLASH_PAGE_SIZE]; // length of data
const uint32_t target_addr = 0; // address of the first data that is to be read

// from flash.c
static inline void dma_done_handle() {
    // Clear interrupt
    dma_irqn_acknowledge_channel(DMA_IRQ_0, dma_chan);

    // Deselect CS pin after transfer is done
    add_alarm_in_us(FLASH_CS_DELAY, cs_alarm, NULL, true);
}

// from flash.c
/*! \brief Wrapper function to be triggered when DMA flash transfer is done.
 */
int64_t cs_alarm(alarm_id_t id, void *user_data) {
    gpio_put(Flash_CS, 1);
    return 0;
}

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

    // DMA set up all from flash.c 
    // not sure what this all does
    /*
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config dma_config = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_dreq(&dma_config, spi_get_dreq(Flash_SPI, true));

    dma_channel_configure(dma_chan, &dma_config, &spi_get_hw(Flash_SPI)->dr, NULL, 0, false);

    // Set up interrupt after DMA finishes
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_done_handle);
    irq_set_enabled(DMA_IRQ_0, true);
    */
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

    flash_write_enable(); // enable write 

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

int flash_busy() {
    //dma_channel_is_busy(dma_chan)
}

// reads flash, changes flash, reads flash again, then erases flash
int main() { 
    stdio_init_all();
    sleep_ms(2000); 
    
    printf("SPI flash example\n");
    sleep_ms(100);
    setup(Flash_CS);
    sleep_ms(100);
    //get_id();
    //sleep_ms(100);
    //flash_read_register();
    //sleep_ms(100);
    flash_read_data(target_addr, page_buf, FLASH_PAGE_SIZE); // read data of all 0s
    sleep_ms(100);

    // make some random data 
    uint8_t Random_Data[Random_Data_Size];

    for (int i = 0; i < Random_Data_Size; i++) {
        Random_Data[i] = i;
    }
    flash_write_data(target_addr, Random_Data, Random_Data_Size); // write some data
    sleep_ms(100);
    flash_read_data(target_addr, page_buf, FLASH_PAGE_SIZE); // read the new data
    sleep_ms(100);
    flash_erase_4k(target_addr);    //erase the data
    flash_read_data(target_addr, page_buf, FLASH_PAGE_SIZE); // read the new data
    
    // DMA example from pico_examples spi_dma
    //dma_do_stuff();

    
    
}

void dma_do_stuff(){ // doesn't work
    sleep_ms(8000); // minicom time 

    // writes data to spi and then reads it back 
    #define TEST_SIZE 1024
    // Grab some unused dma channels
    const uint dma_tx = dma_claim_unused_channel(true);
    const uint dma_rx = dma_claim_unused_channel(true);
    static uint8_t txbuf[TEST_SIZE];
    static uint8_t rxbuf[TEST_SIZE];
    for (uint i = 0; i < TEST_SIZE; ++i) {
        txbuf[i] = i;
    }
    // Configure TX

    dma_channel_config c = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(Flash_SPI, true));

    dma_channel_configure(dma_tx, &c,
                          &spi_get_hw(Flash_SPI)->dr, // write address
                          txbuf, // read address
                          TEST_SIZE, // element count (each element is of size transfer_data_size)
                          false); // don't start yet    

    // Configre RX (pretty simular to tx)
    c = dma_channel_get_default_config(dma_rx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(Flash_SPI, false));
    channel_config_set_read_increment(&c, false); // this sets the speed i believe
    channel_config_set_write_increment(&c, true);

    dma_channel_configure(dma_rx, &c,
                          rxbuf, // write address
                          &spi_get_hw(Flash_SPI)->dr, // read address
                          TEST_SIZE, // element count (each element is of size transfer_data_size)
                          false); // don't start yet

    // Begin data transfer
    printf("begin the trans\n");
    dma_start_channel_mask((1u << dma_tx) | (1u << dma_rx));
    dma_channel_wait_for_finish_blocking(dma_rx);   // wait for the rx to finish
    if (dma_channel_is_busy(dma_tx)) {
        panic("RX completed before TX");    // apparently panic is a real function in c
    }
    
    printf("All good\n");
    dma_channel_unclaim(dma_tx);
    dma_channel_unclaim(dma_rx);


}



//
#define Flash_Baud 1000 * 1000 // for test 



// pins for the Data Flash chip on the OBC (spi0)
#define Flash_SPI spi0
#define Flash_MOSI 3
#define Flash_MISO 0 
#define Flash_CS 1
#define Flash_SCK 2 

// commands
#define Dummy_byte 0x00
#define Read_ID 0x4B

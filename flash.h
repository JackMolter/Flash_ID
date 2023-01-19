// values
#define Flash_Baud 1000 * 1000 // for test 
#define FLASH_PAGE_SIZE 256
#define FLASH_SECTOR_SIZE 4096
#define Random_Data_Size 256
#define FLASH_CS_DELAY 100 // us
// __not_in_flash_func means a function wont be stored in flash 

// pins
#define Flash_SPI spi0
#define Flash_MOSI 3
#define Flash_MISO 0 //4
#define Flash_CS 1
#define Flash_SCK 2 

// commands
#define Dummy_byte 0x00
#define Read_ID 0x4B
#define Write_Enable 0x06
#define Read_Data 0x03
#define Write_Data 0x02
#define Erase_4 0x20
#define Erase_32 0x52
#define Erase_64 0xD8
#define Erase_Chip 0xC7

#define Read_Register1 0x05 // probably the only useful one
#define Read_Register2 0x35
#define Read_Register3 0x15



// initialize functions 

// write enable 
void __not_in_flash_func(flash_write_enable)(); 

// read status registers
void __not_in_flash_func(flash_read_register)();

// read data
// needs address of data to read, length of data
void __not_in_flash_func(flash_read_data)(uint32_t addr, uint8_t *buf, size_t len);

// write data
void __not_in_flash_func(flash_write_data)(uint32_t addr, uint8_t *buf, size_t len);

// 4k Erase
//  addr is start of 4k block address 
// Reases 4k-bytes of data, the erased state is all 1s.
void __not_in_flash_func(flash_erase_4k)(uint32_t addr);

// 32k erase
void __not_in_flash_func(flash_erase_32k)(uint32_t addr);

// 64k erase
void __not_in_flash_func(flash_erase_64k)(uint32_t addr);

// chip erase
void __not_in_flash_func(flash_erase_chip)();

// DMA stuff
// check if busy
int __not_in_flash_func(check_busy)();
static inline void __not_in_flash_func(dma_done_handle)();
int64_t __not_in_flash_func(cs_alarm)();

void __not_in_flash_func(dma_do_stuff)();

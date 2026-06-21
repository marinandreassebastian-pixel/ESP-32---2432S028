// ========================================
// Setari pentru placa ESP32-2432S028 (CYD)
// ========================================

// 1. Driver-ul ecranului
#define ILI9341_2_DRIVER 

// 2. Corectia culorilor inversate la nivel de hardware
// (Rezolva problema in care albastru aparea galben si rosu aparea cyan)
#define TFT_INVERSION_ON

// Daca cumva dupa inversare rosu si albastru sunt inca schimbate intre ele,
// sterge cele doua bare (//) de la linia de mai jos:
// #define TFT_RGB_ORDER TFT_BGR 

// ========================================
// 3. Pinii corecti pentru placa ta
// ========================================
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15  // Chip select
#define TFT_DC    2  // Data/Command
#define TFT_RST  -1  // Setat pe -1 pentru ca resetul e legat intern la placa
#define TFT_BL   21  // Backlight (Lumina de fundal)

// ========================================
// 4. Pinul pentru Touchscreen
// ========================================
#define TOUCH_CS 33

// ========================================
// 5. Setari de functionare si Fonturi
// ========================================
#define TFT_BACKLIGHT_ON HIGH 

// Incarcarea fonturilor de baza
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4

// Frecventele recomandate pentru stabilitatea imaginii si a touch-ului
#define SPI_FREQUENCY  55000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
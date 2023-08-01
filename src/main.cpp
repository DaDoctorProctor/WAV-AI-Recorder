#include "Arduino.h"

/* SPI SD Libraries */
#include "FS.h"
#include "SD.h"
#include "SPI.h"

/* I2C Microphone Libraries */
#include <driver/i2s.h>
#include <SPIFFS.h>

/* I2C Microphone Settings */
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE   (16000) //16000
#define I2S_SAMPLE_BITS   (16)
#define I2S_READ_LEN      (16 * 1024)
#define RECORD_TIME       (6) //Seconds
#define I2S_CHANNEL_NUM   (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

File file;
File sdfile;

const int headerSize = 44;

class homero {
private:
    int repetitions = 40;
    char indexFromTextFile;
    char FileName;
    char* TW1;
    char* TW2;
    char* TW3;
    char* TW4;
    char *TWChar[];

public:
    /* Get and set for repetitions */
    void setRepetitions(int value) {
        repetitions = value;
    }
    int getRepetitions() const {
        return repetitions-1;
    }


    /* Get and set for the test words */
    void setTW1(char* TestWord){
      TW1 = TestWord;
    }
    char* getTW1() const {
      return TW1;
    }
    /* Get and set for the test words */
    void setTW2(char* TestWord){
      TW2 = TestWord;
    }
    char* getTW2() const {
      return TW2;
    }
    /* Get and set for the test words */
    void setTW3(char* TestWord){
      TW3 = TestWord;
    }
    char* getTW3() const {
      return TW3;
    }
    /* Get and set for the test words */
    void setTW4(char* TestWord){
      TW4 = TestWord;
    }
    char* getTW4() const {
      return TW4;
    }


    /* Get and set for the index */
    void setIndexFromTextFile(char index){
      indexFromTextFile = index;
    }
    char getIndexFromTextFile() const {
      return indexFromTextFile;
    }

    /* Get and set for the index */
    void setFileName(char fileName){
      FileName = fileName;
    }
    char getFileName() const {
      return FileName;
    }

};


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path, homero& data){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
    //data.setIndexFromTextFile(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}


/* SPIFFS 
----------------------
*/
void listSPIFFS(void) {
  Serial.println(F("\r\nListing SPIFFS files:"));
  static const char line[] PROGMEM =  "=================================================";

  Serial.println(FPSTR(line));
  Serial.println(F("  File name                              Size"));
  Serial.println(FPSTR(line));

  fs::File root = SPIFFS.open("/");
  if (!root) {
    Serial.println(F("Failed to open directory"));
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(F("Not a directory"));
    return;
  }

  fs::File file = root.openNextFile();
  while (file) {

    if (file.isDirectory()) {
      Serial.print("DIR : ");
      String fileName = file.name();
      Serial.print(fileName);
    } else {
      String fileName = file.name();
      Serial.print("  " + fileName);
      // File path can be 31 characters maximum in SPIFFS
      int spaces = 33 - fileName.length(); // Tabulate nicely
      if (spaces < 1) spaces = 1;
      while (spaces--) Serial.print(" ");
      String fileSize = (String) file.size();
      spaces = 10 - fileSize.length(); // Tabulate nicely
      if (spaces < 1) spaces = 1;
      while (spaces--) Serial.print(" ");
      Serial.println(fileSize + " bytes");
    }

    file = root.openNextFile();
  }

  Serial.println(FPSTR(line));
  Serial.println();
  delay(1000);
}

void wavHeader(byte* header, int wavSize){
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSize = wavSize + headerSize - 8;
  header[4] = (byte)(fileSize & 0xFF);
  header[5] = (byte)((fileSize >> 8) & 0xFF);
  header[6] = (byte)((fileSize >> 16) & 0xFF);
  header[7] = (byte)((fileSize >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  header[20] = 0x01;
  header[21] = 0x00;
  header[22] = 0x01;
  header[23] = 0x00;
  header[24] = 0x80;
  header[25] = 0x3E;
  header[26] = 0x00;
  header[27] = 0x00;
  header[28] = 0x00;
  header[29] = 0x7D;
  header[30] = 0x00;
  header[31] = 0x00;
  header[32] = 0x02;
  header[33] = 0x00;
  header[34] = 0x10;
  header[35] = 0x00;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(wavSize & 0xFF);
  header[41] = (byte)((wavSize >> 8) & 0xFF);
  header[42] = (byte)((wavSize >> 16) & 0xFF);
  header[43] = (byte)((wavSize >> 24) & 0xFF);
  
}

void SPIFFSInit(fs::FS &fs, const char * filename){
  // if(!SPIFFS.begin(true)){
  //   Serial.println("SPIFFS initialisation failed!");
  //   while(1) yield();
  // }

  //SPIFFS.remove(filename);

  //file = SPIFFS.open(filename, FILE_WRITE);
  sdfile = fs.open(filename, FILE_WRITE);

  // if(!file){
  //   Serial.println("File is not available!");
  // }

  byte header[headerSize];
  wavHeader(header, FLASH_RECORD_SIZE);

  //file.write(header, headerSize);
  sdfile.write(header,headerSize);

  //listSPIFFS();
}

void SDInit(fs::FS &fs, const char * filename){
  sdfile = fs.open(filename, FILE_WRITE);
  byte header[headerSize];
  wavHeader(header, FLASH_RECORD_SIZE);
  sdfile.write(header,headerSize);
}

void i2sInit(){
  /* DESCRIPTION: This is the configuration file for the i2c protocol microphone. */
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 64,
    .dma_buf_len = 1024,
    .use_apll = 1
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}


void i2s_adc_data_scale(uint8_t * d_buff, uint8_t* s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint32_t dac_value = 0;
    for (int i = 0; i < len; i += 2) {
        dac_value = ((((uint16_t) (s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
        d_buff[j++] = 0;
        d_buff[j++] = dac_value * 256 / 2048;
    }
}

void i2s_adc(void *arg){
    /* DESCRIPTION: Enables the i2c where the microphone is located, captures de audio inside the buffer
    and finally it saves it into the flash/sdcard. */
    int i2s_read_len = I2S_READ_LEN;
    int flash_wr_size = 0;
    size_t bytes_read;

    char* i2s_read_buff = (char*) calloc(i2s_read_len, sizeof(char));
    uint8_t* flash_write_buff = (uint8_t*) calloc(i2s_read_len, sizeof(char));

    i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    //i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY); dafuq does this
    
    Serial.printf("         3 \n"); delay(1000);
    Serial.printf("         2 \n"); delay(1000);
    Serial.printf("         1 \n"); delay(1000);
    Serial.println(" *** Recording Start *** ");
    while (flash_wr_size < FLASH_RECORD_SIZE) {
        /* Read data from I2S bus, in this case, from ADC. */
        i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        //example_disp_buf((uint8_t*) i2s_read_buff, 64);
        /* save original data from I2S(ADC) into flash. */
        i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2s_read_buff, i2s_read_len);
        //file.write((const byte*) flash_write_buff, i2s_read_len); use to write in the flash
        sdfile.write((const byte*) flash_write_buff, i2s_read_len);

        flash_wr_size += i2s_read_len;
        ets_printf("Sound recording %u%%\n", flash_wr_size * 100 / FLASH_RECORD_SIZE);
        //ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL)); stack size for debug
    }
    //file.close();
    sdfile.close();
    Serial.printf("Done. Thank you. \n");

    free(i2s_read_buff);
    i2s_read_buff = NULL;
    free(flash_write_buff);
    flash_write_buff = NULL;

    //listSPIFFS();
    vTaskDelete(NULL);
}

void example_disp_buf(uint8_t* buf, int length)
{
    printf("======\n");
    for (int i = 0; i < length; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    printf("======\n");
}


void setWord(homero& data, int iteration){
  if (iteration >= 0  && iteration <= 9) {
    data.setTW1("Adelante");
  } else if (iteration >= 10 && iteration <= 19){
    data.setTW1("Atras");
  } else if (iteration >= 20 && iteration <= 29){
    data.setTW1("Izquierda");
  } else if (iteration >= 30 && iteration <= data.getRepetitions()){
    data.setTW1("Derecha");
  }
}

void recordAdministrator(homero& data){
  
  /* DESCRIPTION: This is the main function responsible for recording the audios. */

  /* Read the config file to find the number of test subject and create a folder */
  //readFile(SD, "/config.txt", data);
  //char folderIndex = data.getIndexFromTextFile();

  // const char student[] = "student";
  // char folderName[strlen(student) + folderIndex + 1];
  // strcpy(folderName, student);
  // strcat(folderName, folderIndex);

  const char dirName[] = "/student";
  const char dirIndex[] = "1";
  createDir(SD, dirName);

  for (int i = 0; i <= data.getRepetitions(); i++) {
    
    // const char dirName[] = "/student";
    // const char dirIndex[] = "1";
    const char str1[] = "/student/recording";
    /* Make sure it's large enough to hold the integer as a string */
    int index = i;
    char str2[20]; 
    /*Convert the integer to a C-style const char array */
    std::sprintf(str2, "%d", index);
    const char str3[] = ".wav";
    /* Calculate the size of the destination char array */
    char result[strlen(str1) + strlen(str2) + strlen(str3) + 1];
    /* Copy the first string to the result char array */
    strcpy(result, str1);
    strcat(result, str2);
    strcat(result, str3);
    /*const char filename[] = "/recording1.wav";*/

    /* Initiate the main loop for the recording */
    SDInit(SD, result); 
    i2sInit();
    delay(1000);
    Serial.printf("Please say the word:    ");
    setWord(data,i);
    Serial.printf(data.getTW1());
    Serial.printf("\n");
    xTaskCreate(i2s_adc, "i2s_adc", 1024 * 2, NULL, 1, NULL);
    delay(8000);
  }
  
}

void sdStatus(){
  if(!SD.begin(5)){
    Serial.println("Card Mount Failed. The SD module does not respond.");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card slotted. Insert an SD card to the slot.");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}


void setup(){
  Serial.begin(115200);

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  Serial.printf("\n");
}

void loop(){

  homero data;
  Serial.printf("Welcome to the AI training system. Please select an option:\n");
  Serial.printf("[1] Record new test subject\n");
  Serial.printf("[2] SD Card status\n");
  Serial.printf("[3] List file DIR\n");
  Serial.printf("\n");

  char command1;
  while(true){
      if(Serial.available() > 0){
        command1 = Serial.read();

        if(command1 == '1'){
          recordAdministrator(data);
          Serial.printf("\n");
          break;
        } else if(command1 == '2'){
          sdStatus();
          Serial.printf("\n");
          break;
        } else if(command1 == '3'){
          listDir(SD, "/student", 0);
          Serial.printf("\n");
          break;
        } else if(command1 == '4'){
          deleteFile(SD, "/student/recording0.wav");
          deleteFile(SD, "/student/recording1.wav");
          deleteFile(SD, "/student/recording2.wav");
          deleteFile(SD, "/student/recording3.wav");
          deleteFile(SD, "/student/recording4.wav");
          deleteFile(SD, "/student/recording5.wav");
          deleteFile(SD, "/student/recording6.wav");
          deleteFile(SD, "/student/recording7.wav");
          deleteFile(SD, "/student/recording8.wav");
          deleteFile(SD, "/student/recording9.wav");
          deleteFile(SD, "/student/recording10.wav");
          deleteFile(SD, "/student/recording11.wav");
          removeDir(SD, "/student");
          Serial.printf("\n");
          break;
        }
    }
  }

}


  // listDir(SD, "/", 0);
  // createDir(SD, "/mydir");
  // listDir(SD, "/", 0);
  // removeDir(SD, "/mydir");
  // listDir(SD, "/", 2);
  // writeFile(SD, "/hello.txt", "Hello ");
  // appendFile(SD, "/hello.txt", "World!\n");
  // readFile(SD, "/hello.txt");
  // deleteFile(SD, "/foo.txt");
  // renameFile(SD, "/hello.txt", "/foo.txt");
  // readFile(SD, "/foo.txt");
  // testFileIO(SD, "/test.txt");
#include "FS.h"          // 파일 시스템 인터페이스 라이브러리
#include "SD.h"          // SD 카드 제어를 위한 라이브러리
#include "SPI.h"         // SPI 통신 라이브러리

// SD 카드에 연결된 핀 정의
#define CS_PIN   10      // SD 카드의 CS (Chip Select) 핀
#define MOSI_PIN 11      // SPI 데이터 출력 핀 (Master Out)
#define CLK_PIN  12      // SPI 클럭 핀
#define MISO_PIN 13      // SPI 데이터 입력 핀 (Master In)

// 지정된 경로에 있는 디렉토리 또는 파일 목록 출력 함수
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);
    File root = fs.open(dirname);  // 디렉토리 열기
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();  // 첫 번째 파일 열기
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels > 0){
                listDir(fs, file.path(), levels - 1);  // 재귀적으로 하위 디렉토리 탐색
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();  // 다음 파일로 이동
    }
}

// 디렉토리 생성 함수
void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

// 디렉토리 삭제 함수
void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

// 파일 읽기 함수
void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }
    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());  // 한 글자씩 출력
    }
    file.close();
}

// 파일 새로 쓰기 함수 (덮어쓰기)
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

// 파일에 내용 추가 쓰기 함수
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

// 파일 이름 변경 함수
void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);

    if (!fs.exists(path1)) {
        Serial.println("Original file does not exist");
        return;
    }

    if (fs.exists(path2)) {
        Serial.println("Target file exists. Deleting...");
        fs.remove(path2);  // 덮어쓰기 방지를 위해 기존 파일 삭제
    }

    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

// 파일 삭제 함수
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

// 파일 읽기/쓰기 성능 측정 함수
void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];  // 버퍼 512바이트
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;

    // 파일 읽기 테스트
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len > 512 ? 512 : len;
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read in %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }

    // 파일 쓰기 테스트
    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    start = millis();
    for(size_t i = 0; i < 2048; i++){
        file.write(buf, 512);  // 512바이트씩 총 1MB 작성
    }
    end = millis() - start;
    Serial.printf("%u bytes written in %u ms\n", 2048 * 512, end);
    file.close();
}

// 초기 설정 함수
void setup(){
    Serial.begin(115200);  // 시리얼 통신 시작

    // HSPI 버스를 이용한 SPI 설정
    SPIClass spi = SPIClass(HSPI);
    spi.begin(CLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);  // 순서: SCK, MISO, MOSI, CS

    // SD 카드 마운트 시도
    if(!SD.begin(CS_PIN, spi)){
        Serial.println("Card Mount Failed");
        return;
    }

    // 카드 타입 확인
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC) Serial.println("MMC");
    else if(cardType == CARD_SD) Serial.println("SDSC");
    else if(cardType == CARD_SDHC) Serial.println("SDHC");
    else Serial.println("UNKNOWN");

    // 용량 출력
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    // 파일 시스템 테스트 실행
    listDir(SD, "/", 0);                            // 루트 디렉토리 목록 출력
    writeFile(SD, "/hello.txt", "Hello ");          // 새 파일 생성 및 쓰기
    appendFile(SD, "/hello.txt", "World!\n");       // 내용 추가
    readFile(SD, "/hello.txt");                     // 파일 읽기
    renameFile(SD, "/hello.txt", "/foo.txt");       // 파일 이름 변경
    testFileIO(SD, "/test.txt");                    // 입출력 속도 테스트
}

void loop(){
    // 반복 동작 없음
}

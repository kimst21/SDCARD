#include "FS.h"          // 파일 시스템 라이브러리
#include "SD.h"          // SD 카드 제어 라이브러리
#include "SPI.h"         // SPI 통신 라이브러리

#define CS_PIN 10
#define MOSI_PIN 11
#define CLK_PIN 12
#define MISO_PIN 13

// 디렉토리의 내용을 나열하는 함수
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);  // 디렉토리 경로 출력
    File root = fs.open(dirname);                      // 지정된 디렉토리 열기
    if(!root){                                         // 디렉토리 열기 실패 시
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){                           // 지정된 경로가 디렉토리가 아닌 경우
        Serial.println("Not a directory");
        return;
    }
    
    File file = root.openNextFile();                   // 다음 파일/디렉토리 열기
    while(file){                                       // 파일이 존재하는 동안 반복
        if(file.isDirectory()){                        // 하위 항목이 디렉토리인 경우
            Serial.print("  DIR : "); 
            Serial.println(file.name());               // 디렉토리 이름 출력
            if(levels){                                // 재귀 호출: 하위 디렉토리 나열
                listDir(fs, file.path(), levels -1);
            }
        } else {                                       // 파일인 경우
            Serial.print("  FILE: ");
            Serial.print(file.name());                 // 파일 이름 출력
            Serial.print("  SIZE: ");
            Serial.println(file.size());               // 파일 크기 출력
        }
        file = root.openNextFile();                    // 다음 항목으로 이동
    }
}

// 디렉토리를 생성하는 함수
void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){                                // 디렉토리 생성 시도
        Serial.println("Dir created");                // 성공 메시지 출력
    } else {
        Serial.println("mkdir failed");               // 실패 메시지 출력
    }
}

// 디렉토리를 삭제하는 함수
void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){                               // 디렉토리 삭제 시도
        Serial.println("Dir removed");               // 성공 메시지 출력
    } else {
        Serial.println("rmdir failed");              // 실패 메시지 출력
    }
}

// 파일을 읽는 함수
void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);                        // 파일 열기
    if(!file){                                        // 파일 열기 실패 시
        Serial.println("Failed to open file for reading");
        return;
    }
    Serial.print("Read from file: ");
    while(file.available()){                          // 파일 내용이 존재하는 동안 반복
        Serial.write(file.read());                    // 파일의 데이터를 시리얼 모니터에 출력
    }
    file.close();                                     // 파일 닫기
}

// 파일에 쓰기 (새 파일 생성 또는 기존 파일 덮어쓰기)
void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);            // 쓰기 모드로 파일 열기
    if(!file){                                        // 파일 열기 실패 시
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){                          // 메시지 쓰기
        Serial.println("File written");              // 성공 메시지 출력
    } else {
        Serial.println("Write failed");              // 실패 메시지 출력
    }
    file.close();                                     // 파일 닫기
}

// 파일에 내용 추가하기
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);           // 추가 모드로 파일 열기
    if(!file){                                        // 파일 열기 실패 시
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){                          // 메시지 추가하기
        Serial.println("Message appended");          // 성공 메시지 출력
    } else {
        Serial.println("Append failed");             // 실패 메시지 출력
    }
    file.close();                                     // 파일 닫기
}

// 파일 이름 변경 함수
void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {                    // 파일 이름 변경 시도
        Serial.println("File renamed");             // 성공 메시지 출력
    } else {
        Serial.println("Rename failed");            // 실패 메시지 출력
    }
}

// 파일 삭제 함수
void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){                              // 파일 삭제 시도
        Serial.println("File deleted");             // 성공 메시지 출력
    } else {
        Serial.println("Delete failed");            // 실패 메시지 출력
    }
}

// 파일 읽기/쓰기 속도 테스트 함수
void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);                        // 파일 열기
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;

    if(file){                                         // 파일 읽기 속도 테스트
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len > 512 ? 512 : len;   // 최대 512바이트씩 읽기
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);                // 파일 쓰기 속도 테스트
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    start = millis();
    for(size_t i = 0; i < 2048; i++){
        file.write(buf, 512);                        // 512바이트씩 쓰기 반복
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();                                    // 파일 닫기
}

void setup(){
    Serial.begin(115200);

SPIClass spi = SPIClass(HSPI);
// SPI 객체 생성
 spi.begin(CLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

    if(!SD.begin(CS_PIN, spi)){                                 // SD 카드 초기화
        Serial.println("Card Mount Failed");
        return;
    }

    uint8_t cardType = SD.cardType();                // SD 카드 유형 확인
    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC) Serial.println("MMC");
    else if(cardType == CARD_SD) Serial.println("SDSC");
    else if(cardType == CARD_SDHC) Serial.println("SDHC");
    else Serial.println("UNKNOWN");

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    listDir(SD, "/", 0);            // 루트 디렉토리 나열
    createDir(SD, "/mydir");        // 디렉토리 생성
    removeDir(SD, "/mydir");        // 디렉토리 삭제
    writeFile(SD, "/hello.txt", "Hello ");  // 파일 쓰기
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    testFileIO(SD, "/test.txt");
}

void loop(){
    // 루프 내 반복 작업 없음
}

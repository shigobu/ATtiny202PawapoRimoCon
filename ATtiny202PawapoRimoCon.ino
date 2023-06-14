#include "CH9329_Keyboard.h"

// ボーレート設定値計算マクロ。最後の"+0.5"は、四捨五入させるため？？
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

#define SW_OFF HIGH
#define SW_ON LOW

const int LEFT_SW_PIN = PIN_PA1;
const int RIGHT_SW_PIN = PIN_PA2;
const int SW_ON_THRESHOLD = 10;
uint8_t reportData[KEY_REPORT_DATA_LENGTH] = {};

// USART初期化
void USART0_init(void) {
  PORTA.DIR &= ~PIN7_bm;
  PORTA.DIR |= PIN6_bm;
  USART0.BAUD = (uint16_t)USART0_BAUD_RATE(CH9329_DEFAULT_BAUDRATE);
  USART0.CTRLB |= USART_TXEN_bm;
}

// 一つの値送信
void USART0_sendValue(uint8_t c) {
  while (!(USART0.STATUS & USART_DREIF_bm)) {
    ;
  }
  USART0.TXDATAL = c;
}

// 複数の値送信
void USART0_sendValue(uint8_t* c, size_t length) {
  for (size_t i = 0; i < length; i++ ) {
    USART0_sendValue(c[i]);
  }
}

// CH9329へキー押下情報送信
void CH9329_write(uint8_t c){
    size_t length = 0;
    CH9329_Keyboard.press(c);
    length = CH9329_Keyboard.getReportData(reportData, KEY_REPORT_DATA_LENGTH);
    USART0_sendValue(reportData, length);

    CH9329_Keyboard.release(c);
    length = CH9329_Keyboard.getReportData(reportData, KEY_REPORT_DATA_LENGTH);
    USART0_sendValue(reportData, length);
}

void setup() {
  USART0_init();
  CH9329_Keyboard.begin();

  pinMode(LEFT_SW_PIN, INPUT_PULLUP);
  pinMode(RIGHT_SW_PIN, INPUT_PULLUP);
  delay(5000);
}

void loop() {
  //チャタリング対策。SW_ON_THRESHOLDミリ秒ONが続いたものを採用する。
  int rightCount = 0;
  while (digitalRead(RIGHT_SW_PIN) == SW_ON){
    rightCount++;
    delay(1);
    if (rightCount > SW_ON_THRESHOLD){
      CH9329_write(KEY_RIGHT_ARROW);
      while (digitalRead(RIGHT_SW_PIN) == SW_ON);
      break;
    }
  }

  int leftCount = 0;
  while (digitalRead(LEFT_SW_PIN) == SW_ON){
    leftCount++;
    delay(1);
    if (leftCount > SW_ON_THRESHOLD){
      CH9329_write(KEY_LEFT_ARROW);
      while (digitalRead(LEFT_SW_PIN) == SW_ON);
      break;
    }
  }
}

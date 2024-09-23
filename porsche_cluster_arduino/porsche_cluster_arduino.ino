// CAN-BUS Shield from loovee@seeed.cc
// CAN команды панели - https://github.com/VintageCollector/Porsche_Panamera_970-CAN-Bus-ID
// За основу взят код - https://github.com/VintageCollector/cluster-dashboard-ets2-ats/tree/Porsche-Panamera-970
#include <string.h>
#include <SPI.h>

#define CAN_2515

// Джойстик аналоговый для удобства перебора меню панели
// Analog joystick for easy navigation through the panel menu
#define axis_X A1    // Axis Х -> Analog 0
#define axis_Y A2    // Axis Y -> Analog 1
#define button 2     // Button D2 (not used)

// Set SPI CS Pin according to your hardware

#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
const int SPI_CS_PIN  = BCM8;
const int CAN_INT_PIN = BCM25;
#else

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
#endif

#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif

// STATE ENUMS
typedef enum{
  I_OFF = 0,
  I_LEFT = 1,
  I_RIGHT = 2,
  I_HAZZARD = 3,
} INDICATOR;

// STATE PRND
typedef enum{
  G_DRIVE = 0x50,
  G_NEUTRAL = 0x60,
  G_REVERSE = 0x70,
  G_PARKING = 0x80,
  G_MANUAL = 0xE0,
} GEARPOSITION;

//  STATE DRIVEMODE
typedef enum{
  G_FIRST = 0x01,
  G_SECOND = 0x02,
  G_THIRD = 0x03,
  G_FOURTH = 0x04,
  G_FIFTH = 0x05,
  G_SIXTH = 0x08,
  G_SEVENTH = 0x09,
  G_EIGHTH = 0x0a,
} GEAR;

/*
  CAN ENUMS
*/
typedef enum{
  L_BRAKE = 0b10000000,
  L_FOG = 0b00010011,
  L_BACKLIGHT = 0b00000001,
  L_MAIN = 0b00010111,
  L_DIP = 0b00010011,
} CAN_LIGHTS;

bool s_handbrake = false;
bool s_ignition = true;

uint8_t s_esp = 0;
String temp_esp;

bool s_light_parking = true;
bool s_light_dip = false;
bool s_light_main = false;
bool s_light_fog = false;

uint8_t s_light_indicator = I_OFF;
uint8_t gear_indicator = G_PARKING;
uint8_t gear_selection = G_FIRST;

uint16_t s_speed = 0;
String temp_speed;
uint16_t s_rpm = 800;
String temp_rpm;
uint16_t s_fuel = 1000; // 1000 - max;
uint8_t s_engine_temp = 91;
String temp_engine_temp;
uint8_t s_oil_temp = 100;
String temp_oil_temp;
uint8_t s_gear = 10;
String temp_gear;
uint8_t ouside_temp = 10;

uint8_t s_time_hour = 14;
uint8_t s_time_minute = 01;
uint8_t s_time_sec = 0;
uint8_t s_time_day = 14;
uint8_t s_time_month = 10;
uint16_t s_time_year = 2024;

int buttonState = 0;
int buttonState2 = 0;
int buttonState3 = 0;
int buttonState4 = 0;
int buttonState5 = 0;


// CAN фреймы
uint8_t ignition_frame_on[8] = {0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t handbrake_frame[8] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t ignition_frame_off[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t speed_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t rpm_frame[8] = {0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00};
uint8_t steering_wheel_frame[7] = {0x83, 0xFD, 0xFC, 0x00, 0x00, 0xFF, 0xF1};
uint8_t oil_temp_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00};
uint8_t indicator_frame[8] = {0x80, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t engine_temp_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00};
uint8_t lights_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t gear_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t mode_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00};
uint8_t time_frame[8] = {0x13, 0x2E, 0x1E, 0x01, 0x4F, 0xDC, 0x07, 0xFD};
uint8_t vinnumber_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// uint8_t abs_frame[8] = {0x00, 0xE0, 0xB3, 0xFC, 0xF0, 0x43, 0x00, 0x65};
uint8_t abs_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t check_engine_frame[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// uint8_t fuel_frame[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t fuel_frame[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t abs_counter_frame[6] = {0xD4, 0XF1, 0x01, 0xFF, 0xFF, 0xFF};
uint8_t airbag_counter_frame[2] = {0xC3, 0xFF};

// Переменные таймера
uint32_t lastTime = 0;
uint16_t canCounter = 0;

// Переменные для хранения значений осей джойстика
int val_X, val_Y;
unsigned int x;
unsigned int y;


void canSendAbsCounter(){
  CAN.sendMsgBuf(0x1E1, 0, 6, abs_counter_frame);
  abs_counter_frame[0] = ((abs_counter_frame[0] + 1) | 0xF0);
  abs_counter_frame[1] = ((abs_counter_frame[1] + 1) | 0xF0);
}

void canSendAirbagCounter(){
  CAN.sendMsgBuf(0x0D7, 0, 2, airbag_counter_frame);
  airbag_counter_frame[0]++;
}

void read_joy()
{  
  val_X = analogRead(axis_X);    // Считываем аналоговое значение оси Х
  val_Y = analogRead(axis_Y);    // Считываем аналоговое значение оси Y
  //x = (val_X / 3)- 30;
  //y = (val_Y / 3)+ 10;
}

void canSendvinnumber(){

  // Читаем джойстик и отправляем команды на цифровое меню
  read_joy();
  //digitalWrite(INT_PIN, 1); //if here is a CAN ID with its Bytes, remove the '//' at the begin of this line!
  buttonState = digitalRead(button);
  
  // Joy down -> menu down
  if (val_Y < 130)
  {
  
  vinnumber_frame[0] = 0x12;
  vinnumber_frame[2] = 0x01;
  CAN.sendMsgBuf(0x5bf, 0, 8, vinnumber_frame);
  }
  // Joy up -> menu up
  buttonState2 = digitalRead(4);
    if (val_Y > 830)
    
  {
  
  vinnumber_frame[0] = 0x12;
  vinnumber_frame[2] = 0x0f;
  CAN.sendMsgBuf(0x5bf, 0, 8, vinnumber_frame);
  }
  
  buttonState3 = digitalRead(7);
    if (val_X < 130)
  {
  vinnumber_frame[0] = 0x03;
  vinnumber_frame[1] = 0x13;
  vinnumber_frame[2] = 0x01;
  CAN.sendMsgBuf(0x5bf, 0, 8, vinnumber_frame);
  }
  
  buttonState5 = digitalRead(5);
    if (val_X > 830)
  {
  vinnumber_frame[0] = 0x00;
  vinnumber_frame[1] = 0x13;
  vinnumber_frame[2] = 0x10;
  CAN.sendMsgBuf(0x5bf, 0, 8, vinnumber_frame);
  }
}

void canSendGear(){
  switch(s_gear) {
    case 0:
      gear_indicator = G_REVERSE;
      break;
    case 1:
      if (s_speed == 0) {
        gear_indicator = G_NEUTRAL;
      }
      break;
    case 2:
      gear_indicator = G_DRIVE;
      gear_selection = G_FIRST;
      break;
    case 3:
      gear_indicator = G_DRIVE;
      gear_selection = G_SECOND;
      break;
    case 4:
      gear_indicator = G_DRIVE;
      gear_selection = G_THIRD;
      break;
    case 5:
      gear_indicator = G_DRIVE;
      gear_selection = G_FOURTH;
      break;
    case 6:
      gear_indicator = G_DRIVE;
      gear_selection = G_FIFTH;
      break;
    case 7:
      gear_indicator = G_DRIVE;
      gear_selection = G_SIXTH;
      break;
    case 8:
      gear_indicator = G_DRIVE;
      gear_selection = G_SEVENTH;
      break;
    case 9:
      gear_indicator = G_DRIVE;
      gear_selection = G_EIGHTH;
      break;   
    default:
      gear_indicator = G_PARKING;
      gear_selection = G_FIRST;
      break;
  }
   gear_frame[1] = gear_indicator;
   gear_frame[0] = gear_selection;
   
    // лаунч контроль
    // gear_frame[2] = 0x01;

    CAN.sendMsgBuf(0x310, 0, 8, gear_frame);
}

void canSendLights(){
  uint16_t lights = 0;

  if(s_light_parking) lights |= L_BACKLIGHT;
  if(s_light_dip) lights |= L_DIP;
  if(s_light_main) lights |= L_MAIN;
  if(s_light_fog) lights |= L_FOG;
  
  lights_frame[2] = lights;

  CAN.sendMsgBuf(0x662, 0, 8, lights_frame);
}

void canSendEngineTemp(){
  engine_temp_frame[5] = s_engine_temp + 80;

  if(s_rpm > 1 && s_rpm < 2200)
  {
    engine_temp_frame[6] = (int(s_rpm * 14) >> 8);
  }
  else if(s_rpm > 2200 && s_rpm < 3000)
  {
  engine_temp_frame[6] = (int(s_rpm * 7) >> 8);}  
  else if(s_rpm > 3000 && s_rpm < 5000)
  {
  engine_temp_frame[6] = (int(s_rpm * 6) >> 8);}
  else if(s_rpm > 5000 && s_rpm < 8000)
  {
    engine_temp_frame[6] = (int(s_rpm * 3) >> 8);
  }
  CAN.sendMsgBuf(0x522, 0, 8, engine_temp_frame);
}


void canSendIndicator(){

// unsigned long current_time = millis();

  //if (current_time - prev_time >= interval) {
    // Update the previous time
  //  prev_time = current_time;

  if(s_light_indicator == I_LEFT){

    indicator_frame[2] = 0x04;
  }
  else if (s_light_indicator == I_RIGHT){
    indicator_frame[2] = 0x08;
  }
  else if (s_light_indicator == I_HAZZARD){
    indicator_frame[2] = 0x0c;
  }
  else if (s_light_indicator == I_OFF){
    indicator_frame[2] = 0x00;
  }
  
    CAN.sendMsgBuf(0x363, 0, 8, indicator_frame);
}

// TempOil
// 0x80 (128) - 0xd3 (211)
void canSendOilTemp(){
  oil_temp_frame[5] = s_oil_temp + 60;
  CAN.sendMsgBuf(0x6b5, 0, 8, oil_temp_frame);
}

// 0x01 check engine light flashing
// 0x05  check engine light flashing + reduced power warning lcd
void canSendCheckEngine(){
  check_engine_frame[0] = 0x05;
  CAN.sendMsgBuf(0x522, 0, 8, check_engine_frame);
}

void canSendSteeringWheel(){
  steering_wheel_frame[1] = 0;
  steering_wheel_frame[2] = 0;

  CAN.sendMsgBuf(0x0C4, 0, 7, steering_wheel_frame);
}

void canSendRPM(){
  uint16_t rpm = s_rpm; 

  rpm_frame[2] = (int(rpm * 0.34) & 0xff); 
  rpm_frame[3] = (int(rpm * 0.34) >> 8);

  CAN.sendMsgBuf(0x3a3, 0, 8, rpm_frame);
}

void canSendFuel(){
  uint16_t level = min(100+(s_fuel*8), 8000);
  fuel_frame[0] = level;
  fuel_frame[1] = (level >> 8);

  fuel_frame[2] = fuel_frame[0];
  fuel_frame[3] = fuel_frame[1];

  CAN.sendMsgBuf(0x349, 0, 5, fuel_frame);
}

void canSendSpeed(){
  uint16_t speed_value = s_speed/2;
  speed_frame[2] = speed_value*1.125;
  speed_frame[5] = speed_value*1.11;
  CAN.sendMsgBuf(0x105, 0, 8, speed_frame);
}

void canSendIgnitionFrame(){
  if(s_ignition){
    CAN.sendMsgBuf(0x3C0, 0, 8, ignition_frame_on);
    ignition_frame_on[3]++;
  }else{
    CAN.sendMsgBuf(0x3C0, 0, 8, ignition_frame_off);
    ignition_frame_off[3]++;
  }
}

void canSendHandbrake(){
  if(s_handbrake){
    handbrake_frame[0] = 0x11;
  }else{
    handbrake_frame[0] = 0x00;
  }
  CAN.sendMsgBuf(0x30d, 0, 8, handbrake_frame);
}

void canSendMode(){

  ouside_temp = 33;

  mode_frame[1] = 0x40; // Sport + mode
  mode_frame[6] = ouside_temp; // температура
  CAN.sendMsgBuf(0x677, 0, 8, mode_frame);
}

void canSendTime(){
  time_frame[0] = s_time_hour;
  time_frame[1] = s_time_minute;
  time_frame[2] = s_time_sec;

  time_frame[3] = s_time_day;
  time_frame[4] = (s_time_month << 4) | 0x0F;

  time_frame[5] = (uint8_t)s_time_year;
  time_frame[6] = (uint8_t)(s_time_year >> 8);
  CAN.sendMsgBuf(0x2F8, 0, 8, time_frame);
}

void canSendAbs(){
  // abs_frame[2] = ((((abs_frame[2] >> 4) + 3) << 4) & 0xF0) | 0x03;
  // CAN.sendMsgBuf(0x19E, 0, 8, abs_frame); 
  // abs_frame[4] = 0x08; ABS LAMP 
  if (s_esp == 1) {
    abs_frame[4] = 0x10;
  } else {
    abs_frame[4] = 0x00;
  }
  CAN.sendMsgBuf(0X38C, 0, 8, abs_frame);
}


// Отправка данных 
void canSend(){
  uint32_t courentTime = millis();
  if(courentTime - lastTime > 10){
    //10 ms interval
    // Зажигание
    canSendIgnitionFrame();
    canSendHandbrake();
    canSendSpeed();
    canSendSteeringWheel();
    canSendRPM();
    canSendGear();
    canSendMode();
    // canSendCheckEngine();

    if(canCounter % 20 == 0){ //200 ms interval
      canSendLights();
       // internal 600ms timer
      canSendAbs();
      canSendEngineTemp();
      canSendAbsCounter();
      canSendAirbagCounter();
      canSendFuel();
      canSendOilTemp();
    }

    if(canCounter % 15 == 0){ //150 ms interval
     canSendvinnumber();
    }

    if(canCounter % 50 == 0){ //500 ms interval
      canSendIndicator();
    }

    if(canCounter % 100 == 0){ //1000 ms interval
      canSendTime();
    }

    canCounter++;
    lastTime = courentTime;
  }
}

// Получаем данные из серийного порта через индекс и разделитель :
// get value from serial port by separator and index
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    SERIAL_PORT_MONITOR.setTimeout(50);
    while(!Serial){};

    while (CAN_OK != CAN.begin(CAN_500KBPS)) {             // init can bus : baudrate = 500k
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");

    pinMode(2, INPUT_PULLUP); // right
    pinMode(4, INPUT_PULLUP); // down
    pinMode(5, INPUT_PULLUP); // mid
    pinMode(6, INPUT_PULLUP); // up
    //pinMode(7, INPUT_PULLUP); // left
    pinMode(button, INPUT_PULLUP); // left

    delay(1000);                       // send data per 100ms
    SERIAL_PORT_MONITOR.println("CAN BUS sendMsgBuf ok!");

}

void loop() {
  
  // Основной блок, отсылаем всю информацию на приборную панель
  //------------------------------------------
  canSend();
  
  unsigned char stmp1[8] = {0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x20};
  CAN.sendMsgBuf(0x5fa, 0, 8, stmp1); // elevation
  
  unsigned char stmp2[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x00, 0x00};
  CAN.sendMsgBuf(0x663, 0, 8, stmp2); // battery voltage
  //------------------------------------------

  // Этот блок принимает данные на серийный порт, от игр.
  // Для тестирования это все можно закомментировать
  // ---------------------------------------
  if (SERIAL_PORT_MONITOR.available() > 0) {
     // Читаем полученные данные и сохраняем их в переменной
     String receivedData = SERIAL_PORT_MONITOR.readString();
     // Отправляем ответ обратно в Serial порт
    temp_speed = getValue(receivedData, ':', 0);
    temp_rpm = getValue(receivedData, ':', 1);
    temp_gear = getValue(receivedData, ':', 2);
    temp_esp = getValue(receivedData, ':', 3);
    s_gear = temp_gear.toInt();
    s_speed = temp_speed.toInt();
    s_rpm = temp_rpm.toInt();
    s_esp = temp_esp.toInt();
  // ---------------------------------------


// String yval = getValue(myString, ':', 1);
//     SERIAL_PORT_MONITOR.println(receivedData);
//     SERIAL_PORT_MONITOR.println(temp_speed);
 }
}

// END FILE

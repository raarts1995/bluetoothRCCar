#include <Arduino_FreeRTOS.h>
#include <timers.h>
#include <queue.h>

#define headLightL A2
#define headLightR A3
#define rearLightL 16
#define rearLightR 14

#define steerL A1
#define steerR A0

#define motorF 5
#define motorR 6

#define inputDataBuffer 32
#define bluetooth Serial1

#define bluetoothTimeout 500 //ms

bool bluetoothConnection = true;

//data vb: M255,R0,S0,L1,Ih
//M(otor):     pwm value (0 - 255)
//R(everse):   boolean
//S(teering):  l(eft), 0 (center), r(ight)
//L(ight):     1 (on), 0 (off)
//I(ndicator): l(eft indicator), r(ight indicator), h(azard lights), o(ff)

typedef struct {
  byte speed;
  bool direction;
} motor_t;
motor_t motorData = {0, 0};

#define steerCenter 0
#define steerRight  1
#define steerLeft   2
byte steerData= 0;

#define lightStateBit  (1 << 0)
#define indicatorLBit  (1 << 1)
#define indicatorRBit  (1 << 2)
#define hazardBit      (1 << 3)
#define indicatorState (1 << 4)
byte lightData = 0;

TaskHandle_t btTaskHandle = NULL;
TaskHandle_t motorTaskHandle = NULL;
TaskHandle_t steerTaskHandle = NULL;
TaskHandle_t lightTaskHandle = NULL;

TimerHandle_t indicatorTimer = NULL;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //USB debug
  Serial1.begin(9600);  //bluetooth module

  pinMode(headLightL, OUTPUT);
  pinMode(headLightR, OUTPUT);
  pinMode(rearLightL, OUTPUT);
  pinMode(rearLightR, OUTPUT);
  pinMode(steerL, OUTPUT);
  pinMode(steerR, OUTPUT);
  pinMode(motorF, OUTPUT);
  pinMode(motorR, OUTPUT);

  xTaskCreate(
    btTask, //task
    "Bluetooth", //task name
    256, //stack size
    NULL, //parameters
    3, //priority
    &btTaskHandle //task handle
  );

  xTaskCreate(
    motorTask,
    "Motor",
    128,
    NULL,
    2,
    &motorTaskHandle
  );

  xTaskCreate(
    steerTask,
    "Steer",
    128,
    NULL,
    2,
    &steerTaskHandle
  );
  
  xTaskCreate(
    lightTask,
    "Light",
    128,
    NULL,
    1,
    &lightTaskHandle
  );
  
  indicatorTimer = xTimerCreate(
    "indicator timer", //timer name
    500/portTICK_PERIOD_MS, //timer period
    pdTRUE, //autoreload
    NULL, //timer ID
    indicatorTimerTick //callback function
  );
}

void loop() {
  // put your main code here, to run repeatedly:
}

void motorTask(void *params) {
  while (true) {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
      if(motorData.direction) {
        analogWrite(motorF, 0);
        analogWrite(motorR, motorData.speed);
      } else {
        analogWrite(motorF, motorData.speed);
        analogWrite(motorR, 0);
      }
    }
  }
}

void steerTask(void *params) {
  while (true) {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
      if (steerData == steerLeft) {
        digitalWrite(steerL, HIGH);
        digitalWrite(steerR, LOW);
      }
      else if (steerData == steerRight) {
        digitalWrite(steerL, LOW);
        digitalWrite(steerR, HIGH);
      }
      else {
        digitalWrite(steerL, LOW);
        digitalWrite(steerR, LOW);
      }
    }
  }
}

void lightTask(void *params) {
  bool blink = false;
  
  while (true) {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
      //handle indicators
      blink = !bluetoothConnection; //blink is always on without bluetooth connection
      if (lightData & (indicatorLBit | hazardBit)) { //left indicator
        blink = true;
      }
      else {
        digitalWrite(headLightL, (lightData & lightStateBit) != 0);
        digitalWrite(rearLightL, (lightData & lightStateBit) != 0);
      }
      
      if (lightData & (indicatorRBit | hazardBit)) { //right indicator
        blink = true;
      }
      else {
        digitalWrite(headLightR, (lightData & lightStateBit) != 0);
        digitalWrite(rearLightR, (lightData & lightStateBit) != 0);
      }
      
      if (blink) {
        if (!xTimerIsTimerActive(indicatorTimer)) { //start timer if not active
          if ((lightData & lightStateBit) != 0)
            lightData |= indicatorState; //turn on indicatorstatebit to immediately toggle indicator light when headlights are on
          else
            lightData &= ~indicatorState;
          
          indicatorTimerTick(indicatorTimer); //call first time for immediate response
          if (!xTimerStart(indicatorTimer, 0)) {
            ;
          }
        }
      }
      else { //indicators not used
        if (!xTimerStop(indicatorTimer, 0)) {
          ;
        }
      }
    }
  }
}

void indicatorTimerTick(TimerHandle_t tmr) {
  if ((lightData & indicatorLBit) | (lightData & hazardBit) | !bluetoothConnection) {
    digitalWrite(headLightL, (lightData & indicatorState) == 0);
    digitalWrite(rearLightL, (lightData & indicatorState) == 0);
  }
  if ((lightData & indicatorRBit) | (lightData & hazardBit) | !bluetoothConnection) {
    digitalWrite(headLightR, (lightData & indicatorState) == 0);
    digitalWrite(rearLightR, (lightData & indicatorState) == 0);
  }
  lightData ^= indicatorState; //toggle state of indicator bit
}

void btTask(void *param) {
  char dataIn[inputDataBuffer + 1] = {'\0'};
  unsigned long lastMsgTime = 0;
  
  while (true) {
    if (checkData(dataIn)) {
      lastMsgTime = millis();
      bluetoothConnection = true;
    }
    
    if (((lastMsgTime + bluetoothTimeout) < millis()) && bluetoothConnection) {
      bluetoothConnection = false;
      motorData = {0, 0};
      steerData = 0;
      xTaskNotifyGive(motorTaskHandle);
      xTaskNotifyGive(steerTaskHandle);
      xTaskNotifyGive(lightTaskHandle);
    }
    
    vTaskDelay(50/portTICK_PERIOD_MS);
  }
}

bool checkData(char* dataIn) {
  bool received = false;
  while (bluetooth.available()) {
    received = true;
    char c = (char)bluetooth.read();
    bluetooth.print(c);
    if (c == '\n') {
      parseData(dataIn);
      memset(dataIn, '\0', inputDataBuffer + 1);
    }
    else {
      if (strlen(dataIn) < inputDataBuffer)
        dataIn[strlen(dataIn)] = c;
      else
        dataIn[inputDataBuffer] = c;
    }
  }
  return received;
}

void parseData(char* dataIn) {
  char tmp[8] = {'\0'};
  
  while (true) {
    memset(tmp, '\0', sizeof(tmp));
    int len = sizeof(tmp) - 1; 
    if (strchr(dataIn, ',') != NULL) {
      int newLen = strchr(dataIn, ',') - dataIn;
      if (newLen < len)
        len = newLen;
    }
    strncpy(tmp, dataIn, len); //copy data until comma to tmp
    
    switch(tmp[0]) {
      case 'M': //Motor
        motorData.speed = atoi(&tmp[1]);
        xTaskNotifyGive(motorTaskHandle);
        break;
        
      case 'R': //Reverse
        motorData.direction = (tmp[1] == '1');
        xTaskNotifyGive(motorTaskHandle);
        break;
        
      case 'S': //Steering
        switch (tmp[1]) {
          case 'l':
            steerData = steerLeft;
            break;
          case 'r':
            steerData = steerRight;
            break;
          default: //0 or unknown value
            steerData = steerCenter;
            break;
        }
        xTaskNotifyGive(steerTaskHandle);
        break;
        
      case 'L': //Lights
        if (tmp[1] == '1') //toggle lights
          lightData ^= lightStateBit;
        else //0 or unknown value: lights off
          lightData &= ~lightStateBit;
        
        xTaskNotifyGive(lightTaskHandle);
        break;
        
      case 'I': //Indicator
        switch (tmp[1]) {
          case 'l': //toggle left indicator
            lightData ^= indicatorLBit;
            lightData &= ~indicatorRBit;
            break;
          case 'r': //togle right indicator
            lightData ^= indicatorRBit;
            lightData &= ~indicatorLBit;
            break;
          case 'h': //toggle hazard light
            lightData ^= hazardBit;
            break;
          default: //0 or unknown value: all off
            lightData &= ~(indicatorLBit | indicatorRBit | hazardBit);
            break;
        }
        xTaskNotifyGive(lightTaskHandle);
        break;
        
      default:
        break;
    }
    
    dataIn = strchr(dataIn, ','); //set the dataIn pointer after the comma
    if (dataIn != NULL)
      dataIn++;
    else 
      break;
  }
}

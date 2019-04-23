#include <Arduino_FreeRTOS.h>

#define headLightL A3
#define headLightR A2
#define rearLightL 14
#define rearLightR 16

#define steerL A1
#define steerR A0

#define driveF 5
#define driveR 6

#define bluetooth Serial1;

//data vb: M255,R0,S0,L1,Ih
//M(otor):     pwm value (0 - 255)
//R(everse):   boolean
//S(teering):  l(eft), 0 (center), r(ight)
//L(ight):     1 (on), 0 (off)
//I(ndicator): l(eft indicator), r(ight indicator), h(azard lights), o(ff)
/*String dataIn;

enum steerDir { Center, Left, Right };
typedef struct btData{
  byte motor;
  bool motorDir;
  int steer;
  byte lights;
};*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);

  pinMode(headLightL, OUTPUT);
  pinMode(headLightR, OUTPUT);
  /*pinMode(steerL, OUTPUT);
  pinMode(steerR, OUTPUT);
  digitalWrite(steerL, LOW);
  digitalWrite(steerR, LOW);*/

  xTaskCreate(
    ledTask,
    (const portCHAR *)"Blink",
    128,  // Stack size
    NULL,
    2,  // priority
    NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void ledTask(void *params) {
  while (true) {
    digitalWrite(headLightL, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(headLightL, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void checkData() {
  while (bluetooth.available()) {
    char c = bluetooth.read();
    if (c == '\n') {
      parseData();
      dataIn = "";
    }
  }
}

void parseData() {
  String tmp;
  while (1) {
    if (dataIn.indexOf(",") != -1)
      tmp = dataIn.substring(0, dataIn.indexOf(","));
  }
}

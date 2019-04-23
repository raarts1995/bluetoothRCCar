#include <Arduino_FreeRTOS.h>

#define headLightL A3
#define headLightR A2
<<<<<<< HEAD
#define rearLightL 16
#define rearLightR 14
=======
#define rearLightL 14
#define rearLightR 16
>>>>>>> master

#define steerL A1
#define steerR A0

#define driveF 5
#define driveR 6

<<<<<<< HEAD
#define bluetooth Serial1
=======
#define bluetooth Serial1;
>>>>>>> master

//data vb: M255,R0,S0,L1,Ih
//M(otor):     pwm value (0 - 255)
//R(everse):   boolean
//S(teering):  l(eft), 0 (center), r(ight)
//L(ight):     1 (on), 0 (off)
//I(ndicator): l(eft indicator), r(ight indicator), h(azard lights), o(ff)
<<<<<<< HEAD
//C(onnect):   respond with hello
String dataIn;

/*enum steerDir { Center, Left, Right };
=======
/*String dataIn;

enum steerDir { Center, Left, Right };
>>>>>>> master
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
<<<<<<< HEAD
  pinMode(rearLightL, OUTPUT);
  pinMode(rearLightR, OUTPUT);
=======
>>>>>>> master
  /*pinMode(steerL, OUTPUT);
  pinMode(steerR, OUTPUT);
  digitalWrite(steerL, LOW);
  digitalWrite(steerR, LOW);*/

  xTaskCreate(
    ledTask,
<<<<<<< HEAD
    "Blink",
=======
    (const portCHAR *)"Blink",
>>>>>>> master
    128,  // Stack size
    NULL,
    2,  // priority
    NULL);
<<<<<<< HEAD

  xTaskCreate(
    btTask,
    "Bluetooth",
    256,
    NULL,
    3,
    NULL);
=======
>>>>>>> master
}

void loop() {
  // put your main code here, to run repeatedly:
}

void ledTask(void *params) {
  while (true) {
<<<<<<< HEAD
    digitalWrite(headLightL, HIGH);
    digitalWrite(headLightR, HIGH);
    digitalWrite(rearLightL, HIGH);
    digitalWrite(rearLightR, HIGH);
    vTaskDelay( 500/portTICK_PERIOD_MS );
    digitalWrite(headLightL, LOW);
    digitalWrite(headLightR, LOW);
    digitalWrite(rearLightL, LOW);
    digitalWrite(rearLightR, LOW);
    vTaskDelay( 500/portTICK_PERIOD_MS );
  }
}

void btTask(void *param) {
  while (true) {
    checkData();
    vTaskDelay(50/portTICK_PERIOD_MS);
=======
    digitalWrite(headLightL, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(headLightL, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
>>>>>>> master
  }
}

void checkData() {
  while (bluetooth.available()) {
<<<<<<< HEAD
    char c = (char)bluetooth.read();
=======
    char c = bluetooth.read();
>>>>>>> master
    if (c == '\n') {
      parseData();
      dataIn = "";
    }
<<<<<<< HEAD
    else
      dataIn += c;
=======
>>>>>>> master
  }
}

void parseData() {
  String tmp;
<<<<<<< HEAD
  bool last = false;
  while (!last) {
    if (dataIn.indexOf(',') != -1) {
      tmp = dataIn.substring(0, dataIn.indexOf(','));
      Serial.println(tmp);
    }
    else {
      tmp = dataIn;
      last = true;
    }
    
    Serial.println("tmp: " + tmp + "\nrest: " + dataIn);
    char c = tmp.charAt(0);
    tmp.remove(0, 1);
    switch(c) {
      case 'M': //Motor
        break;
      case 'R': //Reverse
        break;
      case 'S': //Steering
        break;
      case 'L': //Lights
        break;
      case 'I': //Indicator
        break;
      case 'C': //Connect
        bluetooth.println("hello");
      default:
        break;
    }
    dataIn.remove(0, tmp.length() + 1);
=======
  while (1) {
    if (dataIn.indexOf(",") != -1)
      tmp = dataIn.substring(0, dataIn.indexOf(","));
>>>>>>> master
  }
}

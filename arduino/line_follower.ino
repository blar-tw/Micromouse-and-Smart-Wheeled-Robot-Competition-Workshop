// BitRacer Pro Max line-following example (Arduino IDE / ATSAMD21G18)
// 2026 TMIRC Summer Camp - Dual-Platform Wheeled Robot Design & Application
// Flow: button start -> rotate & sample to calibrate IR max/min -> PD line follow
//       -> stop after passing the right-side marker twice (finish line)

//------ IR sensor control -----//
#define IR_1 A6
#define IR_2 A5
#define IR_3 A4
#define IR_4 A3
#define IR_5 A0
#define IR_L A2
#define IR_R A1
#define IRcontrol 13
//------ LED & button control -----//
#define LED_R 30   // right LED
#define LED_L 31   // left LED
#define BUTTON 25  // button
//------- motor control -----------//
#define PWML 2    // left motor PWM
#define BIN_1 28  // left motor direction 1
#define BIN_2 29  // left motor direction 2
#define AIN_1 32  // right motor direction 1
#define AIN_2 33  // right motor direction 2
#define PWMR 3    // right motor PWM
//------- speed PD control -----------//
#define kp 100
#define kd 200
#define Basic_speed 100
//------- arrays --------------//
int IR[7] = { 0 };                                            // raw IR values
int IR_Max[7] = { 1, 1, 1, 1, 1, 1, 1 };                      // IR max init values
int IR_Min[7] = { 1023, 1023, 1023, 1023, 1023, 1023, 1023 }; // IR min init values
float IR_Nor[7] = { 0 };                                      // normalized IR values
//------- variables --------------//
float x = 0, err = 0, ekd = 0, errold = 0;
int pd = 0;
int L_Cnt = 0, R_Cnt = 0;

void setup() {
  Serial.begin(9600);       // set baud rate
  pinMode(IR_1, INPUT);
  pinMode(IR_2, INPUT);
  pinMode(IR_3, INPUT);
  pinMode(IR_4, INPUT);
  pinMode(IR_5, INPUT);
  pinMode(IR_R, INPUT);
  pinMode(IR_L, INPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_L, OUTPUT);
  pinMode(BUTTON, INPUT);
  digitalWrite(LED_R, HIGH);  // right LED off
  digitalWrite(LED_L, HIGH);  // left LED off

  // wait for button press, then start calibration
  while (digitalRead(BUTTON) != 0)
    ;
  delay(1000);
  Motor(100, 100);
  int CONT = 0;
  while (1) {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_L, LOW);
    Ir();
    Maxmin();
    if (CONT >= 50) break;
    else CONT++;
  }
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_L, HIGH);
  Motor(0, 0);
}

void loop() {
  while (digitalRead(BUTTON) == 0) {
    delay(1000);
    while (1) {
      follow();
      prompt();
      if (R_Cnt == 2) {  // right-side marker passed twice -> finish
        follow();
        delay(400);
        break;
      }
    }
    while (1) {
      Motor(0, 0);
    }
  }
}

void follow() {
  Ir();       // read IR
  Nor();      // normalize
  weights();  // weight calc + PD control
  Print1();   // serial monitor
}

void Ir()  // read IR sensors
{
  digitalWrite(IRcontrol, HIGH);  // turn IR on
  IR[6] = analogRead(IR_L);
  IR[0] = analogRead(IR_R);
  IR[3] = analogRead(IR_3);
  IR[4] = analogRead(IR_4);
  IR[2] = analogRead(IR_2);
  IR[5] = analogRead(IR_5);
  IR[1] = analogRead(IR_1);
  digitalWrite(IRcontrol, LOW);  // turn IR off
}

void Maxmin()  // track max/min per sensor
{
  Ir();
  for (int j = 0; j < 6; j++) {
    if (IR[j] > IR_Max[j]) {
      IR_Max[j] = IR[j];
    }
    if (IR[j] < IR_Min[j]) {
      IR_Min[j] = IR[j];
    }
  }
}

void Nor()  // normalization
{
  for (int i = 0; i < 6; i++) {
    IR_Nor[i] = (1023.0 / (IR_Max[i] - IR_Min[i])) * (IR[i] - IR_Min[i]);
    if (IR_Nor[i] > 1023) {
      IR_Nor[i] = 1023;
    }
    if (IR_Nor[i] < 1) {
      IR_Nor[i] = 1;
    }
  }
}

void weights()  // weighted position + PD control
{
  char mode = 'C';
  if (mode == 'C') {
    if (IR_Nor[1] > 700 || IR_Nor[2] > 700 || IR_Nor[3] > 700 || IR_Nor[4] > 700 || IR_Nor[5] > 700)
      x = (IR_Nor[1] * -2.0 + IR_Nor[2] * -1.0 + IR_Nor[3] * 0 + IR_Nor[4] * 1.0 + IR_Nor[5] * 2.0)
          / (IR_Nor[1] + IR_Nor[2] + IR_Nor[3] + IR_Nor[4] + IR_Nor[5]);
  }
  errold = err;
  err = 0 - x;
  ekd = err - errold;
  pd = (kp * err) + (kd * ekd);
  Motor(Basic_speed - pd, Basic_speed + pd);
}

void Motor(int16_t PWM_L, int16_t PWM_R)  // drive motors
{
  if (PWM_R >= 255) PWM_R = 255;
  else if (PWM_R <= -255) PWM_R = -255;
  if (PWM_L >= 255) PWM_L = 255;
  else if (PWM_L <= -255) PWM_L = -255;
  if (PWM_L >= 0) {
    digitalWrite(BIN_1, HIGH);
    digitalWrite(BIN_2, LOW);
    analogWrite(PWML, PWM_L);
  } else {
    digitalWrite(BIN_1, LOW);
    digitalWrite(BIN_2, HIGH);
    analogWrite(PWML, abs(PWM_L));
  }
  if (PWM_R >= 0) {
    digitalWrite(AIN_1, LOW);
    digitalWrite(AIN_2, HIGH);
    analogWrite(PWMR, PWM_R);
  } else {
    digitalWrite(AIN_1, HIGH);
    digitalWrite(AIN_2, LOW);
    analogWrite(PWMR, abs(PWM_R));
  }
}

void prompt()  // detect left/right white markers on the track
{
  static int L_status = 0, R_status = 0, C_status = 0;
  if (C_status == 0 && IR[1] > 600 && IR[3] > 600 && IR[5] > 600) {
    // intersection (left, center, right all see white line)
    C_status = 1;
    digitalWrite(LED_L, LOW);
    digitalWrite(LED_R, LOW);
  } else if (C_status == 1) {
    if (IR[6] > 400) L_status = 1;
    if (IR[0] > 400) R_status = 1;
    else if (L_status == 1 && R_status == 1 && IR[6] < 400 && IR[0] < 400) {
      digitalWrite(LED_L, HIGH);
      digitalWrite(LED_R, HIGH);
      L_status = 0;
      R_status = 0;
      C_status = 0;
    }
  } else {
    if (L_status == 0 && IR[6] > 400) {
      L_status = 1;
      digitalWrite(LED_L, LOW);
    } else if (L_status == 1 && IR[6] < 400) {
      digitalWrite(LED_L, HIGH);
      L_status = 0;
      L_Cnt++;
    }
    if (R_status == 0 && IR[0] > 400) {
      R_status = 1;
      digitalWrite(LED_R, LOW);
    } else if (R_status == 1 && IR[0] < 400) {
      digitalWrite(LED_R, HIGH);
      R_status = 0;
      R_Cnt++;
    }
  }
}

void Print1()  // serial monitor (uncomment as needed)
{
  // Serial.print(IR[1]); Serial.print(" \t ");
  // Serial.print(IR[2]); Serial.print(" \t ");
  // Serial.print(IR[3]); Serial.print(" \t ");
  // Serial.print(IR[4]); Serial.print(" \t ");
  // Serial.print(IR[5]);
  // Serial.print(" Nor: ");
  // Serial.print(IR_Nor[1]); Serial.print(" \t ");
  // Serial.print(IR_Nor[2]); Serial.print(" \t ");
  // Serial.print(IR_Nor[3]); Serial.print(" \t ");
  // Serial.print(IR_Nor[4]); Serial.print(" \t ");
  // Serial.print(IR_Nor[5]); Serial.println();
}

// BitRacer Pro Max 循線範例（Arduino IDE / ATSAMD21G18）
// 2026 TMIRC 暑期營 - 雙平台輪型機器人設計與應用
// 流程：按鈕啟動 → 旋轉取樣校正紅外線最大最小值 → PD 循線 → 偵測右側提示符號 2 次後停車

//------紅外線感測器控制-----//
#define IR_1 A6
#define IR_2 A5
#define IR_3 A4
#define IR_4 A3
#define IR_5 A0
#define IR_L A2
#define IR_R A1
#define IRcontrol 13
//------LED&BUT控制-----//
#define LED_R 30   // LED燈右邊
#define LED_L 31   // LED燈左邊
#define BUTTON 25  // 按鈕
//-------馬達控制-----------//
#define PWML 2    // 左邊馬達腳
#define BIN_1 28  // 左邊馬達正反腳1
#define BIN_2 29  // 左邊馬達正反腳2
#define AIN_1 32  // 右邊馬達正反腳1
#define AIN_2 33  // 右邊馬達正反腳2
#define PWMR 3    // 右邊馬達腳
//-------速度PD控制-----------//
#define kp 100
#define kd 200
#define Basic_speed 100
//-------陣列設置--------------//
int IR[7] = { 0 };                                            // 紅外線初始值
int IR_Max[7] = { 1, 1, 1, 1, 1, 1, 1 };                      // 紅外線最大初始值
int IR_Min[7] = { 1023, 1023, 1023, 1023, 1023, 1023, 1023 }; // 紅外線最小初始值
float IR_Nor[7] = { 0 };                                      // 紅外線正規化初始值
//-------變數設置--------------//
float x = 0, err = 0, ekd = 0, errold = 0;
int pd = 0;
int L_Cnt = 0, R_Cnt = 0;

void setup() {
  Serial.begin(9600);       // 設定傳輸鮑率
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
  digitalWrite(LED_R, HIGH);  // 關閉右邊LED燈
  digitalWrite(LED_L, HIGH);  // 關閉左邊LED燈

  // 等待按鈕按下後開始校正
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
      if (R_Cnt == 2) {  // 右側提示符號經過2次 → 終點
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
  Ir();       // 讀取紅外線
  Nor();      // 歸一化
  weights();  // 權重計算與PD控制
  Print1();   // 視窗監控
}

void Ir()  // 讀取紅外線副程式
{
  digitalWrite(IRcontrol, HIGH);  // 開啟紅外線
  IR[6] = analogRead(IR_L);
  IR[0] = analogRead(IR_R);
  IR[3] = analogRead(IR_3);
  IR[4] = analogRead(IR_4);
  IR[2] = analogRead(IR_2);
  IR[5] = analogRead(IR_5);
  IR[1] = analogRead(IR_1);
  digitalWrite(IRcontrol, LOW);  // 關閉紅外線
}

void Maxmin()  // 最大最小值副程式
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

void Nor()  // 歸一化副程式
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

void weights()  // 權重與PD控制
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

void Motor(int16_t PWM_L, int16_t PWM_R)  // 驅動馬達
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

void prompt()  // 提示符號控制（賽道左右白色標記偵測）
{
  static int L_status = 0, R_status = 0, C_status = 0;
  if (C_status == 0 && IR[1] > 600 && IR[3] > 600 && IR[5] > 600) {
    // 路口（左中右同時偵測到白線）
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

void Print1()  // 視窗監控副程式（需要時取消註解）
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

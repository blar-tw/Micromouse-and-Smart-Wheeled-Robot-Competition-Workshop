// BitRacer Pro Max 循線程式 (MakeCode / micro:bit)
// 重建自研習簡報 p.35–43
//
// 怎麼用：
//   1. MakeCode → 擴展 → 貼上 https://github.com/CorgiQQQ/BitRacer_Pro_Max
//   2. 切到「JavaScript」分頁，把整份程式碼貼進去
//   3. 切回「積木」分頁，會自動變成積木
//   4. 連接 micro:bit → 下載
//
// 操作：
//   按 A     → 校正（車子前進取樣紅外線最大最小值）
//   按 B     → 開始循線
//   按 A+B   → 停車
//
// 用到的擴展積木（簡報 p.20–21）：
//   SmartCar.setIRPower(true/false)                          紅外線開關
//   SmartCar.getIR(index)                                    讀取第 index 顆感測器 (0~6)
//   SmartCar.setMotor(SmartCar.MotorList.左輪/右輪/雙輪, 速度)   速度 -1000~1000
//   感測器索引：0~4 = IR-1~IR-5，5 = IR-L，6 = IR-R
//
// LED 積木已對照擴展原始碼確認：SmartCar.setLED(SmartCar.LEDList.左邊/右邊/兩邊, on)
// I2C 位址 = 0x10（正常用積木就好，不用自己寫 I2C）

let 校正模式 = 0
let 校正值 = [0, 0, 0, 0, 0, 0, 0]
let 紅外線值 = [0, 0, 0, 0, 0, 0, 0]
let 最大值 = [0, 0, 0, 0, 0, 0, 0]
let 最小值 = [5000, 5000, 5000, 5000, 5000, 5000, 5000]
let 目標最大值 = 4000
let 目標最小值 = 1
let bassSPD = 350
let LinePOS = 0
let pError = 0
let pErrorOld = 0
let dError = 0
let PDvalue = 0
let NMZ_IR = 0
let 路口狀態 = 0
let 左提示符號狀態 = 0
let 右提示符號狀態 = 0
let 左提示符號次數 = 0
let 右提示符號次數 = 0

serial.redirectToUSB()

// ---- 按鈕 ----
input.onButtonPressed(Button.A, function () {
    校正模式 = 1
    SmartCar.setMotor(SmartCar.MotorList.雙輪, 350)
    basic.pause(300)
    SmartCar.setMotor(SmartCar.MotorList.雙輪, 0)
})
input.onButtonPressed(Button.B, function () {
    校正模式 = 2
})
input.onButtonPressed(Button.AB, function () {
    校正模式 = 3
})

// ---- 主迴圈 ----
basic.forever(function () {
    if (校正模式 == 1) {
        SmartCar.setIRPower(true)
        紅外線取樣()
        最大最小值()
    } else if (校正模式 == 2) {
        SmartCar.setIRPower(true)
        紅外線取樣()
        正規化()
        權重()
        PD(250, 350)
        起終點()
    } else if (校正模式 == 3) {
        SmartCar.setMotor(SmartCar.MotorList.雙輪, 0)
    }
})

// ---- 讀取七顆紅外線 ----
function 紅外線取樣() {
    for (let index = 0; index <= 6; index++) {
        NMZ_IR = SmartCar.getIR(index)
        紅外線值[index] = NMZ_IR
    }
}

// ---- 記錄每顆的最大 / 最小值（校正用）----
function 最大最小值() {
    for (let index4 = 0; index4 <= 6; index4++) {
        NMZ_IR = 紅外線值[index4]
        if (NMZ_IR < 最小值[index4]) {
            最小值[index4] = NMZ_IR
        }
        if (NMZ_IR > 最大值[index4]) {
            最大值[index4] = NMZ_IR
        }
    }
}

// ---- 正規化：把每顆感測器對應到 目標最小值~目標最大值 ----
function 正規化() {
    for (let index2 = 0; index2 <= 6; index2++) {
        NMZ_IR = 紅外線值[index2]
        let NMZ_IRmax = 最大值[index2]
        let NMZ_IRmin = 最小值[index2]
        let NMZ_IRadj = Math.map(NMZ_IR, NMZ_IRmin, NMZ_IRmax, 目標最小值, 目標最大值)
        校正值[index2] = Math.constrain(NMZ_IRadj, 目標最小值, 目標最大值) + 200
    }
}

// ---- 權重：用 IR-1~IR-5 算出賽道位置 LinePOS（中心 = 3）----
function 權重() {
    let 權重和 = 0
    let 紅外線和 = 0
    for (let index3 = 0; index3 <= 4; index3++) {
        NMZ_IR = 校正值[index3]
        權重和 += NMZ_IR * (index3 + 1)
        紅外線和 += NMZ_IR
    }
    LinePOS = 權重和 / 紅外線和
    // serial.writeValue("LinePOS", LinePOS)  // 想用「Show data 裝置」看位置就打開這行
}

// ---- PD 控制：算出左右輪速度 ----
function PD(KP: number, KD: number) {
    pError = 3 - LinePOS
    dError = pError - pErrorOld
    pErrorOld = pError
    PDvalue = KP * pError + KD * dError
    SmartCar.setMotor(SmartCar.MotorList.左輪, Math.constrain(bassSPD - PDvalue, -1000, 1000))
    SmartCar.setMotor(SmartCar.MotorList.右輪, Math.constrain(bassSPD + PDvalue, -1000, 1000))
}

// ---- 起終點偵測：用兩側 IR-L(5) / IR-R(6) 數側邊白色標記，右側數到 2 次就停 ----
function 起終點() {
    SmartCar.setLED(SmartCar.LEDList.兩邊, false)
    if (校正值[0] > 3000 && 校正值[4] > 3000) {
        路口狀態 = 1
    } else if (校正值[5] > 3000 && 校正值[6] > 3000 && 路口狀態 == 1) {
        SmartCar.setLED(SmartCar.LEDList.兩邊, true)
    } else {
        if (校正值[5] > 3000) {
            左提示符號狀態 = 1
        } else if (校正值[5] < 3000 && 左提示符號狀態 == 1) {
            左提示符號次數 += 1
            左提示符號狀態 = 0
            SmartCar.setLED(SmartCar.LEDList.左邊, true)
        }
        if (校正值[6] > 3000) {
            右提示符號狀態 = 1
        } else if (校正值[6] < 3000 && 右提示符號狀態 == 1) {
            右提示符號次數 += 1
            右提示符號狀態 = 0
            SmartCar.setLED(SmartCar.LEDList.右邊, true)
        }
        if (右提示符號次數 == 2) {
            校正模式 = 3
        }
    }
}

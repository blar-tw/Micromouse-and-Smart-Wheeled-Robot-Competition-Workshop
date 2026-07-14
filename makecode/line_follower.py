# BitRacer Pro Max 循線程式 (MakeCode Python / micro:bit)
# 與 line_follower.ts 相同邏輯的 Python 版
#
# 怎麼用：
#   1. MakeCode 專案先加好擴展 https://github.com/CorgiQQQ/BitRacer_Pro_Max
#   2. 上方切到「Python」分頁，把整份貼進去
#   3. 想看積木就切回「積木」分頁（會自動轉換）
#   4. 連接 micro:bit → 下載
#
# 操作：
#   按 A   → 校正（車子前進取樣紅外線最大最小值）
#   按 B   → 開始循線
#   按 A+B → 停車
#
# 注意：這是 MakeCode Python（不是 MicroPython），只能貼在 MakeCode 網站裡用。
#       在 VS Code 本機看到紅字是正常的（SmartCar/basic/input 定義在 MakeCode 裡）。
#       如果某個名稱在 MakeCode 裡紅掉，以「JavaScript 分頁貼 .ts → 切 Python 分頁」
#       自動轉換出來的名稱為準。

校正模式 = 0
校正值 = [0, 0, 0, 0, 0, 0, 0]
紅外線值 = [0, 0, 0, 0, 0, 0, 0]
最大值 = [0, 0, 0, 0, 0, 0, 0]
最小值 = [5000, 5000, 5000, 5000, 5000, 5000, 5000]
目標最大值 = 4000
目標最小值 = 1
bassSPD = 350
LinePOS = 0
pError = 0
pErrorOld = 0
dError = 0
PDvalue = 0
NMZ_IR = 0
路口狀態 = 0
左提示符號狀態 = 0
右提示符號狀態 = 0
左提示符號次數 = 0
右提示符號次數 = 0

serial.redirect_to_usb()


# ---- 讀取七顆紅外線 ----
def 紅外線取樣():
    global NMZ_IR
    for index in range(7):
        NMZ_IR = SmartCar.get_ir(index)
        紅外線值[index] = NMZ_IR


# ---- 記錄每顆的最大 / 最小值（校正用）----
def 最大最小值():
    global NMZ_IR
    for index4 in range(7):
        NMZ_IR = 紅外線值[index4]
        if NMZ_IR < 最小值[index4]:
            最小值[index4] = NMZ_IR
        if NMZ_IR > 最大值[index4]:
            最大值[index4] = NMZ_IR


# ---- 正規化：把每顆感測器對應到 目標最小值~目標最大值 ----
def 正規化():
    global NMZ_IR
    for index2 in range(7):
        NMZ_IR = 紅外線值[index2]
        NMZ_IRmax = 最大值[index2]
        NMZ_IRmin = 最小值[index2]
        NMZ_IRadj = Math.map(NMZ_IR, NMZ_IRmin, NMZ_IRmax, 目標最小值, 目標最大值)
        校正值[index2] = Math.constrain(NMZ_IRadj, 目標最小值, 目標最大值) + 200


# ---- 權重：用 IR-1~IR-5 算出賽道位置 LinePOS（中心 = 3）----
def 權重():
    global NMZ_IR, LinePOS
    權重和 = 0
    紅外線和 = 0
    for index3 in range(5):
        NMZ_IR = 校正值[index3]
        權重和 += NMZ_IR * (index3 + 1)
        紅外線和 += NMZ_IR
    LinePOS = 權重和 / 紅外線和
    # serial.write_value("LinePOS", LinePOS)  # 想用「Show data 裝置」看位置就打開這行


# ---- PD 控制：算出左右輪速度 ----
def PD(KP, KD):
    global pError, dError, pErrorOld, PDvalue
    pError = 3 - LinePOS
    dError = pError - pErrorOld
    pErrorOld = pError
    PDvalue = KP * pError + KD * dError
    SmartCar.set_motor(SmartCar.MotorList.左輪,
                       Math.constrain(bassSPD - PDvalue, -1000, 1000))
    SmartCar.set_motor(SmartCar.MotorList.右輪,
                       Math.constrain(bassSPD + PDvalue, -1000, 1000))


# ---- 起終點偵測：右側白色標記數到 2 次就停 ----
def 起終點():
    global 路口狀態, 左提示符號狀態, 左提示符號次數
    global 右提示符號狀態, 右提示符號次數, 校正模式
    SmartCar.set_led(SmartCar.LEDList.兩邊, False)
    if 校正值[0] > 3000 and 校正值[4] > 3000:
        路口狀態 = 1
    elif 校正值[5] > 3000 and 校正值[6] > 3000 and 路口狀態 == 1:
        SmartCar.set_led(SmartCar.LEDList.兩邊, True)
    else:
        if 校正值[5] > 3000:
            左提示符號狀態 = 1
        elif 校正值[5] < 3000 and 左提示符號狀態 == 1:
            左提示符號次數 += 1
            左提示符號狀態 = 0
            SmartCar.set_led(SmartCar.LEDList.左邊, True)
        if 校正值[6] > 3000:
            右提示符號狀態 = 1
        elif 校正值[6] < 3000 and 右提示符號狀態 == 1:
            右提示符號次數 += 1
            右提示符號狀態 = 0
            SmartCar.set_led(SmartCar.LEDList.右邊, True)
        if 右提示符號次數 == 2:
            校正模式 = 3


# ---- 按鈕 ----
def on_button_pressed_a():
    global 校正模式
    校正模式 = 1
    SmartCar.set_motor(SmartCar.MotorList.雙輪, 350)
    basic.pause(300)
    SmartCar.set_motor(SmartCar.MotorList.雙輪, 0)
input.on_button_pressed(Button.A, on_button_pressed_a)


def on_button_pressed_b():
    global 校正模式
    校正模式 = 2
input.on_button_pressed(Button.B, on_button_pressed_b)


def on_button_pressed_ab():
    global 校正模式
    校正模式 = 3
input.on_button_pressed(Button.AB, on_button_pressed_ab)


# ---- 主迴圈 ----
def on_forever():
    if 校正模式 == 1:
        SmartCar.set_ir_power(True)
        紅外線取樣()
        最大最小值()
    elif 校正模式 == 2:
        SmartCar.set_ir_power(True)
        紅外線取樣()
        正規化()
        權重()
        PD(250, 350)
        起終點()
    elif 校正模式 == 3:
        SmartCar.set_motor(SmartCar.MotorList.雙輪, 0)
basic.forever(on_forever)

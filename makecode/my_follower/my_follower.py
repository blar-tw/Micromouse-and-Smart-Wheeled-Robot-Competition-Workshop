# my_follower.py — 比賽風格循線演算法 (MakeCode Python / micro:bit)
# BitRacer Pro Max 用；跟研習版 line_follower.py 操作方式相同，演算法升級。
#
# 操作：A = 校正（推車或讓它前進，讓每顆感測器都掃過白線和黑底）
#       B = 開始循線（會重置計數器，可重複按）
#       A+B = 停車
#
# 相對研習版的升級（比賽常用技巧，出處見同資料夾 README.md）：
#   1. 一次 I2C 讀完 7 顆感測器（get_all_ir_values，1 次傳輸 vs 7 次）→ 控制迴圈更快
#   2. 位置刻度改成 -2000..+2000（Pololu readLine 慣例），解析度更細
#   3. 斷線記憶：完全看不到線時，回報「最後看到線的那一側」的滿刻度位置
#      → PD 自然把車頭甩回去找線（Pololu readLine 的招）
#   4. 完整 PID（I 項預設 0，附防積分飽和 anti-windup）
#   5. 自適應速度：誤差大（彎道）自動減速、直線全速 —— 快車的關鍵
#   6. D 項低通濾波：減少感測器雜訊造成的抖動
#   7. 斷線超過 LOST_TIMEOUT 毫秒自動停車（安全機制）
#   8. 側邊標記偵測加遲滯（hysteresis）：>600 算看到、<400 算離開，不會抖動誤計數
#
# 注意：這是 MakeCode Python，只能貼在 makecode.microbit.org 的 Python 分頁。
#       VS Code 本機顯示紅字是正常的。

# ================= 可調參數（全部集中在這） =================
KP = 0.25            # 比例增益：修正力道（等效老師版 KP=250；0.15~0.4 之間試）
KI = 0.0             # 積分增益：競速通常設 0；車子總是偏一邊才加一點點（如 0.001）
KD = 1.0             # 微分增益：阻尼（等效老師版 KD 的 3 倍；抖動就降、過彎甩尾就加）
                     # ⚠️ 這個刻度下 KD 每 +1 = 老師版 +1000，動一點點就差很多
SPEED_MAX = 300      # 直線基礎速度（先保守，確定不重開機再往上加）
SPEED_MIN = 180      # 彎道最低基礎速度
SPEED_LOST = 150     # 斷線找線時的速度
CORR_MAX = 500       # 單次修正上限：避免瞬間硬甩、電流爆衝把 micro:bit 拉重開
I_MAX = 200000       # 積分上限（anti-windup）
LINE_IS_WHITE = True # True = 黑底白線；白底黑線改 False
LINE_THRESHOLD = 200 # 歸一化 0~1000 後，多亮才算「看得到線」
LOST_TIMEOUT = 1200  # 斷線超過幾毫秒自動停車
FINISH_COUNT = 2     # 右側標記數到幾次停車（起點+終點 = 2）
# ============================================================

mode = 0                     # 0=待機 1=校正 2=循線 3=停車
raw = [0, 0, 0, 0, 0, 0, 0]
cal_min = [5000, 5000, 5000, 5000, 5000, 5000, 5000]
cal_max = [0, 0, 0, 0, 0, 0, 0]
norm = [0, 0, 0, 0, 0, 0, 0]   # 歸一化後 0~1000，1000 = 線正下方
pos = 0                      # 賽道位置 -2000(IR-1側) .. +2000(IR-5側)
last_pos = 0
last_error = 0
d_filt = 0
integral = 0
line_seen_time = 0
r_marker_on = 0
r_marker_count = 0
l_marker_on = 0

music.set_built_in_speaker_enabled(False)   # 消音（V2 喇叭）
serial.redirect_to_usb()


# 一次 I2C 讀 7 顆 → 歸一化到 0~1000（1000 = 線）
def read_sensors():
    global raw
    raw = SmartCar.get_all_ir_values()
    for i in range(7):
        span = cal_max[i] - cal_min[i]
        if span < 1:
            norm[i] = 0
        else:
            v = (raw[i] - cal_min[i]) * 1000 / span
            v = Math.constrain(v, 0, 1000)
            if LINE_IS_WHITE:
                norm[i] = v
            else:
                norm[i] = 1000 - v


# 加權平均算位置；完全沒看到線 → 回報最後那一側的滿刻度（斷線記憶）
def read_position():
    global pos, last_pos, line_seen_time
    read_sensors()
    total = 0
    weighted = 0
    on_line = False
    for i in range(5):                       # 前排 IR-1..IR-5 = norm[0..4]
        total += norm[i]
        weighted += norm[i] * (i - 2) * 1000  # 權重 -2000,-1000,0,1000,2000
        if norm[i] > LINE_THRESHOLD:
            on_line = True
    if on_line:
        pos = weighted / total
        last_pos = pos
        line_seen_time = input.running_time()
    else:
        # 斷線：往最後看到線的那一側滿刻度，讓 PD 把車甩回去
        if last_pos >= 0:
            pos = 2000
        else:
            pos = -2000
    return on_line


# 校正：記錄每顆感測器的最大最小值
def read_sensors_raw_calibrate():
    global raw
    raw = SmartCar.get_all_ir_values()
    for i in range(7):
        if raw[i] < cal_min[i]:
            cal_min[i] = raw[i]
        if raw[i] > cal_max[i]:
            cal_max[i] = raw[i]


def pid_drive(on_line):
    global last_error, integral, d_filt
    error = pos                              # 目標 = 0（線在正中央）
    if on_line:
        integral = Math.constrain(integral + error, -I_MAX, I_MAX)
        d_raw = error - last_error
        d_filt = d_filt * 0.6 + d_raw * 0.4  # D 項低通濾波
        last_error = error                   # 只在看得到線時更新，避免重新找到線時 D 爆衝
        # 自適應速度：偏差越大跑越慢（彎道減速、直線全速）
        base = SPEED_MAX - (SPEED_MAX - SPEED_MIN) * abs(error) / 2000
    else:
        d_filt = 0                           # 斷線瞬間 pos 會跳到 ±2000，關掉 D 避免馬達硬甩爆電流
        base = SPEED_LOST
    correction = KP * error + KI * integral + KD * d_filt
    correction = Math.constrain(correction, -CORR_MAX, CORR_MAX)
    SmartCar.set_motor(SmartCar.MotorList.左輪,
                       Math.constrain(base + correction, -1000, 1000))
    SmartCar.set_motor(SmartCar.MotorList.右輪,
                       Math.constrain(base - correction, -1000, 1000))


# 右側標記計數（含遲滯 + 路口忽略），數滿 FINISH_COUNT 次停車
def check_markers():
    global r_marker_on, r_marker_count, l_marker_on, mode
    # 路口：前排大部分都看到線 → 這是橫線不是標記，忽略側邊
    across = 0
    for i in range(5):
        if norm[i] > 600:
            across += 1
    if across >= 4:
        return
    # 左標記（只亮燈提示）
    if l_marker_on == 0 and norm[5] > 600:
        l_marker_on = 1
        SmartCar.set_led(SmartCar.LEDList.左邊, True)
    elif l_marker_on == 1 and norm[5] < 400:
        l_marker_on = 0
        SmartCar.set_led(SmartCar.LEDList.左邊, False)
    # 右標記（計數 → 停車）
    if r_marker_on == 0 and norm[6] > 600:
        r_marker_on = 1
        SmartCar.set_led(SmartCar.LEDList.右邊, True)
    elif r_marker_on == 1 and norm[6] < 400:
        r_marker_on = 0
        r_marker_count += 1
        SmartCar.set_led(SmartCar.LEDList.右邊, False)
        if r_marker_count >= FINISH_COUNT:
            mode = 3


def on_button_pressed_a():
    global mode
    mode = 1
    SmartCar.set_ir_power(True)
    SmartCar.set_motor(SmartCar.MotorList.雙輪, 350)
    basic.pause(300)
    SmartCar.set_motor(SmartCar.MotorList.雙輪, 0)
input.on_button_pressed(Button.A, on_button_pressed_a)


def on_button_pressed_b():
    global mode, integral, last_error, d_filt, r_marker_count, line_seen_time
    # 重新開跑前歸零（研習版沒做，導致第二次按 B 會直接停車）
    integral = 0
    last_error = 0
    d_filt = 0
    r_marker_count = 0
    line_seen_time = input.running_time()
    mode = 2
input.on_button_pressed(Button.B, on_button_pressed_b)


def on_button_pressed_ab():
    global mode
    mode = 3
input.on_button_pressed(Button.AB, on_button_pressed_ab)


def on_forever():
    global mode
    if mode == 1:
        # 校正：記錄每顆的最大最小值（推車讓每顆都掃過線和底）
        SmartCar.set_ir_power(True)
        read_sensors_raw_calibrate()
    elif mode == 2:
        SmartCar.set_ir_power(True)
        on_line = read_position()
        pid_drive(on_line)
        check_markers()
        # 斷線太久 → 安全停車
        if input.running_time() - line_seen_time > LOST_TIMEOUT:
            mode = 3
        # serial.write_value("pos", pos)   # 調參數時打開，用 Show data 看
    elif mode == 3:
        SmartCar.set_motor(SmartCar.MotorList.雙輪, 0)
basic.forever(on_forever)

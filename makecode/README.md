# MakeCode 資源

## 擴展積木

BitRacer Pro Max 專屬擴展積木（簡化 micro:bit 與車體 MCU 的 I2C 通訊）：

- GitHub：<https://github.com/CorgiQQQ/BitRacer_Pro_Max>
- 在 MakeCode「擴展」中貼上上面的網址即可加入積木

提供的積木（感測器控制）：

- 紅外線開關 on/off
- 讀取第 index 顆感測器
- 一次讀取七顆紅外線
- 設定 LED 狀態
- 讀取超音波距離 (cm)
- 設定馬達（左輪/右輪/雙輪）速度

## 範例程式

- 停車程式：<https://makecode.microbit.org/S92273-39996-50243-0737>

## 循線程式流程（研習範例）

1. **按鈕 A**：校正模式 1 — 車體前進取樣，記錄各感測器最大/最小值
2. **按鈕 B**：校正模式 2 — 開始循線（紅外線取樣 → 正規化 → 權重 → PD(250, 350) → 起終點判斷）
3. **按鈕 A+B**：校正模式 3 — 停車（雙輪速度 0）

PD 函式核心（KP、KD 為參數）：

```
pError  = 3 − LinePOS
dError  = pError − pErrorOld
PDvalue = KP × pError + KD × dError
左輪速度 = 制限(bassSPD − PDvalue, −1000, 1000)
右輪速度 = 制限(bassSPD + PDvalue, −1000, 1000)
```

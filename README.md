# Micromouse & Smart Wheeled Robot Competition Workshop

2026 TMIRC 暑期營 — 雙平台輪型機器人設計與應用（BitRacer Pro Max）

指導老師：蘇景暉、李炳輝｜BitRacer Pro Max 開發者：鄭力瑋（龍華科大）

## 簡介

BitRacer Pro Max 是針對 108 課綱與 STEM 教育設計的教學輪型機器人，
採用「雙平台開發架構」：

- **MakeCode（圖形化積木）** — 適合入門學習者，micro:bit 作為主控端，透過 I2C 與車體 MCU（ATSAMD21G18）通訊
- **Arduino IDE（文字程式）** — 直接開發車上的 ATSAMD21G18，適合進階學習者

## 硬體規格

| 項目 | 說明 |
|------|------|
| 微控制器 | ATSAMD21G18（車體）＋ micro:bit（上層主控） |
| 馬達 | 直流馬達 ×2，TB6612FNG 驅動，四輪齒輪傳動 |
| 感測器 | 紅外線反射感測器（VCNT2020）×5＋左右擴展 ×2 |
| 測距 | HC-SR04P 超音波模組（擴充插槽） |
| 編碼器 | 磁式編碼器（左右輪） |
| 顯示 | 1.8 吋 TFT LCD（128×160, SPI） |
| 其他 | 無源蜂鳴器、LED ×2、按鈕 ×2 |
| 電源 | 14500 鋰電池，TP4056 充電 IC，MCP1826 LDO（3.3V） |

## 儲存庫結構

```
├── slides/      研習簡報 PDF
├── docs/        硬體說明與 I2C 命令表
├── arduino/     Arduino IDE 循線範例程式（PD 控制）
└── makecode/    MakeCode 擴展積木與範例連結
```

## 快速連結

- MakeCode 擴展積木（BitRacer Pro Max）：<https://github.com/CorgiQQQ/BitRacer_Pro_Max>
- MakeCode 停車範例程式：<https://makecode.microbit.org/S92273-39996-50243-0737>
- MakeCode 開發環境：<https://makecode.microbit.org/>

## 課程單元（MakeCode 積木單元化設計）

1. 發展環境 & 擴展積木
2. 紅外線數值 & 正規化（歸一化）
3. 賽道位置數據估測（加權平均）
4. 比例差分（PD）回授控制
5. 超音波距離監控
6. 編碼器 & LCD 螢幕顯示

# CLRD125C 通道参数调试指南

更新时间：2026-06-26

## 1. 当前状态

当前工程已经可以正常通过 SMBus 写入 4 片 CLRD125C。

当前实际启用的配置表是：

```c
#define CLRD125C_ACTIVE_CONFIG      g_clrd125cXazxIniConfig
#define CLRD125C_ACTIVE_CONFIG_NAME "125C-XAZX.ini"
```

也就是当前写入的是 `125C-XAZX.ini` 原始配置。

当前 4 片地址：

| dev | 7-bit 地址 | 8-bit 写地址 |
| --- | --- | --- |
| dev0 | 0x58 | 0xB0 |
| dev1 | 0x5C | 0xB8 |
| dev2 | 0x60 | 0xC0 |
| dev3 | 0x64 | 0xC8 |

目前通过插拔/回环状态初步确认：

| SFP | CLRD125C | 通道 |
| --- | --- | --- |
| SFP2 | dev1 / 0x5C | CH4 |
| SFP3 | dev1 / 0x5C | CH6 |

回环插入后，日志显示：

```text
LOS_val[7:0]=11110011
SFP_OK[7:0]=00001100
dev1 reg70=0xFF
focus(CH1=1 CH3=1 CH4=1 CH6=1)
```

说明 SFP2、SFP3 板级 LOS 消失，dev1 的重点通道也检测到信号。

## 2. 状态打印怎么看

当前 5 秒打印一次链路状态：

```text
[LINK-STAT] LOS_val[7:0]=11110011 LOS=0xF3 SFP_OK[7:0]=00001100
[CLRD125C-CH] dev1 0x5C reg70=0xFF CH[7:0]=11111111 focus(CH1=1 CH3=1 CH4=1 CH6=1)
```

含义：

| 字段 | 含义 |
| --- | --- |
| `LOS_val[7:0]` | MCU 直接读取的板级 SFP LOS 原始值 |
| `SFP_OK[7:0]` | 对 `LOS_val` 取反后的直观显示，1 表示该 SFP 有光 |
| `reg70` | CLRD125C D[70]/0xC6，内部 `det_losb<7:0>` |
| `CH[7:0]` | CLRD125C CH7~CH0 的 losb 状态 |
| `focus(...)` | 当前重点关注的 CH1/CH3/CH4/CH6 状态 |

极性：

```text
板级 LOS：0 = OK，1 = LOS
CLRD125C losb：1 = OK，0 = LOS
```

## 3. 当前配置效果

当前 `125C-XAZX.ini` 配置偏保守，适合先确认链路是否可通。

dev1 重点通道当前参数：

| 通道 | 寄存器 | 当前值 | 作用 |
| --- | --- | --- | --- |
| EN_HF | D[7] / 0x87 | 0x00 | 高频增强关闭 |
| CH1 CTLE | D[22] / 0x96 | 0x08 | CH1 均衡 |
| CH1 MODE/VOD | D[23] / 0x97 | 0x05 | CH1 模式/摆幅 |
| CH1 DEM | D[24] / 0x98 | 0x05 | CH1 去加重 |
| CH3 CTLE | D[36] / 0xA4 | 0x08 | CH3 均衡 |
| CH3 MODE/VOD | D[37] / 0xA5 | 0x05 | CH3 模式/摆幅 |
| CH3 DEM | D[38] / 0xA6 | 0x05 | CH3 去加重 |
| CH4 CTLE | D[44] / 0xAC | 0x04 | CH4 均衡，当前关联 SFP2 |
| CH4 MODE/VOD | D[45] / 0xAD | 0x05 | CH4 模式/摆幅 |
| CH4 DEM | D[46] / 0xAE | 0x05 | CH4 去加重 |
| CH6 CTLE | D[58] / 0xBA | 0x04 | CH6 均衡，当前关联 SFP3 |
| CH6 MODE/VOD | D[59] / 0xBB | 0x05 | CH6 模式/摆幅 |
| CH6 DEM | D[60] / 0xBC | 0x05 | CH6 去加重 |
| Route | D[79] / 0xCF | 0x50 | 当前路由，Fanout 关闭 |

## 4. 如何修改配置

当前有两份配置表：

```c
static const uint8_t g_clrd125cFullConfig[CLRD125C_REG_NUM]
const uint8_t g_clrd125cXazxIniConfig[CLRD125C_REG_NUM]
```

当前实际写入由这个宏决定：

```c
#define CLRD125C_ACTIVE_CONFIG g_clrd125cXazxIniConfig
```

如果要切到 LT1258 等效配置：

```c
#define CLRD125C_ACTIVE_CONFIG g_clrd125cFullConfig
```

如果只想调某个参数，建议直接改当前激活表中的对应值。

数组下标和寄存器关系：

```text
D[n] = 数组下标 n
线上寄存器地址 = 0x80 + n
```

例如：

```text
D[44] -> 0xAC -> CH4 CTLE
D[58] -> 0xBA -> CH6 CTLE
```

## 5. 推荐调试顺序

不要一次改很多参数。推荐顺序如下：

### 第一步：保持当前配置，先看 FPGA 是否 link

当前回环下已经满足：

```text
SFP2 OK
SFP3 OK
dev1 CH4 OK
dev1 CH6 OK
```

因此先确认：

```text
FPGA 是否 link
BER 是否为 0
是否有链路抖动或反复掉线
```

### 第二步：如果不 link，先只调 CH4/CH6 的 CTLE

优先只改：

```text
CH4 CTLE: D[44]
CH6 CTLE: D[58]
```

建议测试值：

| 阶段 | D[44] CH4 CTLE | D[58] CH6 CTLE |
| --- | --- | --- |
| 当前 | 0x04 | 0x04 |
| 测试 1 | 0x06 | 0x06 |
| 测试 2 | 0x08 | 0x08 |
| 测试 3 | 0x09 | 0x09 |

每改一次，只观察：

```text
FPGA link
BER
眼图
LOS/CH状态
```

### 第三步：再考虑 VOD

如果 CTLE 有改善但仍不稳定，再调：

```text
CH4 MODE/VOD: D[45]
CH6 MODE/VOD: D[59]
```

当前值为：

```text
D[45] = 0x05
D[59] = 0x05
```

LT1258 等效值为：

```text
D[45] = 0x42
D[59] = 0x42
```

建议不要一开始就改到 0x42，除非需要做 LT1258 对比。

### 第四步：最后调 DEM

最后再调：

```text
CH4 DEM: D[46]
CH6 DEM: D[60]
```

当前值：

```text
D[46] = 0x05
D[60] = 0x05
```

LT1258 等效值：

```text
D[46] = 0x02
D[60] = 0x02
```

## 6. LT1258 等效配置

如果目标是尽量复现原 LT1258 效果，可以切到或参考以下值：

| 功能 | 寄存器 | LT1258 等效值 |
| --- | --- | --- |
| EN_HF | D[7] / 0x87 | 0x5A |
| CH1 CTLE | D[22] / 0x96 | 0x09 |
| CH1 MODE/VOD | D[23] / 0x97 | 0x42 |
| CH1 DEM | D[24] / 0x98 | 0x02 |
| CH3 CTLE | D[36] / 0xA4 | 0x09 |
| CH3 MODE/VOD | D[37] / 0xA5 | 0x42 |
| CH3 DEM | D[38] / 0xA6 | 0x02 |
| CH4 CTLE | D[44] / 0xAC | 0x09 |
| CH4 MODE/VOD | D[45] / 0xAD | 0x42 |
| CH4 DEM | D[46] / 0xAE | 0x02 |
| CH6 CTLE | D[58] / 0xBA | 0x09 |
| CH6 MODE/VOD | D[59] / 0xBB | 0x42 |
| CH6 DEM | D[60] / 0xBC | 0x02 |
| Route | D[79] / 0xCF | 0x50 |

## 7. 每次调试记录建议

每次只记录一组变化：

```text
日期时间：
配置表：
修改寄存器：
修改前：
修改后：
SFP_OK：
dev/ch：
FPGA link：
BER：
眼图现象：
结论：
```

这样后面才能判断是哪一个参数真正起作用。


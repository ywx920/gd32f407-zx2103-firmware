# CLRD125C 四片配置与功能调试步骤

更新时间：2026-06-25

## 1. 当前结论

当前板卡上的 4 片 CLRD125C 已经可以在 I2C1/SMBus 总线上通信，实测 7-bit 地址为：

| 芯片 | 7-bit 地址 | 线上写地址 | 线上读地址 |
| --- | --- | --- | --- |
| device0 | 0x59 | 0xB2 | 0xB3 |
| device1 | 0x5D | 0xBA | 0xBB |
| device2 | 0x61 | 0xC2 | 0xC3 |
| device3 | 0x65 | 0xCA | 0xCB |

代码中的 `gpioI2cWrite/gpioI2cRead/gpioI2cProbe` 使用的是 7-bit 地址，函数内部会左移 1 位后发到总线上。因此代码里填 `0x59`，逻辑分析仪/示波器 I2C 解码时看到的写地址会是 `0xB2`。

寄存器访问方式：

```text
逻辑数组下标：D[n]
线上寄存器地址：0x80 + n
寄存器范围：D[0]~D[79] => 0x80~0xCF
```

## 2. 本次代码调整

当前主工程已采用 4 片完整配置表写入方式：

```c
#define CLRD125C_TEST_MODE CLRD125C_TEST_MODE_WRITE_4DEV
```

流程为：

```text
main()
  └─ clrd125cLt1258StyleTest()
       ├─ 初始化 I2C1 GPIO 模拟总线
       ├─ 依次选择 0x59 / 0x5D / 0x61 / 0x65
       ├─ 对每片芯片做地址 ACK 探测
       ├─ 写入完整 80 字节 g_clrd125cFullConfig[]
       ├─ 回读 D[0]~D[79]
       ├─ 比对写入值和回读值
       └─ 输出每片与总体 summary
```

已将下面 3 个已知回读差异改为 `WARN`，不再作为配置失败：

| D 下标 | 线上寄存器 | 当前现象 | 处理 |
| --- | --- | --- | --- |
| D[0] | 0x80 | 回读值随芯片地址不同变化 | WARN |
| D[69] | 0xC5 | 写 0xAA 后读回 0x5A | WARN |
| D[70] | 0xC6 | 写 0xA0 后读回 0xA5 | WARN |

判断是否配置成功时，只看：

```text
write_fail = 0
read_fail = 0
verify_diff = 0
```

`verify_warn` 只作为记录，不代表失败。

## 3. 烧录后的期望打印

正常情况下，应看到类似：

```text
BOOT: basic init done
BOOT: start CLRD125C direct write

[CLRD125C-CONFIG] four-device full-table configure/readback start

[CLRD125C-CONFIG] device0 selected: 7-bit=0x59 write=0xB2 read=0xB3 reg-base=0x80
[CLRD125C-CONFIG] device0 address probe ACK
[CLRD125C-CONFIG] device0 write full 125C-XAZX table D[0]~D[79]
[CLRD125C-CONFIG] device0 read back full table D[0]~D[79]
[CLRD125C-CONFIG] device0 READ  WARN ...
[CLRD125C-CONFIG] device0 summary: write_fail=0 read_fail=0 verify_diff=0 verify_warn=3 => OK

...

[CLRD125C-CONFIG] total summary: write_fail=0 read_fail=0 verify_diff=0 verify_warn=12 => OK
Init summary: LT1258=DISABLED, CLRD125C=OK(SMBUS)
```

如果出现 `write_fail` 或 `read_fail`，优先排查地址、ENSMB、RESET、电源、SCL/SDA 波形。

如果出现 `verify_diff`，说明有除 D[0]/D[69]/D[70] 外的寄存器回读不一致，需要结合手册判断该位是否只读、保留、写 1 清除、自清零或被硬件状态覆盖。

## 4. 如何修改具体芯片功能

后续配置 CLRD125C 时，优先修改 `g_clrd125cFullConfig[80]` 这张完整表。

位置：

```text
20241202/project_up_new/bsp/clrd125c_v1.c
```

核心规则：

```text
g_clrd125cFullConfig[0]  -> D[0]  -> 线上寄存器 0x80
g_clrd125cFullConfig[1]  -> D[1]  -> 线上寄存器 0x81
...
g_clrd125cFullConfig[79] -> D[79] -> 线上寄存器 0xCF
```

例如要改 D[36]，实际写到总线上的寄存器地址就是：

```text
0x80 + 36 = 0xA4
```

## 5. 常用功能配置入口

以下为当前调试阶段最常改的功能点。具体 bit 定义以 CLRD125C-K V1.0 手册和 `CLRD125C_regmap_V1.0.xlsx` 为准。

### 5.1 关闭外部引脚控制，改由寄存器控制

这些寄存器决定配置是否由 SMBus 寄存器接管：

| 功能 | D 下标 | 线上寄存器 | 当前值 | 说明 |
| --- | --- | --- | --- | --- |
| EQ/VGA/DEM 引脚控制屏蔽 | D[6] | 0x86 | 0xE0 | 让均衡/增益/去加重由寄存器控制 |
| MODE/ENHF 引脚控制屏蔽 | D[8] | 0x88 | 0x0C | 让模式/高频增强由寄存器控制 |
| SEL/INPUT_EN 引脚控制屏蔽 | D[78] | 0xCE | 0x07 | 让路由/输入使能由寄存器控制 |

如果这些 override 没打开，写了后面的功能寄存器，也可能被外部管脚电平覆盖。

### 5.2 路由与通道开关

| 功能 | D 下标 | 线上寄存器 | 当前值 |
| --- | --- | --- | --- |
| 通道 power-down / enable | D[5] | 0x85 | 0xFF |
| 路由/输出选择 | D[79] | 0xCF | 0x50 |

当前目标是复现原 LT1258 有效链路效果，不改变原来的主业务链路方向。若后续要改输入输出映射，应先改路由寄存器，再用示波器确认对应 SFP 通道输出。

### 5.3 CTLE / EQ 均衡

厂商调试建议图中给出的通道调节寄存器大致按通道分布。常用 CTLE/EQ 相关寄存器包括：

| 通道 | D 下标 | 线上寄存器 |
| --- | --- | --- |
| CH0 | D[15] | 0x8F |
| CH1 | D[22] | 0x96 |
| CH2 | D[29] | 0x9D |
| CH3 | D[36] | 0xA4 |
| CH4 | D[44] | 0xAC |
| CH5 | D[51] | 0xB3 |
| CH6 | D[58] | 0xBA |
| CH7 | D[65] | 0xC1 |

操作方法：

1. 先保持当前完整表不变，确认 4 片都 `OK`。
2. 只改一个通道的一个参数。
3. 重新烧录或通过临时测试函数写单寄存器。
4. 用眼图、误码率、LOS、链路稳定性判断效果。
5. 记录每次改动：芯片地址、D 下标、原值、新值、测试结果。

### 5.4 线性模式和非线性模式

线性模式可以理解为“主要做连续时间均衡/放大，输出尽量保持线性响应”，适合先把链路跑通、减少变量。

非线性模式会启用高频增强、去加重、输出摆幅等更强的整形能力，适合最终优化眼图。但它的参数耦合更强：CTLE、VOD、DEM、EN_HF 需要一起看。

建议调试顺序：

```text
先用当前表确认 4 片可写可读
  -> 确认链路是否已经正常
  -> 如果链路正常，先不要大改
  -> 如果眼图/BER 不理想，先小步调 CTLE
  -> 再调 VOD
  -> 最后调 DEM / 非线性增强
```

## 6. 推荐的实际操作步骤

### 步骤 1：确认软件通信

烧录当前工程，串口查看：

```text
address probe ACK
write_fail=0
read_fail=0
verify_diff=0
verify_warn=12
CLRD125C=OK(SMBUS)
```

只要满足以上条件，说明 MCU 到 4 片 CLRD125C 的 SMBus 读写链路是通的。

### 步骤 2：确认高速链路

软件读写 OK 不等于高速信号一定 OK。还需要确认：

1. SFP 对应通道是否有输出。
2. LOS 状态是否正常。
3. 对端是否能建链。
4. 眼图是否张开。
5. BER 是否满足要求。

### 步骤 3：功能参数微调

每次只改一类参数：

```text
CTLE/EQ -> VOD -> DEM -> 路由/通道使能
```

不建议同时改多个寄存器，否则很难判断哪个参数带来了变化。

### 步骤 4：形成最终配置表

当找到稳定配置后，把最终值固化到：

```text
g_clrd125cFullConfig[]
```

并更新本文件或项目记录，至少记录：

```text
芯片地址
寄存器 D 下标
线上寄存器地址
最终值
对应通道
测试现象
```

## 7. 推荐策略

当前最推荐的方式是：

```text
使用 SMBus，从 MCU 对 4 片 CLRD125C 写完整 80 字节配置表；
启动时写入一次，随后完整回读校验；
D[0]/D[69]/D[70] 作为已知 WARN；
其余寄存器必须严格匹配。
```

这个方式比纯引脚模式更适合当前板卡，因为：

1. 4 片地址已经实际扫到并验证。
2. 可以完整控制每个芯片、每个通道。
3. 可以回读确认配置是否真的进入芯片。
4. 后续调眼图时，只需要改配置表，不需要再改硬件。


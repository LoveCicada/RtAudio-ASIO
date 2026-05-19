# RtAudio + ASIO SDK：LTC 时间码输出方案

## 业务场景

播控服务器安装 **Dante Virtual Soundcard (DVS)**，在 Windows 音频设置中将 ASIO 作为 Audio Interface。本机通过 ASIO 播放 **LTC WAV** 时间码文件，经网线由音响端 **Q-SYS** 选取 Dante 音频流并解析时间码。

```
播控服务器 (LtcAsioPlayer)
    -> RtAudio (WINDOWS_ASIO)
    -> Steinberg ASIO SDK Host
    -> Dante Virtual Soundcard
    -> Dante 网络
    -> Q-SYS (LTC 解析)
```

## 为何不用 SDL2

SDL2 在 Windows 上通常走 DirectSound/WASAPI，**无法选择 ASIO 设备**，不能满足「指定 DVS ASIO 输出」的需求。

## 技术选型

| 组件 | 版本/说明 |
|------|-----------|
| RtAudio | 6.0.1（Release） |
| Steinberg ASIO SDK | 2.3.4（外部 `third_party/ASIO`） |
| Qt | 5.12.12（Widgets） |
| C++ | C++11 |
| 构建 | CMake 3.16+，MSVC x64 |

## Demo 程序

工程根目录下的 **LtcAsioPlayer**：

- 列举 ASIO 输出设备（含 DVS）
- 加载 16-bit PCM WAV（单声道/立体声）
- 选择输出通道偏移后播放（支持循环，适合 LTC）

## 相关文档

- [setup.md](setup.md) — 依赖与 SDK 配置
- [build.md](build.md) — 编译与部署
- [development-plan.md](development-plan.md) — 后续集成播控的计划

## 参考工程

同仓库外已有直连 ASIO SDK 的探测 Demo：`E:\ownCode\ASIO\ASIO`（AsioProbeQt）。本工程使用 **RtAudio 抽象层**，便于后续统一音频接口与错误处理。

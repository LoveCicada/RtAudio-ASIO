# LtcAsioPlayer — RtAudio ASIO LTC 播放 Demo

通过 **RtAudio** 选择 **ASIO** 设备（如 Dante Virtual Soundcard）播放 **LTC WAV** 时间码。

## 快速开始

```powershell
.\scripts\setup-third-party.ps1

cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/soft/Qt5.12.12/5.12.12/msvc2017_64"

cmake --build build --config Release
```

详细说明见 [ASIO/build.md](ASIO/build.md)。

## 文档

- [ASIO/README.md](ASIO/README.md) — 方案概述
- [ASIO/setup.md](ASIO/setup.md) — 依赖配置
- [ASIO/build.md](ASIO/build.md) — 编译运行
- [ASIO/development-plan.md](ASIO/development-plan.md) — 后续计划

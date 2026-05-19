# 环境与第三方依赖配置

## 前置条件

- Windows 10/11 x64
- Visual Studio 2019/2022/2026（x64 工具链）
- CMake 3.16+
- Qt 5.12.12（MSVC 2017 64-bit 套件）
- Dante Virtual Soundcard（实机验证时）

本机 Qt 路径示例：`D:/soft/Qt5.12.12/5.12.12/msvc2017_64`

## 一键准备 third_party

在工程根目录执行：

```powershell
.\scripts\setup-third-party.ps1
```

默认从以下路径复制：

| 组件 | 默认源路径 |
|------|------------|
| RtAudio 6.0.1 | `E:\opensource\RtAudio\rtaudio-6.0.1` |
| ASIO SDK 2.3.4 | `E:\ownCode\ASIO\ASIO-SDK\ASIO-SDK_2.3.4_2025-10-15\ASIOSDK` |

复制结果：

```
third_party/
  rtaudio/     # RtAudio 源码（已打 ASIO 外部 SDK 补丁）
  ASIO/        # Steinberg SDK（common/, host/, host/pc/）
```

自定义源路径：

```powershell
.\scripts\setup-third-party.ps1 `
  -RtAudioSource "D:\path\rtaudio-6.0.1" `
  -AsioSdkSource "D:\path\ASIOSDK"
```

## RtAudio 与外部 ASIO SDK 的集成

RtAudio 6.0.1 默认编译内嵌在 `include/` 下的旧版 SDK。本工程已修补 `third_party/rtaudio/CMakeLists.txt`：

- CMake 变量：`RTAUDIO_ASIO_SDK_ROOT`（默认 `third_party/ASIO`）
- 编译 Steinberg 的 `asio.cpp`、`asiodrivers.cpp`、`asiolist.cpp`
- 仍使用 RtAudio 自带的 `include/iasiothiscallresolver.cpp`

### 本地补丁说明

1. **`third_party/ASIO/common/asio.cpp`**：`ASIOExit()` 在调用 `asioDrivers->removeCurrentDriver()` 前增加空指针判断（与 RtAudio 内嵌旧版 SDK 一致）。
2. **`third_party/rtaudio/RtAudio.cpp`**：在 `RtApiAsio` 构造函数中设置 `asioDrivers = &drivers`，使 Steinberg `ASIOExit()` 与 RtAudio 使用同一 `AsioDrivers` 实例。

## 许可证注意

- **RtAudio**：BSD 风格许可
- **Steinberg ASIO SDK 2.3.4**：专有 / GPLv3 双许可；**勿将完整 SDK 提交到公开仓库**
- `third_party/ASIO/` 已加入 `.gitignore`，每台机器本地复制或运行 setup 脚本

## DVS 使用注意

- DVS 需处于 Running 状态
- 避免被其他 DAW/宿主独占
- WAV 采样率须与 DVS 当前采样率一致（LTC 常用 **48000 Hz**）
- 应用需保持可见窗口（RtAudio ASIO 使用前台窗口句柄）

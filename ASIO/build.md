# 编译与运行

## 1. 准备依赖

```powershell
cd E:\ownCode\RtAudio-ASIO
.\scripts\setup-third-party.ps1
```

## 2. 配置 CMake

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/soft/Qt5.12.12/5.12.12/msvc2017_64" `
  -DRTAUDIO_ASIO_SDK_ROOT="E:/ownCode/RtAudio-ASIO/third_party/ASIO"
```

若使用 VS 2019，将生成器改为 `"Visual Studio 16 2019"`。

## 3. 编译

```powershell
cmake --build build --config Release
```

产物：`build\Release\LtcAsioPlayer.exe`

## 4. 部署 Qt 运行时

```powershell
& "D:/soft/Qt5.12.12/5.12.12/msvc2017_64/bin/windeployqt.exe" build\Release\LtcAsioPlayer.exe
```

## 5. 命令行验证（可选，先于 GUI）

```powershell
cmake -S third_party/rtaudio -B build-rtaudio-test -G "Visual Studio 18 2026" -A x64 `
  -DRTAUDIO_API_ASIO=ON -DRTAUDIO_API_WASAPI=OFF `
  -DRTAUDIO_BUILD_TESTING=ON -DRTAUDIO_BUILD_STATIC_LIBS=ON `
  -DRTAUDIO_ASIO_SDK_ROOT="E:/ownCode/RtAudio-ASIO/third_party/ASIO"

cmake --build build-rtaudio-test --config Release --target audioprobe
.\build-rtaudio-test\tests\Release\audioprobe.exe asio
```

## 6. 运行 Demo

1. 启动 Dante Virtual Soundcard
2. 运行 `build\Release\LtcAsioPlayer.exe`
3. 点击「刷新 ASIO 设备」，选择 **Dante Virtual Soundcard**
4. 「选择 LTC WAV」加载时间码文件
5. 设置「输出通道偏移」（单声道 LTC 通常为 0）
6. 点击「播放」

## 常见错误

| 现象 | 处理 |
|------|------|
| 设备列表为空 | 启动 DVS；关闭占用 ASIO 的 DAW；重启 DVS |
| `openStream` 失败 | WAV 采样率与 DVS 不一致；改为 48000 Hz |
| 编译 `asiolist` UNICODE 错误 | 确认 MSVC 使用 `/UUNICODE /U_UNICODE`（工程已配置） |
| 有设备但 probe 失败 | DVS 未运行或驱动被占用；查看日志区 RtAudio 警告 |
| 播放无声 | 调整「输出通道偏移」；确认 Q-SYS/Dante 路由到对应通道 |

## LTC WAV 格式要求

Demo 当前仅支持：

- RIFF WAVE
- PCM **16-bit**
- **单声道或立体声**

推荐使用 48000 Hz 单声道。其他格式可用 ffmpeg 转码：

```powershell
ffmpeg -i input.wav -ar 48000 -ac 1 -sample_fmt s16 output_ltc.wav
```

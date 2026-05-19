# 开发计划

## Phase 1：Demo 验证（当前）

- [x] RtAudio 6.0.1 + ASIO SDK 2.3.4 集成
- [x] CMake 静态链接 rtaudio（仅 ASIO API）
- [x] Qt 5.12 Demo：设备枚举、WAV 播放、循环
- [x] 文档：README / setup / build
- [ ] **实机**：播控服务器 + DVS + Q-SYS 端到端 LTC 锁定

### 实机验证步骤（播控服务器）

1. 确认 DVS Running，采样率 48000 Hz
2. 运行 `audioprobe.exe asio` 或 LtcAsioPlayer 刷新设备
3. 播放标准 LTC WAV，观察 DVS 电平
4. 在 Q-SYS 中选择对应 Dante 流，确认时间码锁定

## Phase 2：集成播控服务

- 将 `LtcWavPlayer` 抽离为无 Qt 依赖的 `LtcAsioEngine` 静态库
- 提供 C++ API：`load()` / `play()` / `stop()` / `setDevice()`
- 与现有播控业务逻辑对接（替换 SDL2 播放路径）
- 配置化：设备名、通道偏移、采样率、循环策略

## Phase 3：生产可靠性

- 设备热插拔 / DVS 重启后的自动重连
- 播放失败重试与日志上报
- 无人值守启动：默认设备名匹配 DVS
- 可选：多路 LTC 输出（受 ASIO 单流限制时需评估方案）

## 风险与对策

| 风险 | 对策 |
|------|------|
| ASIO 单流限制 | 单进程仅一路 LTC；多路需多进程或直连 SDK |
| 采样率不一致 | 启动前检测 DVS 采样率；WAV 预检 |
| SDK 许可 | 不提交 SDK 源码；部署文档说明许可选择 |
| HWND / 前台窗口 | 服务化时考虑隐藏窗口或改用直连 ASIO Host API |

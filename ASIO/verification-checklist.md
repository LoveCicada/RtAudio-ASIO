# 实机验证清单（播控服务器 + DVS + Q-SYS）

## 本机构建验证（已完成）

- [x] `audioprobe.exe asio` 编译通过，API 为 ASIO
- [x] 探测到 Dante 驱动名（需 DVS Running 才能成功初始化设备）
- [x] `LtcAsioPlayer.exe` Release 编译与 windeployqt 部署成功

## 播控服务器端到端（待现场执行）

- [ ] DVS 控制面板状态为 **Running**，采样率 **48000 Hz**
- [ ] `audioprobe.exe asio` 列出 **Dante Virtual Soundcard** 且 `Output Channels > 0`
- [ ] LtcAsioPlayer 刷新后可选中 DVS
- [ ] 加载 48000 Hz / 16-bit / mono 的 LTC WAV
- [ ] 播放时 DVS 输出电平有信号
- [ ] Q-SYS 选取对应 Dante 流，LTC 时间码锁定

## 记录模板

| 项目 | 结果 |
|------|------|
| 日期 | |
| DVS 版本 | |
| WAV 路径 / 采样率 | |
| 输出通道偏移 | |
| Q-SYS 是否锁定 | |

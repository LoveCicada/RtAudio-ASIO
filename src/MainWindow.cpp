#include "MainWindow.h"

#include <QtConcurrent/QtConcurrentRun>

#include <QCheckBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("LTC ASIO Player (RtAudio + Qt 5.12)"));
    resize(980, 640);

    m_player.setLogCallback([this](const std::string& text) {
        appendLog(QString::fromStdString(text));
    });

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* topRow = new QHBoxLayout();
    m_refreshButton = new QPushButton(QStringLiteral("刷新 ASIO 设备"), this);
    m_browseButton = new QPushButton(QStringLiteral("选择 LTC WAV"), this);
    m_playButton = new QPushButton(QStringLiteral("播放"), this);
    m_stopButton = new QPushButton(QStringLiteral("停止"), this);
    m_statusLabel = new QLabel(QStringLiteral("状态: 就绪"), this);

    topRow->addWidget(m_refreshButton);
    topRow->addWidget(m_browseButton);
    topRow->addWidget(m_playButton);
    topRow->addWidget(m_stopButton);
    topRow->addStretch(1);
    topRow->addWidget(m_statusLabel);

    auto* optionsRow = new QHBoxLayout();
    m_wavLabel = new QLabel(QStringLiteral("WAV: 未选择"), this);
    m_channelOffsetSpin = new QSpinBox(this);
    m_channelOffsetSpin->setMinimum(0);
    m_channelOffsetSpin->setMaximum(63);
    m_channelOffsetSpin->setPrefix(QStringLiteral("输出通道偏移: "));
    m_loopCheck = new QCheckBox(QStringLiteral("循环播放"), this);
    m_loopCheck->setChecked(true);

    optionsRow->addWidget(m_wavLabel, 1);
    optionsRow->addWidget(m_channelOffsetSpin);
    optionsRow->addWidget(m_loopCheck);

    m_deviceList = new QListWidget(this);
    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setMinimumHeight(220);

    root->addLayout(topRow);
    root->addLayout(optionsRow);
    root->addWidget(new QLabel(QStringLiteral("ASIO 输出设备"), this));
    root->addWidget(m_deviceList, 1);
    root->addWidget(new QLabel(QStringLiteral("日志"), this));
    root->addWidget(m_logView);

    setCentralWidget(central);

    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshDevices);
    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseWav);
    connect(m_playButton, &QPushButton::clicked, this, &MainWindow::onPlay);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(&m_pollTimer, &QTimer::timeout, this, &MainWindow::onPollPlayback);
    m_pollTimer.start(200);

    onRefreshDevices();
    updateUiState();
}

void MainWindow::appendLog(const QString& text) {
    m_logView->append(text);
}

void MainWindow::updateUiState() {
    const bool playing = m_player.isPlaying();
    const bool busy = m_stopping;
    m_playButton->setEnabled(!playing && !busy && !m_wavPath.isEmpty() && m_deviceList->currentRow() >= 0);
    m_stopButton->setEnabled(playing && !busy);
    m_browseButton->setEnabled(!playing && !busy);
    m_refreshButton->setEnabled(!playing && !busy);
    m_deviceList->setEnabled(!playing && !busy);

    if (busy) {
        m_statusLabel->setText(QStringLiteral("状态: 正在停止..."));
    } else if (playing) {
        m_statusLabel->setText(QStringLiteral("状态: 播放中"));
    } else if (m_wavPath.isEmpty()) {
        m_statusLabel->setText(QStringLiteral("状态: 请选择 WAV"));
    } else {
        m_statusLabel->setText(QStringLiteral("状态: 就绪"));
    }
}

int MainWindow::selectedDeviceIndex() const {
    const int row = m_deviceList->currentRow();
    if (row < 0 || row >= static_cast<int>(m_devices.size())) {
        return -1;
    }
    return row;
}

void MainWindow::onRefreshDevices() {
    std::string error;
    m_devices = m_player.listAsioDevices(&error);

    m_deviceList->clear();
    for (const auto& dev : m_devices) {
        const QString line = QStringLiteral("%1 | id=%2 | OUT:%3 | SR:%4")
                                 .arg(QString::fromStdString(dev.name))
                                 .arg(dev.deviceId)
                                 .arg(dev.outputChannels)
                                 .arg(dev.preferredSampleRate);
        m_deviceList->addItem(line);
    }

    if (!error.empty()) {
        appendLog(QStringLiteral("[提示] %1").arg(QString::fromStdString(error)));
    }
    if (m_devices.empty()) {
        appendLog(QStringLiteral("未发现可用 ASIO 输出设备。请确认 Dante Virtual Soundcard 已启动且未被独占。"));
    } else {
        m_deviceList->setCurrentRow(0);
        appendLog(QStringLiteral("已刷新 %1 个 ASIO 设备。").arg(m_devices.size()));
    }
    updateUiState();
}

void MainWindow::onBrowseWav() {
    const QString path = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("选择 LTC WAV 文件"),
        QString(),
        QStringLiteral("WAV Files (*.wav);;All Files (*.*)"));
    if (path.isEmpty()) {
        return;
    }

    std::string error;
    if (!m_player.loadWav(path.toStdString(), &error)) {
        QMessageBox::warning(this,
                             QStringLiteral("加载失败"),
                             QString::fromStdString(error));
        return;
    }

    m_wavPath = path;
    m_wavLabel->setText(QStringLiteral("WAV: %1 | %2 Hz | %3 ch | %4 frames")
                            .arg(path)
                            .arg(m_player.wavSampleRate())
                            .arg(m_player.wavChannels())
                            .arg(m_player.wavFrameCount()));
    appendLog(QStringLiteral("已加载 WAV: %1").arg(path));
    updateUiState();
}

void MainWindow::onPlay() {
    const int index = selectedDeviceIndex();
    if (index < 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择 ASIO 设备。"));
        return;
    }
    if (m_wavPath.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择 WAV 文件。"));
        return;
    }

    const AsioDeviceEntry& dev = m_devices[static_cast<std::size_t>(index)];
    std::string error;
    if (!m_player.start(dev.deviceId,
                        static_cast<unsigned int>(m_channelOffsetSpin->value()),
                        m_loopCheck->isChecked(),
                        &error)) {
        QMessageBox::warning(this,
                             QStringLiteral("播放失败"),
                             QString::fromStdString(error));
        appendLog(QStringLiteral("[错误] %1").arg(QString::fromStdString(error)));
        return;
    }

    appendLog(QStringLiteral("开始播放 -> %1").arg(QString::fromStdString(dev.name)));
    updateUiState();
}

void MainWindow::onStop() {
    if (m_stopping || !m_player.isPlaying()) {
        return;
    }

    m_stopping = true;
    updateUiState();

    m_player.requestStop();

    QtConcurrent::run([this]() {
        m_player.stopBlocking();
        QMetaObject::invokeMethod(this, "onStopFinished", Qt::QueuedConnection);
    });
}

void MainWindow::onStopFinished() {
    m_stopping = false;
    appendLog(QStringLiteral("已停止播放。"));
    updateUiState();
}

void MainWindow::onPollPlayback() {
    if (m_stopping) {
        return;
    }
    if (!m_player.isPlaying() && m_statusLabel->text().contains(QStringLiteral("播放中"))) {
        appendLog(QStringLiteral("播放已结束。"));
    }
    updateUiState();
}

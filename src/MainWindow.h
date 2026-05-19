#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "LtcWavPlayer.h"

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <vector>

class QCheckBox;
class QLabel;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onRefreshDevices();
    void onBrowseWav();
    void onPlay();
    void onStop();
    void onStopFinished();
    void onPollPlayback();

private:
    void appendLog(const QString& text);
    void updateUiState();
    int selectedDeviceIndex() const;

    LtcWavPlayer m_player;
    std::vector<AsioDeviceEntry> m_devices;
    QString m_wavPath;

    QListWidget* m_deviceList = nullptr;
    QLabel* m_wavLabel = nullptr;
    QSpinBox* m_channelOffsetSpin = nullptr;
    QCheckBox* m_loopCheck = nullptr;
    QPushButton* m_refreshButton = nullptr;
    QPushButton* m_browseButton = nullptr;
    QPushButton* m_playButton = nullptr;
    QPushButton* m_stopButton = nullptr;
    QLabel* m_statusLabel = nullptr;
    QTextEdit* m_logView = nullptr;
    QTimer m_pollTimer;
    bool m_stopping = false;
};

#endif

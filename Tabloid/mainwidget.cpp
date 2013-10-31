#include "engine.h"
#include "progressbar.h"
#include "levelmeter.h"
#include "settingsdialog.h"
#include "utils.h"
#include "mainwidget.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QMenu>
#include <QFileDialog>
#include <QTimerEvent>
#include <QMessageBox>

const int NullTimerId = -1;

// Disable message timeout
const int   NullMessageTimeout = -1;

MainWidget::MainWidget(QWidget *parent)
    :   QWidget(parent)
    ,   m_mode(NoMode)
    ,   m_engine(new Engine(this))
    ,   m_progressBar(new ProgressBar(this))
    ,   m_levelMeter(new LevelMeter(this))
    ,   m_modeButton(new QPushButton(this))
    ,   m_recordButton(new QPushButton(this))
    ,   m_pauseButton(new QPushButton(this))
    ,   m_playButton(new QPushButton(this))
    ,   m_settingsButton(new QPushButton(this))
    ,   m_settingsDialog(new SettingsDialog(
            m_engine->availableAudioInputDevices(),
            m_engine->availableAudioOutputDevices(),
            this))
    ,   m_infoMessage(new QLabel(tr("Select a mode to begin"), this))
    ,   m_infoMessageTimerId(NullTimerId)
    ,   m_modeMenu(new QMenu(this))
    ,   m_loadFileAction(0)
    ,   m_recordAction(0)
{

    createUi();
    connectUi();
}

MainWidget::~MainWidget()
{

}

//Public Slots

void MainWidget::stateChanged(QAudio::Mode mode, QAudio::State state)
{
    Q_UNUSED(mode);

    updateButtonStates();

    if (QAudio::ActiveState != state && QAudio::SuspendedState != state) {
        m_levelMeter->reset();
    }
}

void MainWidget::formatChanged(const QAudioFormat &format)
{
   infoMessage(formatToString(format), NullMessageTimeout);

}

void MainWidget::infoMessage(const QString &message, int timeoutMs)
{
    m_infoMessage->setText(message);

    if (NullTimerId != m_infoMessageTimerId) {
        killTimer(m_infoMessageTimerId);
        m_infoMessageTimerId = NullTimerId;
    }

    if (NullMessageTimeout != timeoutMs)
        m_infoMessageTimerId = startTimer(timeoutMs);
}

void MainWidget::errorMessage(const QString &heading, const QString &detail)
{
    QMessageBox::warning(this, heading, detail, QMessageBox::Close);
}

void MainWidget::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == m_infoMessageTimerId);
    Q_UNUSED(event) // suppress warnings in release builds
    killTimer(m_infoMessageTimerId);
    m_infoMessageTimerId = NullTimerId;
    m_infoMessage->setText("");
}

void MainWidget::audioPositionChanged(qint64 position)
{
    return;
}

void MainWidget::bufferLengthChanged(qint64 length)
{
    m_progressBar->bufferLengthChanged(length);
}

//Private Slots

void MainWidget::showFileDialog()
{
    const QString dir;
    const QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open WAV file"), dir, "*.wav");
    if (fileNames.count()) {
        reset();
        setMode(LoadFileMode);
        m_engine->loadFile(fileNames.front());
        updateButtonStates();
    } else {
        updateModeMenu();
    }
}

void MainWidget::showSettingsDialog()
{
    m_settingsDialog->exec();
    if (m_settingsDialog->result() == QDialog::Accepted) {
        m_engine->setAudioInputDevice(m_settingsDialog->inputDevice());
        m_engine->setAudioOutputDevice(m_settingsDialog->outputDevice());
    }
}

void MainWidget::initializeRecord()
{
    reset();
    setMode(RecordMode);
    if (m_engine->initializeRecord())
        updateButtonStates();
}

//Private Functions

void MainWidget::createUi()
{
    createMenus();

    setWindowTitle(tr("Tabloid"));

    QVBoxLayout *windowLayout = new QVBoxLayout(this);

    m_infoMessage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_infoMessage->setAlignment(Qt::AlignHCenter);
    windowLayout->addWidget(m_infoMessage);

#ifdef SUPERIMPOSE_PROGRESS_ON_WAVEFORM
    QScopedPointer<QHBoxLayout> waveformLayout(new QHBoxLayout);
    waveformLayout->addWidget(m_progressBar);
    m_progressBar->setMinimumHeight(m_waveform->minimumHeight());
    waveformLayout->setMargin(0);
    m_waveform->setLayout(waveformLayout.data());
    waveformLayout.take();
    windowLayout->addWidget(m_waveform);
#else
    windowLayout->addWidget(m_progressBar);
#endif // SUPERIMPOSE_PROGRESS_ON_WAVEFORM

    // Spectrograph and level meter

    QScopedPointer<QHBoxLayout> analysisLayout(new QHBoxLayout);
    analysisLayout->addWidget(m_levelMeter);
    windowLayout->addLayout(analysisLayout.data());
    analysisLayout.take();

    // Button panel

    const QSize buttonSize(30, 30);

    m_modeButton->setText(tr("Mode"));

    m_recordIcon = QIcon(":/images/record.png");
    m_recordButton->setIcon(m_recordIcon);
    m_recordButton->setEnabled(false);
    m_recordButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_recordButton->setMinimumSize(buttonSize);

    m_pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    m_pauseButton->setIcon(m_pauseIcon);
    m_pauseButton->setEnabled(false);
    m_pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pauseButton->setMinimumSize(buttonSize);

    m_playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    m_playButton->setIcon(m_playIcon);
    m_playButton->setEnabled(false);
    m_playButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_playButton->setMinimumSize(buttonSize);

    m_settingsIcon = QIcon(":/images/settings.png");
    m_settingsButton->setIcon(m_settingsIcon);
    m_settingsButton->setEnabled(true);
    m_settingsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_settingsButton->setMinimumSize(buttonSize);

    QScopedPointer<QHBoxLayout> buttonPanelLayout(new QHBoxLayout);
    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(m_modeButton);
    buttonPanelLayout->addWidget(m_recordButton);
    buttonPanelLayout->addWidget(m_pauseButton);
    buttonPanelLayout->addWidget(m_playButton);
    buttonPanelLayout->addWidget(m_settingsButton);

    QWidget *buttonPanel = new QWidget(this);
    buttonPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonPanel->setLayout(buttonPanelLayout.data());
    buttonPanelLayout.take(); // ownership transferred to buttonPanel

    QScopedPointer<QHBoxLayout> bottomPaneLayout(new QHBoxLayout);
    bottomPaneLayout->addWidget(buttonPanel);
    windowLayout->addLayout(bottomPaneLayout.data());
    bottomPaneLayout.take(); // ownership transferred to windowLayout

    // Apply layout

    setLayout(windowLayout);
}

void MainWidget::connectUi()
{
    CHECKED_CONNECT(m_recordButton, SIGNAL(clicked()),
            m_engine, SLOT(startRecording()));

    CHECKED_CONNECT(m_pauseButton, SIGNAL(clicked()),
            m_engine, SLOT(suspend()));

    CHECKED_CONNECT(m_playButton, SIGNAL(clicked()),
            m_engine, SLOT(startPlayback()));

    CHECKED_CONNECT(m_settingsButton, SIGNAL(clicked()),
            this, SLOT(showSettingsDialog()));

    CHECKED_CONNECT(m_engine, SIGNAL(stateChanged(QAudio::Mode,QAudio::State)),
            this, SLOT(stateChanged(QAudio::Mode,QAudio::State)));

    CHECKED_CONNECT(m_engine, SIGNAL(formatChanged(const QAudioFormat &)),
            this, SLOT(formatChanged(const QAudioFormat &)));

    m_progressBar->bufferLengthChanged(m_engine->bufferLength());

    CHECKED_CONNECT(m_engine, SIGNAL(bufferLengthChanged(qint64)),
            this, SLOT(bufferLengthChanged(qint64)));

    CHECKED_CONNECT(m_engine, SIGNAL(dataLengthChanged(qint64)),
            this, SLOT(updateButtonStates()));

    CHECKED_CONNECT(m_engine, SIGNAL(recordPositionChanged(qint64)),
            m_progressBar, SLOT(recordPositionChanged(qint64)));

    CHECKED_CONNECT(m_engine, SIGNAL(playPositionChanged(qint64)),
            m_progressBar, SLOT(playPositionChanged(qint64)));

    CHECKED_CONNECT(m_engine, SIGNAL(recordPositionChanged(qint64)),
            this, SLOT(audioPositionChanged(qint64)));

    CHECKED_CONNECT(m_engine, SIGNAL(playPositionChanged(qint64)),
            this, SLOT(audioPositionChanged(qint64)));

    CHECKED_CONNECT(m_engine, SIGNAL(levelChanged(qreal, qreal, int)),
            m_levelMeter, SLOT(levelChanged(qreal, qreal, int)));

    CHECKED_CONNECT(m_engine, SIGNAL(infoMessage(QString, int)),
            this, SLOT(infoMessage(QString, int)));

    CHECKED_CONNECT(m_engine, SIGNAL(errorMessage(QString, QString)),
            this, SLOT(errorMessage(QString, QString)));

}

void MainWidget::createMenus()
{
    m_modeButton->setMenu(m_modeMenu);

    m_recordAction = m_modeMenu->addAction(tr("Record and parse"));
    m_loadFileAction = m_modeMenu->addAction(tr("Play and parse file"));

    m_loadFileAction->setCheckable(true);
    m_recordAction->setCheckable(true);

    connect(m_loadFileAction, SIGNAL(triggered(bool)), this, SLOT(showFileDialog()));
    connect(m_recordAction, SIGNAL(triggered(bool)), this, SLOT(initializeRecord()));
}

void MainWidget::updateButtonStates()
{
    const bool recordEnabled = ((QAudio::AudioOutput == m_engine->mode() ||
                                (QAudio::ActiveState != m_engine->state() &&
                                 QAudio::IdleState != m_engine->state())) &&
                                RecordMode == m_mode);
    m_recordButton->setEnabled(recordEnabled);

    const bool pauseEnabled = (QAudio::ActiveState == m_engine->state() ||
                               QAudio::IdleState == m_engine->state());
    m_pauseButton->setEnabled(pauseEnabled);

    const bool playEnabled = (/*m_engine->dataLength() &&*/
                              (QAudio::AudioOutput != m_engine->mode() ||
                               (QAudio::ActiveState != m_engine->state() &&
                                QAudio::IdleState != m_engine->state())));
    m_playButton->setEnabled(playEnabled);
}

void MainWidget::reset()
{
    m_engine->reset();
    m_levelMeter->reset();
    m_progressBar->reset();
}

void MainWidget::setMode(Mode mode)
{
    m_mode = mode;
    updateModeMenu();
}

void MainWidget::updateModeMenu()
{
    m_loadFileAction->setChecked(LoadFileMode == m_mode);
    m_recordAction->setChecked(RecordMode == m_mode);
}


#pragma once

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QVariantMap>

#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#ifdef Q_OS_WIN
#include <io.h>
#include <sys/stat.h>
#define AUDIO_PLAYER_OPEN(path) _open(path, _O_WRONLY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE)
#define AUDIO_PLAYER_DUP _dup
#define AUDIO_PLAYER_DUP2 _dup2
#define AUDIO_PLAYER_CLOSE _close
#else
#include <unistd.h>
#define AUDIO_PLAYER_OPEN(path) ::open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600)
#define AUDIO_PLAYER_DUP dup
#define AUDIO_PLAYER_DUP2 dup2
#define AUDIO_PLAYER_CLOSE close
#endif

// Command-line handling shared by the production and the Runner-linked executable.
struct StartupOptions {
    QStringList inputs;     // files, directories or URLs
    bool demo = false;
    bool idle = false;      // test binary only: skip its default demo source
    qreal volume = 0.8;
};

inline StartupOptions parseStartupOptions(const QCoreApplication& app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("QmlTui audio_player: terminal spectrum visualizer and audio player.\n"
                       "Give audio files, directories or URLs to play them; use --demo for the built-in synth album."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("media"),
                                 QStringLiteral("Audio files, directories or URLs (optional)"),
                                 QStringLiteral("[media...]"));
    const QCommandLineOption demoOption(QStringLiteral("demo"),
                                        QStringLiteral("Start with the procedural demo source (showcase / tests)"));
    QCommandLineOption idleOption(QStringLiteral("idle"), QStringLiteral("Start without any source"));
    idleOption.setFlags(QCommandLineOption::HiddenFromHelp);
    const QCommandLineOption volumeOption(QStringLiteral("volume"),
                                          QStringLiteral("Initial playback volume in percent"),
                                          QStringLiteral("percent"), QStringLiteral("80"));
    parser.addOptions({ demoOption, volumeOption, idleOption });
    parser.process(app);

    StartupOptions options;
    options.inputs = parser.positionalArguments();
    options.demo = parser.isSet(demoOption);
    options.idle = parser.isSet(idleOption);
    bool ok = false;
    const int percent = parser.value(volumeOption).toInt(&ok);
    if (ok)
        options.volume = std::clamp(percent, 0, 100) / 100.0;
    return options;
}

inline QVariantMap toVariantMap(const StartupOptions& options)
{
    return {
        { QStringLiteral("inputs"), options.inputs },
        { QStringLiteral("demo"), options.demo },
        { QStringLiteral("volume"), options.volume },
    };
}

// Third-party libraries used by Qt Multimedia (libav, PipeWire, PulseAudio) print
// straight to the stderr file descriptor, bypassing Qt logging and therefore QmlTui's
// message buffering. Any such line would corrupt the alternate-screen frame, so the
// descriptor is parked in a log file while the UI runs and restored afterwards.
class StderrRedirect
{
public:
    explicit StderrRedirect(const QString& logPath)
        : m_path(logPath)
    {
        std::fflush(stderr);
        const int fd = AUDIO_PLAYER_OPEN(QFile::encodeName(m_path).constData());
        if (fd < 0)
            return;
        m_saved = AUDIO_PLAYER_DUP(2);
        AUDIO_PLAYER_DUP2(fd, 2);
        AUDIO_PLAYER_CLOSE(fd);
    }

    ~StderrRedirect()
    {
        if (m_saved < 0)
            return;
        std::fflush(stderr);
        AUDIO_PLAYER_DUP2(m_saved, 2);
        AUDIO_PLAYER_CLOSE(m_saved);
        const qint64 size = QFileInfo(m_path).size();
        if (size > 0)
            std::fprintf(stderr, "qmltui_audio_player: %lld bytes of library diagnostics in %s\n",
                         static_cast<long long>(size), qPrintable(m_path));
        else
            QFile::remove(m_path);
    }

    StderrRedirect(const StderrRedirect&) = delete;
    StderrRedirect& operator=(const StderrRedirect&) = delete;

    static QString defaultLogPath(const QString& appName)
    {
        return QDir::temp().filePath(appName + QStringLiteral(".log"));
    }

private:
    QString m_path;
    int m_saved = -1;
};

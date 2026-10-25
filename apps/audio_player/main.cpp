#include "AudioEngine.h"
#include "Startup.h"

#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <memory>

#include <cstdio>

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("qmltui_audio_player"));
  app.setApplicationVersion(QStringLiteral("1.0"));

  StartupOptions options = parseStartupOptions(app);
  const bool runnerRequested =
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_SCRIPT") ||
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_EVAL") ||
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_CAPTURE_AFTER_MS");
  if (runnerRequested && options.inputs.isEmpty() && !options.idle)
    options.demo = true;

  if (!AudioEngine::multimediaAvailable() && !options.inputs.isEmpty()) {
    std::fprintf(stderr, "qmltui_audio_player: built without Qt Multimedia; no "
                         "audio source available "
                         "(use --demo for the showcase).\n");
    options.inputs.clear();
  }

  // Keep libav / PipeWire chatter off the terminal while the UI owns the
  // screen. Quiet scenario runs render to committed frames, not the tty, and
  // the runner's own summary must stay on stderr, so they are left alone.
  const bool scriptedQuiet =
      runnerRequested && !qEnvironmentVariableIsSet("QMLTUI_RUNNER_LIVE");
  std::unique_ptr<StderrRedirect> quietStderr;
  if (AudioEngine::multimediaAvailable() && !options.demo && !scriptedQuiet)
    quietStderr = std::make_unique<StderrRedirect>(
        StderrRedirect::defaultLogPath(app.applicationName()));

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty(QStringLiteral("cliOptions"),
                                           toVariantMap(options));
  engine.loadFromModule("AudioPlayer", "Main");
  if (engine.rootObjects().isEmpty())
    return -1;
  const int result = app.exec();
  // Restore stderr before the QML engine (and QmlTui's buffered Qt messages) go
  // away.
  quietStderr.reset();
  return result;
}

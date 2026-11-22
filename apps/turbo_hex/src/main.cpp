#include "HexBackend.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
  const bool runnerActive =
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_SCRIPT") ||
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_EVAL") ||
      qEnvironmentVariableIsSet("QMLTUI_RUNNER_CAPTURE_AFTER_MS");

  QCoreApplication app(argc, argv);
  app.setOrganizationName(QStringLiteral("QmlTui"));
  app.setApplicationName(QStringLiteral("qmltui_turbo_hex"));
  app.setApplicationVersion(QStringLiteral("0.6.0"));

  QCommandLineParser parser;
  parser.setApplicationDescription(
      QStringLiteral("Turbo Hex — Multi-Window Hex & Binary Inspector built on "
                     "QmlTui.TVision"));
  parser.addHelpOption();
  parser.addVersionOption();
  QCommandLineOption demoOpt(
      QStringLiteral("demo"),
      QStringLiteral("Start with built-in ELF64 and PNG sample binaries."));
  parser.addOption(demoOpt);
  parser.addPositionalArgument(
      QStringLiteral("files"),
      QStringLiteral("Binary files or directory to inspect."),
      QStringLiteral("[files | dir...]"));
  parser.process(app);

  const QStringList positional = parser.positionalArguments();
  const bool forceDemo =
#ifdef Q_OS_WASM
      true;
#else
      parser.isSet(demoOpt) ||
      qEnvironmentVariableIntValue("QMLTUI_TH_DEMO") == 1 ||
      (runnerActive && positional.isEmpty());
#endif

  turbohex::HexBackend::configureStartup(positional, forceDemo);

  QQmlApplicationEngine engine;
  engine.loadFromModule("TurboHexApp", "Main");
  if (engine.rootObjects().isEmpty()) {
    return -1;
  }
  return app.exec();
}

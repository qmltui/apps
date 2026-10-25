#include <QCoreApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    // Automated scenario runs (QMLTUI_RUNNER_SCRIPT/QMLTUI_RUNNER_EVAL/QMLTUI_RUNNER_CAPTURE_AFTER_MS) default to
    // deterministic mock filesystem; set QMLTUI_FM_LIVE=1 explicitly to test live system.
    const bool scenarioRequested = qEnvironmentVariableIsSet("QMLTUI_RUNNER_SCRIPT")
        || qEnvironmentVariableIsSet("QMLTUI_RUNNER_EVAL")
        || qEnvironmentVariableIsSet("QMLTUI_RUNNER_CAPTURE_AFTER_MS");

    if (scenarioRequested && !qEnvironmentVariableIsSet("QMLTUI_FM_LIVE")) {
        qputenv("QMLTUI_FM_MOCK", "1");
    }

    QCoreApplication app(argc, argv);
    app.setApplicationName("qmltui_file_manager");

    QQmlApplicationEngine engine;
    engine.loadFromModule("QmlTui.Examples.FileManager", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

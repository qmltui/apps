#include "PackageModels.h"

#include <QtTest>

class TestPackageManagerEngine : public QObject {
    Q_OBJECT

private slots:
    void featureTogglesUpdateTransactionPlan() {
        PackageEngine engine;
        QCOMPARE(engine.phase(), static_cast<int>(PackageEngine::PromptPhase));
        QCOMPARE(engine.totalPackages(), 8);
        QCOMPARE(engine.cachedPackages(), 2);
        QVERIFY(!engine.wasmEnabled());

        engine.toggleFeature(2); // enable wasm-sdk
        QVERIFY(engine.wasmEnabled());
        QCOMPARE(engine.totalPackages(), 10);
        QVERIFY(engine.activeFeaturesText().contains(QLatin1String("wasm-sdk")));

        engine.toggleFeature(0); // disable ptyview
        QVERIFY(!engine.ptyviewEnabled());
        QCOMPARE(engine.totalPackages(), 9);
    }

    void fullTransactionLifecycleReachesFinished() {
        PackageEngine engine;
        engine.toggleFeature(2); // 10 packages
        engine.confirmInstall();

        QCOMPARE(engine.phase(), static_cast<int>(PackageEngine::FetchingPhase));
        // 2 cached packages resolve immediately on startFetchingPhase
        QVERIFY(engine.completedPackages() >= 2);

        int safetyTicks = 0;
        while (engine.phase() == PackageEngine::FetchingPhase && safetyTicks < 200) {
            engine.tick(50);
            ++safetyTicks;
        }
        QCOMPARE(engine.phase(), static_cast<int>(PackageEngine::BuildingPhase));

        engine.togglePause();
        QVERIFY(engine.paused());
        const double frozenProgress = engine.overallProgress();
        engine.tick(50);
        QCOMPARE(engine.overallProgress(), frozenProgress);
        engine.togglePause();

        while (engine.phase() == PackageEngine::BuildingPhase && safetyTicks < 500) {
            engine.tick(50);
            ++safetyTicks;
        }
        QCOMPARE(engine.phase(), static_cast<int>(PackageEngine::FinishedPhase));
        QCOMPARE(engine.overallProgress(), 1.0);
        QVERIFY(engine.feedModel()->rowCount() >= 20);
    }
};

QTEST_MAIN(TestPackageManagerEngine)
#include "package_manager_engine.moc"

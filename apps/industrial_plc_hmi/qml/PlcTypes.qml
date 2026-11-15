pragma Singleton
pragma ComponentBehavior: Bound

import QtQml

QtObject {
    id: root

    enum MachineState {
        Idle,
        Processing,
        Error,
        Maintenance
    }

    enum Tab {
        Dashboard,
        Alarms,
        Diagnostics,
        Settings
    }

    enum PendingAction {
        NoAction,
        Reset,
        ToggleMaintenance
    }

    enum TraceMode {
        AllTraces,
        TempOnly,
        SpeedOnly
    }
}

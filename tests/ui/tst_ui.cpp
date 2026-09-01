// Runs every tst_*.qml in this directory.

#include <QtCore/QStandardPaths>
#include <QtQuickTest/quicktest.h>

class Setup : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void applicationAvailable()
    {
        QStandardPaths::setTestModeEnabled(true);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(ui, Setup)

#include "tst_ui.moc"

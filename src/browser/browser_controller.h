// The windows of the running browser, and the session history they share.
//
// There is exactly one of these. It is the only owner of SessionHistory, which
// is the only place a visited URL is remembered - in memory, for this process
// only (PLAN.md §18).

#ifndef PB_BROWSER_BROWSER_CONTROLLER_H
#define PB_BROWSER_BROWSER_CONTROLLER_H

#include "browser/core/session_history.h"
#include "browser/window_controller.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtQml/qqmlregistration.h>

namespace pb::settings {
class SettingsController;
}

namespace pb::browser {

class WindowListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by the browser controller")

public:
    enum Roles {
        WindowRole = Qt::UserRole + 1,
    };

    using QAbstractListModel::QAbstractListModel;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void append(WindowController *window);
    void remove(WindowController *window);
    const QList<WindowController *> &windows() const { return m_windows; }

private:
    QList<WindowController *> m_windows;
};

class BrowserController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application as App.browser")

    Q_PROPERTY(QAbstractListModel *windows READ windowModel CONSTANT)
    Q_PROPERTY(int windowCount READ windowCount NOTIFY windowCountChanged)

public:
    explicit BrowserController(pb::settings::SettingsController *settings,
                               QObject *parent = nullptr);

    Q_INVOKABLE pb::browser::WindowController *createWindow(bool privateMode = false);
    Q_INVOKABLE void closeWindow(pb::browser::WindowController *window);

    QAbstractListModel *windowModel() { return &m_windows; }
    int windowCount() const;

    SessionHistory &history() { return m_history; }
    const SessionHistory &history() const { return m_history; }
    pb::settings::SettingsController *settings() const { return m_settings; }

    // Closes every window and clears the session history. Idempotent.
    void shutdown();

Q_SIGNALS:
    void windowCountChanged();
    void allWindowsClosed();

private:
    pb::settings::SettingsController *m_settings = nullptr;
    WindowListModel m_windows;
    SessionHistory m_history;
};

} // namespace pb::browser

#endif // PB_BROWSER_BROWSER_CONTROLLER_H

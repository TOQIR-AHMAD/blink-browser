#include "browser/browser_controller.h"

#include "browser/window_controller.h"
#include "utils/logging.h"

namespace pb::browser {

int WindowListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_windows.size());
}

QVariant WindowListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_windows.size())
        return {};
    if (role == WindowRole)
        return QVariant::fromValue(m_windows.at(index.row()));
    return {};
}

QHash<int, QByteArray> WindowListModel::roleNames() const
{
    return { { WindowRole, QByteArrayLiteral("window") } };
}

void WindowListModel::append(WindowController *window)
{
    const int row = static_cast<int>(m_windows.size());
    beginInsertRows(QModelIndex(), row, row);
    m_windows.append(window);
    endInsertRows();
}

void WindowListModel::remove(WindowController *window)
{
    const int row = static_cast<int>(m_windows.indexOf(window));
    if (row < 0)
        return;
    beginRemoveRows(QModelIndex(), row, row);
    m_windows.removeAt(row);
    endRemoveRows();
}

BrowserController::BrowserController(pb::settings::SettingsController *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

int BrowserController::windowCount() const
{
    return static_cast<int>(m_windows.windows().size());
}

WindowController *BrowserController::createWindow(bool privateMode)
{
    auto *window = new WindowController(this, privateMode, this);
    m_windows.append(window);
    Q_EMIT windowCountChanged();
    return window;
}

void BrowserController::closeWindow(WindowController *window)
{
    if (!window || !m_windows.windows().contains(window))
        return;

    window->shutdown();
    m_windows.remove(window);
    window->deleteLater();
    Q_EMIT windowCountChanged();

    if (m_windows.windows().isEmpty())
        Q_EMIT allWindowsClosed();
}

void BrowserController::shutdown()
{
    const QList<WindowController *> windows = m_windows.windows();
    for (WindowController *window : windows)
        closeWindow(window);

    m_history.clear();
    pb::log::write(pb::log::Level::Info, "session history cleared");
}

} // namespace pb::browser

#include "tabs/tab_model.h"

#include "tabs/tab.h"

namespace pb::tabs {
namespace {
constexpr int kMaxClosedTabs = 10;
}

TabModel::TabModel(bool privateMode, QObject *parent)
    : QAbstractListModel(parent)
    , m_privateMode(privateMode)
{
}

int TabModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : count();
}

QVariant TabModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= count())
        return {};
    if (role == TabRole)
        return QVariant::fromValue(m_tabs.at(index.row()));
    return {};
}

QHash<int, QByteArray> TabModel::roleNames() const
{
    return { { TabRole, QByteArrayLiteral("tab") } };
}

Tab *TabModel::tabAt(int index) const
{
    if (index < 0 || index >= count())
        return nullptr;
    return m_tabs.at(index);
}

Tab *TabModel::currentTab() const
{
    return tabAt(m_currentIndex);
}

void TabModel::setCurrentIndex(int index)
{
    const int clamped = m_tabs.isEmpty() ? -1 : qBound(0, index, count() - 1);
    if (clamped == m_currentIndex)
        return;
    m_currentIndex = clamped;
    Q_EMIT currentIndexChanged();
}

Tab *TabModel::addTab(const QUrl &url, bool background)
{
    const int row = count();
    beginInsertRows(QModelIndex(), row, row);
    auto *tab = new Tab(m_nextTabId++, url, m_privateMode, this);
    m_tabs.append(tab);
    endInsertRows();

    Q_EMIT countChanged();
    Q_EMIT tabAdded(tab);
    if (!background || m_currentIndex < 0)
        setCurrentIndex(row);
    return tab;
}

void TabModel::closeTab(int index)
{
    Tab *tab = tabAt(index);
    if (!tab)
        return;

    if (!tab->url().isEmpty() && tab->url().scheme() != QLatin1String("about")) {
        m_closedTabs.append({ tab->url(), tab->displayTitle(), index });
        while (m_closedTabs.size() > kMaxClosedTabs)
            m_closedTabs.removeFirst();
        Q_EMIT closedTabsChanged();
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_tabs.removeAt(index);
    endRemoveRows();
    tab->deleteLater();

    Q_EMIT countChanged();

    if (m_tabs.isEmpty()) {
        m_currentIndex = -1;
        Q_EMIT currentIndexChanged();
        Q_EMIT lastTabClosed();
        return;
    }

    // Keep the selection on the neighbour, the way every browser does.
    const int next = qBound(0, index >= count() ? count() - 1 : index, count() - 1);
    m_currentIndex = -1; // force the change signal even when the number matches
    setCurrentIndex(next);
}

void TabModel::closeCurrentTab()
{
    closeTab(m_currentIndex);
}

void TabModel::duplicateTab(int index)
{
    Tab *source = tabAt(index);
    if (!source)
        return;
    // A duplicate starts at the same address with a fresh history: copying the
    // back/forward list would mean copying browsing state between views.
    addTab(source->url(), false);
}

void TabModel::moveTab(int from, int to)
{
    if (from == to || from < 0 || to < 0 || from >= count() || to >= count())
        return;

    // Remember which tab is selected before the move: afterwards
    // m_currentIndex points at whatever slid into that slot.
    Tab *const current = currentTab();

    const int destination = to > from ? to + 1 : to;
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), destination);
    m_tabs.move(from, to);
    endMoveRows();

    if (current) {
        const int index = m_tabs.indexOf(current);
        if (index != m_currentIndex) {
            m_currentIndex = index;
            Q_EMIT currentIndexChanged();
        }
    }
}

void TabModel::reopenClosedTab()
{
    if (m_closedTabs.isEmpty())
        return;
    const ClosedTab closed = m_closedTabs.takeLast();
    Q_EMIT closedTabsChanged();
    addTab(closed.url, false);
}

void TabModel::selectNext()
{
    if (m_tabs.isEmpty())
        return;
    setCurrentIndex((m_currentIndex + 1) % count());
}

void TabModel::selectPrevious()
{
    if (m_tabs.isEmpty())
        return;
    setCurrentIndex((m_currentIndex - 1 + count()) % count());
}

void TabModel::selectFirst()
{
    setCurrentIndex(0);
}

void TabModel::selectLast()
{
    setCurrentIndex(count() - 1);
}

int TabModel::indexOfTab(Tab *tab) const
{
    return static_cast<int>(m_tabs.indexOf(tab));
}

void TabModel::closeAll()
{
    if (!m_tabs.isEmpty()) {
        beginResetModel();
        qDeleteAll(m_tabs);
        m_tabs.clear();
        m_currentIndex = -1;
        endResetModel();
        Q_EMIT countChanged();
        Q_EMIT currentIndexChanged();
    }
    if (!m_closedTabs.isEmpty()) {
        m_closedTabs.clear();
        Q_EMIT closedTabsChanged();
    }
}

} // namespace pb::tabs

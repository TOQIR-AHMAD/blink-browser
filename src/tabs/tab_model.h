// The tabs of one window.
//
// Closed tabs are remembered so Ctrl+Shift+T can bring them back, but only as
// a URL and a title, only in memory, and only until the window closes
// (PLAN.md §18: nothing about browsing survives the session).

#ifndef PB_TABS_TAB_MODEL_H
#define PB_TABS_TAB_MODEL_H

#include "tabs/tab.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtQml/qqmlregistration.h>

namespace pb::tabs {

class TabModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Tab models belong to a window")

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(pb::tabs::Tab *currentTab READ currentTab NOTIFY currentIndexChanged)
    Q_PROPERTY(bool canReopenClosedTab READ canReopenClosedTab NOTIFY closedTabsChanged)

public:
    enum Roles {
        TabRole = Qt::UserRole + 1,
    };

    explicit TabModel(bool privateMode, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(m_tabs.size()); }
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);
    Tab *currentTab() const;
    bool canReopenClosedTab() const { return !m_closedTabs.isEmpty(); }
    bool privateMode() const { return m_privateMode; }

    Q_INVOKABLE pb::tabs::Tab *tabAt(int index) const;
    Q_INVOKABLE pb::tabs::Tab *addTab(const QUrl &url = QUrl(), bool background = false);
    Q_INVOKABLE void closeTab(int index);
    Q_INVOKABLE void closeCurrentTab();
    Q_INVOKABLE void duplicateTab(int index);
    Q_INVOKABLE void moveTab(int from, int to);
    Q_INVOKABLE void reopenClosedTab();
    Q_INVOKABLE void selectNext();
    Q_INVOKABLE void selectPrevious();
    Q_INVOKABLE void selectFirst();
    Q_INVOKABLE void selectLast();
    Q_INVOKABLE int indexOfTab(pb::tabs::Tab *tab) const;

    // Drops every tab and the closed-tab list. Called when the window closes.
    void closeAll();

Q_SIGNALS:
    void countChanged();
    void currentIndexChanged();
    void closedTabsChanged();
    void tabAdded(pb::tabs::Tab *tab);
    void lastTabClosed();

private:
    struct ClosedTab {
        QUrl url;
        QString title;
        int index = 0;
    };

    bool m_privateMode = false;
    int m_nextTabId = 1;
    int m_currentIndex = -1;
    QList<Tab *> m_tabs;
    QList<ClosedTab> m_closedTabs;
};

} // namespace pb::tabs

#endif // PB_TABS_TAB_MODEL_H

#include "downloads/download_manager.h"

#include "settings/settings_controller.h"
#include "utils/logging.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtWebEngineCore/QWebEngineDownloadRequest>

namespace pb::downloads {

DownloadItem::DownloadItem(int id, QWebEngineDownloadRequest *request, bool askWhereToSave,
                           QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_request(request)
    , m_askWhereToSave(askWhereToSave)
{
    if (!request)
        return;

    m_fileName = request->downloadFileName();
    m_totalBytes = request->totalBytes();
    m_temporaryPath = request->downloadDirectory() + QLatin1Char('/') + m_fileName;
    m_savePath = m_askWhereToSave ? QString() : m_temporaryPath;

    connect(request, &QWebEngineDownloadRequest::receivedBytesChanged, this, [this] {
        if (m_request)
            m_receivedBytes = m_request->receivedBytes();
        Q_EMIT progressChanged();
    });
    connect(request, &QWebEngineDownloadRequest::totalBytesChanged, this, [this] {
        if (m_request)
            m_totalBytes = m_request->totalBytes();
        Q_EMIT progressChanged();
    });
    connect(request, &QWebEngineDownloadRequest::isPausedChanged, this, [this] {
        m_paused = m_request && m_request->isPaused();
        Q_EMIT changed();
    });
    connect(request, &QWebEngineDownloadRequest::stateChanged, this,
            &DownloadItem::onStateChanged);
}

qreal DownloadItem::progress() const
{
    if (m_totalBytes <= 0)
        return 0.0;
    return static_cast<qreal>(m_receivedBytes) / static_cast<qreal>(m_totalBytes);
}

void DownloadItem::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;
    Q_EMIT changed();
}

void DownloadItem::onStateChanged()
{
    if (!m_request)
        return;

    switch (m_request->state()) {
    case QWebEngineDownloadRequest::DownloadInProgress:
        setState(Downloading);
        break;
    case QWebEngineDownloadRequest::DownloadCompleted:
        if (m_askWhereToSave && m_savePath.isEmpty()) {
            setState(AwaitingLocation);
            Q_EMIT locationNeeded();
        } else {
            setState(Completed);
        }
        break;
    case QWebEngineDownloadRequest::DownloadCancelled:
        setState(Cancelled);
        break;
    case QWebEngineDownloadRequest::DownloadInterrupted:
        setState(Failed);
        break;
    case QWebEngineDownloadRequest::DownloadRequested:
        break;
    }
}

void DownloadItem::pause()
{
    if (m_request)
        m_request->pause();
}

void DownloadItem::resume()
{
    if (m_request)
        m_request->resume();
}

void DownloadItem::cancel()
{
    if (m_request)
        m_request->cancel();
    setState(Cancelled);
}

bool DownloadItem::moveTo(const QString &target)
{
    if (target.isEmpty())
        return false;

    QDir().mkpath(QFileInfo(target).absolutePath());
    if (QFile::exists(target) && !QFile::remove(target)) {
        pb::log::write(pb::log::Level::Error, "cannot replace the existing file at the target");
        setState(Failed);
        return false;
    }
    if (!QFile::rename(m_temporaryPath, target)) {
        // Different volume: fall back to copy + remove.
        if (!QFile::copy(m_temporaryPath, target)) {
            pb::log::write(pb::log::Level::Error, "could not save the downloaded file");
            setState(Failed);
            return false;
        }
        QFile::remove(m_temporaryPath);
    }

    m_savePath = target;
    m_fileName = QFileInfo(target).fileName();
    setState(Completed);
    Q_EMIT changed();
    return true;
}

DownloadManager::DownloadManager(pb::settings::SettingsController *settings,
                                 const QString &temporaryDirectory, QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(settings)
    , m_temporaryDirectory(temporaryDirectory)
{
}

int DownloadManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : count();
}

QVariant DownloadManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= count())
        return {};
    if (role == DownloadRole)
        return QVariant::fromValue(m_items.at(index.row()));
    return {};
}

QHash<int, QByteArray> DownloadManager::roleNames() const
{
    return { { DownloadRole, QByteArrayLiteral("download") } };
}

int DownloadManager::activeCount() const
{
    int active = 0;
    for (const DownloadItem *item : m_items) {
        if (item->state() == DownloadItem::Downloading)
            ++active;
    }
    return active;
}

QString DownloadManager::suggestedSaveDirectory() const
{
    if (m_settings && !m_settings->downloadDirectory().isEmpty())
        return m_settings->downloadDirectory();
    const QString downloads
        = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    return downloads.isEmpty() ? QDir::homePath() : downloads;
}

void DownloadManager::handleDownloadRequested(QWebEngineDownloadRequest *request)
{
    if (!request)
        return;

    const bool ask = !m_settings || m_settings->askWhereToSave();

    // Qt WebEngine needs the destination before the download is accepted, so
    // "ask where to save" downloads land in the session's temporary directory
    // and are moved once the user answers.
    if (ask) {
        const QString staging = m_temporaryDirectory + QStringLiteral("/staged");
        QDir().mkpath(staging);
        request->setDownloadDirectory(staging);
    } else {
        const QString directory = suggestedSaveDirectory();
        QDir().mkpath(directory);
        request->setDownloadDirectory(directory);
    }

    auto *item = new DownloadItem(m_nextId++, request, ask, this);
    connect(item, &DownloadItem::locationNeeded, this,
            [this, item] { Q_EMIT locationNeeded(item); });

    append(item);
    request->accept();
    Q_EMIT downloadStarted(item);
}

void DownloadManager::append(DownloadItem *item)
{
    const int row = count();
    beginInsertRows(QModelIndex(), row, row);
    m_items.append(item);
    endInsertRows();
    Q_EMIT countChanged();
}

void DownloadManager::saveAs(DownloadItem *item, const QUrl &target)
{
    if (!item)
        return;
    const QString path = target.isLocalFile() ? target.toLocalFile() : target.toString();
    item->moveTo(path);
}

void DownloadManager::discard(DownloadItem *item)
{
    if (!item)
        return;
    const int row = static_cast<int>(m_items.indexOf(item));
    if (row < 0)
        return;

    item->cancel();
    if (!item->temporaryPath().isEmpty() && item->savePath().isEmpty())
        QFile::remove(item->temporaryPath());

    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
    item->deleteLater();
    Q_EMIT countChanged();
}

void DownloadManager::clearFinished()
{
    for (int row = count() - 1; row >= 0; --row) {
        DownloadItem *item = m_items.at(row);
        if (item->state() == DownloadItem::Downloading
            || item->state() == DownloadItem::AwaitingLocation) {
            continue;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_items.removeAt(row);
        endRemoveRows();
        item->deleteLater();
    }
    Q_EMIT countChanged();
}

void DownloadManager::openContainingFolder(DownloadItem *item) const
{
    if (!item || item->savePath().isEmpty())
        return;
    const QString directory = QFileInfo(item->savePath()).absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
}

void DownloadManager::shutdown()
{
    if (m_items.isEmpty())
        return;

    beginResetModel();
    for (DownloadItem *item : m_items) {
        if (item->state() == DownloadItem::Downloading)
            item->cancel();
        item->deleteLater();
    }
    m_items.clear();
    endResetModel();
    Q_EMIT countChanged();
}

} // namespace pb::downloads

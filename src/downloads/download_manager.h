// Downloads.
//
// PLAN.md §28 is explicit that downloads are the one thing that genuinely
// reaches the disk, and that the browser must not pretend otherwise. So:
//
// - The list of downloads lives in memory and dies with the session. There is
//   no download history file and no "recent downloads" that outlives a launch.
// - Nothing about a download is sent anywhere: no filename lookup, no
//   reputation check, no upload.
// - When "ask where to save" is on, the file is written to the session's
//   temporary directory first and moved where the user asks once they answer.
//   If they never answer, the temporary copy goes with the session directory.

#ifndef PB_DOWNLOADS_DOWNLOAD_MANAGER_H
#define PB_DOWNLOADS_DOWNLOAD_MANAGER_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE
class QWebEngineDownloadRequest;
QT_END_NAMESPACE

namespace pb::settings {
class SettingsController;
}

namespace pb::downloads {

class DownloadItem : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Downloads are created by the download manager")

    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString fileName READ fileName NOTIFY changed)
    Q_PROPERTY(QString savePath READ savePath NOTIFY changed)
    Q_PROPERTY(qint64 receivedBytes READ receivedBytes NOTIFY progressChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY progressChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int state READ state NOTIFY changed)
    Q_PROPERTY(bool paused READ paused NOTIFY changed)

public:
    enum State {
        Downloading,
        AwaitingLocation, // finished, waiting for the user to say where it goes
        Completed,
        Cancelled,
        Failed,
    };
    Q_ENUM(State)

    DownloadItem(int id, QWebEngineDownloadRequest *request, bool askWhereToSave,
                 QObject *parent = nullptr);

    int id() const { return m_id; }
    QString fileName() const { return m_fileName; }
    QString savePath() const { return m_savePath; }
    qint64 receivedBytes() const { return m_receivedBytes; }
    qint64 totalBytes() const { return m_totalBytes; }
    qreal progress() const;
    int state() const { return m_state; }
    bool paused() const { return m_paused; }
    bool needsLocation() const { return m_askWhereToSave; }
    QString temporaryPath() const { return m_temporaryPath; }

    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void cancel();

    // Moves the completed temporary file to `target`. Returns false and sets
    // the state to Failed if the move does not succeed.
    bool moveTo(const QString &target);

Q_SIGNALS:
    void changed();
    void progressChanged();
    void locationNeeded();

private:
    void setState(State state);
    void onStateChanged();

    const int m_id;
#ifdef PB_WEB_ENGINE
    QPointer<QWebEngineDownloadRequest> m_request;
#endif
    bool m_askWhereToSave = false;
    QString m_fileName;
    QString m_savePath;
    QString m_temporaryPath;
    qint64 m_receivedBytes = 0;
    qint64 m_totalBytes = 0;
    State m_state = Downloading;
    bool m_paused = false;
};

class DownloadManager : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application as App.downloads")

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int activeCount READ activeCount NOTIFY countChanged)

public:
    enum Roles {
        DownloadRole = Qt::UserRole + 1,
    };

    DownloadManager(pb::settings::SettingsController *settings, const QString &temporaryDirectory,
                    QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(m_items.size()); }
    int activeCount() const;

    // Connected to QWebEngineProfile::downloadRequested.
    void handleDownloadRequested(QWebEngineDownloadRequest *request);

    Q_INVOKABLE QString suggestedSaveDirectory() const;
    Q_INVOKABLE void saveAs(pb::downloads::DownloadItem *item, const QUrl &target);
    Q_INVOKABLE void discard(pb::downloads::DownloadItem *item);
    Q_INVOKABLE void clearFinished();
    Q_INVOKABLE void openContainingFolder(pb::downloads::DownloadItem *item) const;

    // Cancels anything still running and empties the list.
    void shutdown();

Q_SIGNALS:
    void countChanged();
    // The UI shows a save dialog in response to this.
    void locationNeeded(pb::downloads::DownloadItem *item);
    void downloadStarted(pb::downloads::DownloadItem *item);

private:
    void append(DownloadItem *item);

    pb::settings::SettingsController *m_settings = nullptr;
    QString m_temporaryDirectory;
    int m_nextId = 1;
    QList<DownloadItem *> m_items;
};

} // namespace pb::downloads

#endif // PB_DOWNLOADS_DOWNLOAD_MANAGER_H

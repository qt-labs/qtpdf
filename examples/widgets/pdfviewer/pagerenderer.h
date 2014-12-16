#ifndef PAGECACHE_H
#define PAGECACHE_H

#include <QBrush>
#include <QHash>
#include <QPixmap>
#include <QRunnable>
#include <QThread>
#include <QPdfDocument>
#include <QNetworkAccessManager>

class QPdfDocument;

class PageRenderer : public QObject
{
    Q_OBJECT
public:
    PageRenderer();
    ~PageRenderer();

public slots:
    void openDocument(const QUrl &location);
    void requestPage(int page, qreal zoom);

signals:
    void pageSizesAvailable(const QVector<QSizeF> &sizes);
    void pageReady(int page, qreal zoom, QImage image);

private slots:
    void reportPageSizes();
    void loadDocumentImpl(const QUrl &url);
    void requestPageImpl(int page, qreal zoom);
    void renderPages();
    void renderPageIfRequested(int page);
    void handleNetworkRequestError();

private:
    QThread m_workerThread;
    QPdfDocument m_doc;

    struct RenderRequest {
        int page;
        qreal zoom;
    };
    QVector<RenderRequest> m_renderRequests;
    QNetworkAccessManager *m_networkAccessManager;

    // performance statistics
    qreal m_minRenderTime;
    qreal m_maxRenderTime;
    qreal m_totalRenderTime;
    int m_totalPagesRendered;
};

#endif // PAGECACHE_H

#include "pagerenderer.h"
#include <QPainter>
#include <QPdfDocument>
#include <QLoggingCategory>
#include <QElapsedTimer>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

Q_DECLARE_LOGGING_CATEGORY(lcExample)

PageRenderer::PageRenderer()
    : m_networkAccessManager(new QNetworkAccessManager(this))
    , m_minRenderTime(1000000000.)
    , m_maxRenderTime(0.)
    , m_totalRenderTime(0.)
    , m_totalPagesRendered(0)
{
    qRegisterMetaType<QVector<QSizeF> >();

    m_workerThread.start();
    moveToThread(&m_workerThread);

    connect(&m_doc, SIGNAL(documentLoadStarted()), this, SLOT(reportPageSizes()));
    connect(&m_doc, SIGNAL(pageAvailable(int)), this, SLOT(renderPageIfRequested(int)));
}

PageRenderer::~PageRenderer()
{
    m_workerThread.exit();
}

void PageRenderer::openDocument(const QUrl &location)
{
    QMetaObject::invokeMethod(this, "loadDocumentImpl", Qt::QueuedConnection, Q_ARG(QUrl, location));
}

void PageRenderer::requestPage(int page, qreal zoom)
{
    QMetaObject::invokeMethod(this, "requestPageImpl", Qt::QueuedConnection, Q_ARG(int, page), Q_ARG(qreal, zoom));
}

void PageRenderer::reportPageSizes()
{
    QVector<QSizeF> sizes;
    for (int i = 0, count = m_doc.pageCount(); i < count; ++i)
        sizes << m_doc.pageSize(i);
    emit pageSizesAvailable(sizes);
    if (!m_renderRequests.isEmpty())
        QMetaObject::invokeMethod(this, "renderPages", Qt::QueuedConnection);
}

void PageRenderer::loadDocumentImpl(const QUrl &url)
{
    m_renderRequests.clear();
    QNetworkReply *reply = m_networkAccessManager->get(QNetworkRequest(url));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleNetworkRequestError()));
    m_doc.load(reply);
    connect(&m_doc, SIGNAL(documentLoadFinished()), reply, SLOT(deleteLater()));
}

void PageRenderer::requestPageImpl(int page, qreal zoom)
{
    RenderRequest newRequest;
    newRequest.page = page;
    newRequest.zoom = zoom;
    m_renderRequests << newRequest;
    if (!m_doc.isLoading())
        QMetaObject::invokeMethod(this, "renderPages", Qt::QueuedConnection);
}

void PageRenderer::renderPages()
{
    for (int i = 0; i < m_renderRequests.count(); ++i) {
        if (!m_doc.canRender(m_renderRequests.at(i).page))
            continue;
        RenderRequest request = m_renderRequests.takeAt(i);

        QSizeF size = m_doc.pageSize(request.page) * request.zoom;
        QElapsedTimer timer; timer.start();
        QImage img = m_doc.render(request.page, size);
        qreal secs = timer.nsecsElapsed() / 1000000000.0;
        if (secs < m_minRenderTime)
            m_minRenderTime = secs;
        if (secs > m_maxRenderTime)
            m_maxRenderTime = secs;
        m_totalRenderTime += secs;
        ++m_totalPagesRendered;
        emit pageReady(request.page, request.zoom, img);

        qCDebug(lcExample) << "page" << request.page << "zoom" << request.zoom << "size" << size << "in" << secs <<
                              "secs; min" << m_minRenderTime <<
                              "avg" << m_totalRenderTime / m_totalPagesRendered <<
                              "max" << m_maxRenderTime;
        break;
    }
}

void PageRenderer::renderPageIfRequested(int page)
{
    renderPages();
}

void PageRenderer::handleNetworkRequestError()
{
    qDebug() << static_cast<QNetworkReply*>(sender())->errorString();
}

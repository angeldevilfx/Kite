#include "browser_tab.h"

#include <QVBoxLayout>
#include <QWebEngineView>

BrowserTab::BrowserTab(QWidget *parent)
    : QWidget(parent), m_webView(new QWebEngineView(this)) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_webView);

    connect(m_webView, &QWebEngineView::titleChanged, this, &BrowserTab::titleChanged);
    connect(m_webView, &QWebEngineView::urlChanged, this, &BrowserTab::urlChanged);
    connect(m_webView, &QWebEngineView::loadStarted, this, &BrowserTab::loadStarted);
    connect(m_webView, &QWebEngineView::loadFinished, this, &BrowserTab::loadFinished);
}

QWebEngineView *BrowserTab::webView() const {
    return m_webView;
}

void BrowserTab::load(const QUrl &url) {
    m_webView->load(url);
}

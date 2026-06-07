#pragma once

#include <QUrl>
#include <QWidget>

class QWebEngineView;

class BrowserTab : public QWidget {
    Q_OBJECT

public:
    explicit BrowserTab(QWidget *parent = nullptr);

    QWebEngineView *webView() const;
    void load(const QUrl &url);

signals:
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void loadStarted();
    void loadFinished(bool ok);

private:
    QWebEngineView *m_webView;
};

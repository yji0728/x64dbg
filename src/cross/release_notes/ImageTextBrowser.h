#pragma once

#include <QTextBrowser>
#include <QTextCursor>
#include <QTimer>

class ImageTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit ImageTextBrowser(QWidget* parent = nullptr);
    void resizeImages();

protected:
    QVariant loadResource(int type, const QUrl & name) override;

private:
    qreal mSavedScrollPercentage = 0.0;
    QTimer* mResizeTimer = nullptr;
};

#pragma once

#include <QDialog>

namespace Ui
{
    class ReleaseNotesDialog;
}

class ReleaseNotesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReleaseNotesDialog(QWidget* parent = nullptr);
    ~ReleaseNotesDialog();
    bool setMarkdown(QString markdown, const QString & issueUrl);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::ReleaseNotesDialog* ui;
};

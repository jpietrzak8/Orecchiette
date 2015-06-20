#ifndef OREMJPEGDIALOG_H
#define OREMJPEGDIALOG_H

#include <QDialog>

namespace Ui {
class OreMJpegDialog;
}

class OreMJpegDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit OreMJpegDialog(QWidget *parent = 0);
  ~OreMJpegDialog();

  QString getUrl();
  
private:
  Ui::OreMJpegDialog *ui;
};

#endif // OREMJPEGDIALOG_H

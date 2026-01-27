#ifndef FRAMEDEVICESHOW_H
#define FRAMEDEVICESHOW_H

#include <QFrame>

namespace Ui {
class FrameDeviceShow;
}

class FrameDeviceShow : public QFrame
{
    Q_OBJECT

public:
    explicit FrameDeviceShow(QWidget *parent = nullptr);
    ~FrameDeviceShow();

    void setImage(const QString & image);
    void setName(const QString & name);
    void setSelect(bool select=true);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::FrameDeviceShow *ui;
};

#endif // FRAMEDEVICESHOW_H

#ifndef FRAMEDEVICESHOW_H
#define FRAMEDEVICESHOW_H

#include <QFrame>
#include <QScrollArea>

namespace Ui {
class FrameDeviceShow;
}

class FrameDeviceShow : public QFrame
{
    Q_OBJECT

public:
    explicit FrameDeviceShow(QWidget *parent = nullptr);
    ~FrameDeviceShow();

    static FrameDeviceShow *getFrameShow(int index,QWidget *parent);
    void setImage(const QString & image);
    void setName(const QString & name);
    void setSelect(bool select=true);

    QScrollArea *m_sa = nullptr;
    void *m_device = nullptr;
    bool m_bConacted = false;

signals:
    void onClicked(void *device);

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::FrameDeviceShow *ui;
};

#endif // FRAMEDEVICESHOW_H

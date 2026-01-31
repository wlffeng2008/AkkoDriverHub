#include "FrameDeviceShow.h"
#include "ui_FrameDeviceShow.h"

#include <QScrollBar>
#include <QWheelEvent>

static FrameDeviceShow *s_active = nullptr;

FrameDeviceShow *FrameDeviceShow::getFrameShow(int index, QWidget *parent)
{
    static FrameDeviceShow *pGroup[256] = {0};
    FrameDeviceShow *pFrame = pGroup[index];
    if (!pFrame)
    {
        pFrame = new FrameDeviceShow(parent);
        pGroup[index] = pFrame;
    }

    return pFrame;
}

FrameDeviceShow::FrameDeviceShow(QWidget *parent)
    : QFrame(parent), ui(new Ui::FrameDeviceShow) {
    ui->setupUi(this);

    if (!s_active)
        s_active = this;
    setSelect(false);
}

FrameDeviceShow::~FrameDeviceShow() { delete ui; }

void FrameDeviceShow::setImage(const QString &image)
{
    QPixmap Img(image);
    if (!Img.isNull())
    {
        int imgW = Img.width();
        int imgH = Img.height();

        int nW = this->width() - 20;
        int nH = this->height() - 100;

        qreal ir = imgW * 1.0 / imgH;
        if (ir > 1)
        {
            int nSH = nW / ir;
            if(nSH > nH)
            {
                nSH = nH;
                nW  = nH * ir;
            }
            ui->labelImage->setFixedSize(nW, nSH);
        }
        else
        {
            int nSW = nH * ir;
            if(nSW > nW)
            {
                nSW = nW;
                nH = nW / ir;
            }
            ui->labelImage->setFixedSize(nSW, nH);
        }
    }

    ui->labelImage->setScaledContents(true);
    ui->labelImage->setPixmap(Img);
}

void FrameDeviceShow::setName(const QString &name) {
    ui->labelName->setText(name);
}

void FrameDeviceShow::setSelect(bool select)
{
    if (select)
        setStyleSheet("#FrameDeviceShow{background-color:#E0E0E0; border-radius:32px; border: 1px solid #B0B0B0;}");
    else
        setStyleSheet("#FrameDeviceShow{background-color:#F4F4F4; border-radius:32px; border: 1px solid transparent;}");
    update();
}

void simulateMouseClick(QWidget *targetWgt, QPoint pos, Qt::MouseButton button = Qt::LeftButton)
{
    QMouseEvent pressEvt(QEvent::MouseButtonPress, pos, button, button, Qt::NoModifier);
    QApplication::sendEvent(targetWgt, &pressEvt);

    QMouseEvent releaseEvt(QEvent::MouseButtonRelease, pos, button, button, Qt::NoModifier);
    QApplication::sendEvent(targetWgt, &releaseEvt);
}

bool FrameDeviceShow::event(QEvent *event) {
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent *>(event);
        QScrollBar *pSB = m_sa->horizontalScrollBar();
        pSB->setValue(pSB->value() + we->angleDelta().y());
        return true;
    }

    if (event->type() == QEvent::Enter)
    {
        setSelect(true);
    }

    if (event->type() == QEvent::Leave)
    {
        setSelect(false);
    }

    if (event->type() == QEvent::MouseButtonRelease)
    {
        emit onClicked(m_device);
    }

    return QFrame::event(event);
}

void FrameDeviceShow::mousePressEvent(QMouseEvent *event)
{
    if (s_active)
        s_active->setSelect(false);

    s_active = this;
    setSelect();
}

#include "FrameDeviceShow.h"
#include "ui_FrameDeviceShow.h"

static FrameDeviceShow *s_active = nullptr;

FrameDeviceShow::FrameDeviceShow(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::FrameDeviceShow)
{
    ui->setupUi(this);

    if(!s_active) s_active = this;
    setSelect(false);
}

FrameDeviceShow::~FrameDeviceShow()
{
    delete ui;
}

void FrameDeviceShow::setImage(const QString & image){
    ui->labelImage->setScaledContents(false);
    QPixmap Img = QPixmap(image);
    // if(pImg)
    // {
    //     int imgW = pImg->width();
    //     int imgH = pImg->height();

    //     int nW = this->width();
    //     int nH = this->height() - 60;
    //     qreal xr = nW *1.0/imgW;
    //     qreal yr = nH *1.0/imgH;
    //     qreal lr = fmax(xr,yr);

    //     qDebug() << xr << yr;
    //     ui->labelImage->setFixedSize(nW*lr,nH*lr);
    // }
    ;
    ui->labelImage->setPixmap(Img.scaled(ui->labelImage->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

void FrameDeviceShow::setName(const QString & name)
{
    ui->labelTitle1->setText(name);
}

void FrameDeviceShow::setSelect(bool select)
{
    if(select)
        setStyleSheet("background-color:#E0E0E0; border-radius:12px;");
    else
        setStyleSheet("background-color:#E0E0E0; border-radius:12px;");
    update();
}

void FrameDeviceShow::mousePressEvent(QMouseEvent *event)
{
    if(s_active)
        s_active->setSelect(false);

    s_active = this;
    setSelect();
}

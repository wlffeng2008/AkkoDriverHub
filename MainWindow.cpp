#include "MainWindow.h"
#include "ModuleLangMenu.h"
#include "ui_MainWindow.h"

#include "FrameDeviceShow.h"

#include <QStyleOption>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPainter>
#include <QLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint|Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("QMainWindow{background-color: rgba(255, 255, 255, 1); border: 1px solid skyblue; border-radius: 12px; }");

    m_pLangMenu = new ModuleLangMenu(this) ;
    connect(m_pLangMenu,&ModuleLangMenu::onLangChanged,this,[=](int langId,const QString&lang){
        ui->pushButtonLang->setText(QString(" ")+lang) ;
    }) ;

    connect(ui->pushButtonLang,&QPushButton::clicked,this,[=]{
        ModuleLangMenu *pLangMenu = m_pLangMenu ;
        if(!pLangMenu->isHidden())
            return ;

        pLangMenu->setParent(nullptr);
        QRect btnRect = ui->pushButtonLang->geometry();
        QPoint PT = mapToGlobal(btnRect.bottomLeft());
        pLangMenu->setGeometry(PT.x(),PT.y(),110,250);
        pLangMenu->setWindowFlags(Qt::FramelessWindowHint |Qt::WindowStaysOnTopHint|Qt::Tool|Qt::Dialog|Qt::Popup);
        pLangMenu->show();
    });

    for(int i=0; i<2; i++)
    {
        FrameDeviceShow *device = new FrameDeviceShow(this);
        device->setFixedWidth(270);
        device->setImage(rand()%2 ? "./images/mouse1.png" : "./images/mouse2.png");
        if(i==0)
        {
            device->setImage("./images/kb.png");
            device->setFixedWidth(900);
        }
        device->setName("Akko 8K Mouse");
        ui->scrollAreaWidgetContents->layout()->addWidget(device);
    }

    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 重置全局滚轮滚动行数（避免被设为0）
    QApplication::setWheelScrollLines(3); // 默认值，每行滚动3行

    // 确保 viewport() 未被遮挡，且可接收事件
    ui->scrollArea->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    ui->scrollArea->viewport()->setFocusPolicy(Qt::StrongFocus);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    p.setRenderHint(QPainter::Antialiasing, true);

    int borderRadius = 12;
    QPainterPath path;
    path.addRoundedRect(this->rect(), borderRadius, borderRadius);

    p.setClipPath(path);
    p.drawImage(this->rect(),QImage("./images/MainPicture.png"));

    QMainWindow::paintEvent(event);
}

void MainWindow::on_pushButtonExit_clicked()
{
    qApp->exit();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    m_pLangMenu->hide() ;
    if (event->button() == Qt::LeftButton)
    {
        if(event->pos().y() < 300)
        {
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
            m_dragging = true;
            return ;
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_dragging) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
    // QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
    // QMainWindow::mouseReleaseEvent(event);
}

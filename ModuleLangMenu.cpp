#include "ModuleLangMenu.h"
#include "ui_ModuleLangMenu.h"
#include <QFocusEvent>
#include <QButtonGroup>
#include <QPushButton>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>

ModuleLangMenu::ModuleLangMenu(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ModuleLangMenu)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint |Qt::WindowStaysOnTopHint|Qt::Tool|Qt::Dialog|Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);

    static QStringList Langs={
        tr("中文简体"),
        tr("English"),
        tr("中文繁体"),
        tr("日本語"),
        tr("한국어"),
        tr("Русский"),
        tr("Tiếng Việt"),
        tr("Português"),
        tr("ภาษาไทย"),
        tr("Deutsch"),
        tr("Français"),
        tr("Svenska"),
        tr("Italiano"),
        tr("Türkçe")
    } ;

    QString strStyle(R"(

            QPushButton {
                width: 86px;
                height: 24px;
                border-radius: 10px ;

                color: #333;
                border: 1px soild white;
                background: white; }

            QPushButton:hover {
                border: 1px soild #F0F0F0;
                background: #F0F0F0; }

            QPushButton:checked {
                color: white;
                border: 1px soild #6329B6;
                background: #6329B6; }
            )") ;

    m_pBtnGrp = new QButtonGroup(this);
    QLayout *pLayout = ui->verticalLayout;
    pLayout->setSpacing(8);
    pLayout->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    pLayout->setContentsMargins(3,10,3,10);

    connect(m_pBtnGrp,&QButtonGroup::idClicked,this,[=](int id){
        hide() ;
        emit onLangChanged(id,Langs[id]);
    });

    for(int i=0; i<Langs.count(); i++)
    {
        QPushButton *btn = new QPushButton(Langs[i],this) ;
        btn->setFixedSize(90,24);
        btn->setCheckable(true);
        btn->setStyleSheet(strStyle);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setCursor(Qt::PointingHandCursor);

        m_pBtnGrp->addButton(btn,i);
        pLayout->addWidget(btn);
    }
    //pLayout->setAlignment(Qt::AlignTrailing|Qt::AlignHCenter);

    QTimer::singleShot(100,this,[=]{ m_pBtnGrp->button(0)->click();});

    setStyleSheet("QDialog{background-color: rgba(255, 255, 255, 0.8); border: 1px solid #EAEAEA; border-radius:12px;}");

    installEventFilter(this);
    setFocusPolicy(Qt::StrongFocus);
}

void ModuleLangMenu::showEvent(QShowEvent *event)
{
    event->accept();
}

ModuleLangMenu::~ModuleLangMenu()
{
    delete ui;
}

void ModuleLangMenu::setLanguage(int langId)
{
    if(langId>6)
        return ;

    m_pBtnGrp->button(langId)->click();
}

bool ModuleLangMenu::eventFilter(QObject *watch, QEvent *event)
{
    QEvent::Type type = event->type();
    //qDebug() << type ;
    if(type == QEvent::WindowDeactivate || type== QEvent::FocusOut)
    {
        this->hide();
    }

    return QDialog::eventFilter(watch, event);
}

bool ModuleLangMenu::event(QEvent *event)
{
    QEvent::Type type = event->type();
    //qDebug() << type ;
    if(type == QEvent::WindowDeactivate || type== QEvent::FocusOut)
    {
        this->hide();
    }

    return QDialog::event( event);
}

void ModuleLangMenu::changeEvent(QEvent *event)
{
    QEvent::Type type = event->type();
    // qDebug() << type;
    if(type == QEvent::WindowDeactivate || type== QEvent::FocusOut)
    {
        this->hide();
    }

    QDialog::changeEvent(event);
}


void ModuleLangMenu::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QDialog::paintEvent(event);
}

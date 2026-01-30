#include "MainWindow.h"
#include "ModuleLangMenu.h"
#include "ui_MainWindow.h"

#include "FrameDeviceShow.h"

#include "hidapi.h"
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStyleOption>
#include <QThread>
#include <QTimer>
#include <QWheelEvent>
#include <QProcess>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static HWND s_hWndEmb = NULL;
static QSettings settings("HKEY_CURRENT_USER\\Software\\Akko",QSettings::NativeFormat);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint |
                   Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("QMainWindow{background-color: rgba(255, 255, 255, 1); border: "
                  "1px solid skyblue; border-radius: 12px; }");

    m_pLangMenu = new ModuleLangMenu(this);
    connect(m_pLangMenu, &ModuleLangMenu::onLangChanged, this, [=](int langId, const QString &lang) {
        ui->pushButtonLang->setText(QString(" ") + lang);
        int id = langId;
        switch(langId)
        {
        case 0:
        case 1:
            break;
        default:
            id=2;
            break;

        }
        int langs[3] = {2052,1033,1049} ;
        settings.setValue("LANGUAGE",langs[id]);
    });

    connect(ui->pushButtonLang, &QPushButton::clicked, this, [=] {
        ModuleLangMenu *pLangMenu = m_pLangMenu;
        if (!pLangMenu->isHidden())
            return;

        pLangMenu->setParent(nullptr);
        QRect btnRect = ui->pushButtonLang->geometry();
        QPoint PT = mapToGlobal(btnRect.bottomLeft());
        pLangMenu->setGeometry(PT.x(), PT.y(), 110, 250);
        pLangMenu->setWindowFlags(Qt::FramelessWindowHint |
                                  Qt::WindowStaysOnTopHint | Qt::Tool | Qt::Dialog |
                                  Qt::Popup);
        pLangMenu->show();
    });

    m_layout = ui->scrollAreaWidgetContents->layout();

    for (int i = 0; i < 0; i++)
    {
        FrameDeviceShow *device = new FrameDeviceShow(this);
        device->setFixedWidth(280);
        device->setName("Akko 8K Mouse");
        device->setImage(rand() % 2 ? "./images/mouse1.png"
                                    : "./images/mouse2.png");
        if (i == 0) {
            device->setFixedWidth(920);
            device->setImage("./images/kb.png");
            device->setName("Akko Gaming Keyboard");
        }
        m_layout->addWidget(device);
    }

    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QTimer *pCheck = new QTimer(this);
    connect(pCheck, &QTimer::timeout, this, [=] {
        bool toHide = m_layout->count() == 0;

        static bool last = false;
        if (last == toHide) {
            ui->scrollArea->setHidden(toHide);
            ui->labelLogo->setHidden(toHide);
            ui->labelPrev->setHidden(toHide);
            ui->labelNext->setHidden(toHide);
            ui->labelLarge1->setHidden(toHide);
            ui->pushButtonScan->setHidden(toHide);
            ui->labelLarge->setHidden(!toHide);
            update();
        }

        last = toHide;
    });
    pCheck->start(100);

    ui->scrollArea->viewport()->installEventFilter(this);

    ui->labelPrev->installEventFilter(this);
    ui->labelNext->installEventFilter(this);

    enumDevice();
    connect(ui->pushButtonScan, &QPushButton::clicked, this,
            [=] { enumDevice(); });

    QTimer *pTMUsb = new QTimer(this);
    static USBNotifier *pUsb = new USBNotifier(this);
    QCoreApplication::instance()->installNativeEventFilter(pUsb);
    connect(pUsb, &USBNotifier::devicePluggined,this,[=](bool in) {
        pTMUsb->stop();
        pTMUsb->start(800);
    });

    connect(pTMUsb,&QTimer::timeout,this,[=]{
        pTMUsb->stop();
        ui->pushButtonScan->click();
    });

    settings.setValue("AkkoReturn", 0);
    settings.setValue("AkkoWnd", 0);
    ui->frameEmb->hide();
    ui->frameEmb->setStyleSheet("background-color: rgb(27, 27, 27);");

    QTimer *pTMRet = new QTimer(this);
    pTMRet->start(100);
    connect(pTMRet,&QTimer::timeout,this,[=]{
        if(settings.value("AkkoReturn", 0).toInt() == 1)
        {
            settings.setValue("AkkoReturn", 0);
            ui->frameEmb->hide();
        };
    });

    QProcess::startDetached("akko.exe", QStringList{"/super"});

    QTimer *pTMFindWnd = new QTimer(this);
    pTMFindWnd->start(100);

    connect(pTMFindWnd,&QTimer::timeout,this,[=]{
        ui->frameEmb->hide();
        if(s_hWndEmb)
        {
            HWND hParentWnd = (HWND)ui->frameEmb->winId();
            ::SetParent(s_hWndEmb,hParentWnd);
            ::SetWindowPos(s_hWndEmb, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW| SWP_NOSIZE);
            static int nCount = 0;
            if(nCount ++ > 10)
            {
                pTMFindWnd->stop();
            }
            return ;
        }

        //HWND hWnd = (HWND)settings.value("AkkoWnd", 0).toUInt();
        HWND hWnd = ::FindWindow((LPCWSTR)QString("MainWnd").utf16(), (LPCWSTR)QString("Akko").utf16());
        if(!hWnd) hWnd = (HWND)(settings.value("AkkoWnd", 0).toUInt());

        if(hWnd)
        {
            s_hWndEmb = hWnd;
            pTMFindWnd->stop();
            pTMFindWnd->start(100);
            {
                LONG style = ::GetWindowLongPtr(hWnd, GWL_STYLE);
                style &= ~WS_CAPTION;    //
                style &= ~WS_VISIBLE;    //
                style &= ~WS_POPUP  ;    // 移除标题栏
                style &= ~WS_BORDER;     // 移除边框
                style &= ~WS_THICKFRAME; // 移除可调整大小边框
                style &= ~WS_MAXIMIZEBOX;// 移除最大化按钮
                style &= ~WS_MINIMIZEBOX;// 移除最小化按钮
                ::SetWindowLongPtr(hWnd, GWL_STYLE, style|WS_CHILD);

                LONG exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
                exStyle &= ~WS_EX_DLGMODALFRAME;
                ::SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);

                ::SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0,  SWP_NOSIZE | SWP_HIDEWINDOW);
                ui->frameEmb->hide();
            }
        }
    });

}

typedef struct {
    quint32 id;
    quint16 VID;
    quint16 PID;
    quint8 type; // 0 keyboard 1 mouse 2 headphone
    QString name;

} AkkoDeviceInfo;

static QList<AkkoDeviceInfo> s_AkkoDeviceTable = {
    // 项目3632
    {  69, 0x0461, 0x4002, 0, "3098B"},
    {  96, 0x0461, 0x4002, 0, "5108B"},
    {  94, 0x0461, 0x4002, 0, "5087B"},
    { 138, 0x0461, 0x4002, 0, "3098S"},
    { 138, 0x0461, 0x4002, 0, "AK980"},
    { 884, 0x3151, 0x4010, 0, "ICE1"},
    { 175, 0x0461, 0x4002, 0, "AK67(MOD005)"},
    { 189, 0x0461, 0x4002, 0, "AK75(MOD007)"},
    { 235, 0x0461, 0x4002, 0, "AK65(MOD008)"},
    { 217, 0x0461, 0x4002, 0, "3084B"},
    { 511, 0x3151, 0x4003, 0, "3098B Plus"},
    { 306, 0x0461, 0x4003, 0, "3084B Plus"},
    { 414, 0x3151, 0x4003, 0, "3084B Plus"},
    {1114, 0x3151, 0x4003, 0, "3084B Plus"},
    { 250, 0x0461, 0x4003, 0, "5108B Plus"},
    { 406, 0x3151, 0x4003, 0, "5087B Plus"},
    {1515, 0x3151, 0x4003, 0, "5087B Plus"},
    { 548, 0x25A7, 0x2301, 0, "3108 rf US"},
    { 567, 0x25A7, 0x2301, 0, "3087 rf"},
    { 344, 0x3151, 0x4003, 0, "ACR Pro75"},
    { 345, 0x0461, 0x4003, 0, "ACR Pro68"},
    { 372, 0x0461, 0x4003, 0, "Alice"},
    { 352, 0x0461, 0x4002, 0, "MOD007S"},
    { 313, 0x0461, 0x4002, 0, "MOD006/AK84"},
    { 338, 0x0461, 0x4003, 0, "5075B Plus"},
    { 281, 0x0461, 0x4003, 0, "PC75B Plus"},
    { 413, 0x3151, 0x4003, 0, "PC75B Plus"},
    { 358, 0x0461, 0x4003, 0, "PC75S"},
    { 424, 0x3151, 0x4003, 0, "3068B Plus"},
    {1457, 0x3151, 0x4003, 0, "3068B Plus"},
    { 386, 0x0461, 0x4003, 0, "5108BEU Plus"},
    { 409, 0x3151, 0x4003, 0, "5087BP-ISO-UK"},
    { 447, 0x3151, 0x4003, 0, "3068BP-ISO(UK)"},
    {1536, 0x3151, 0x4003, 0, "3068B Plus  ABNT2"},
    //{468,0x普通,0,"PC21B Plus"},
    { 512, 0x3151, 0x4003, 0, "5098B Plus"},
    { 455, 0x3151, 0x4003, 0, "5108S"},
    { 456, 0x3151, 0x4003, 0, "3068S"},
    {2337, 0x3151, 0x4003, 0, "AMICIS65"},
    { 457, 0x3151, 0x4004, 0, "3084S"},
    { 458, 0x3151, 0x4005, 0, "5087S"},
    {1436, 0x3151, 0x4003, 0, "5087S"},
    { 459, 0x3151, 0x4006, 0, "5075S"},
    {1452, 0x3151, 0x4003, 0, "5075S"},
    {1594, 0x3151, 0x4003, 0, "5075S 俄罗斯版本"},
    { 704, 0x3151, 0x4003, 0, "007 PRO"},
    { 575, 0x3151, 0x4003, 0, "MG108"},
    { 666, 0x05AC, 0x024F, 0, "SPR67"},
    { 594, 0x3151, 0x4003, 0, "PC75B Plus-S"},
    { 568, 0x3151, 0x4003, 0, "Alice-S"},
    { 606, 0x3151, 0x4003, 0, "ACR Pro75-S"},
    {1408, 0x3151, 0x4003, 0, "TOP75"},
    { 598, 0x3151, 0x4003, 0, "5075B Plus-S"},
    {1528, 0x3151, 0x4003, 0, "5075B Plus-S"},
    { 609, 0x3151, 0x4003, 0, "MG75 RGB"},
    { 603, 0x3151, 0x4003, 0, "3061SEU"},
    { 679, 0x3151, 0x4003, 0, "PC98B Plus-S"},
    { 630, 0x3151, 0x4003, 0, "ACR Pro68-S"},
    { 570, 0x3151, 0x4003, 0, "5087SEU"},
    { 620, 0x3151, 0x4003, 0, "5075S-S"},
    { 654, 0x3151, 0x4003, 0, "3096B Plus"},
    { 652, 0x3151, 0x4003, 0, "5108S白光版"},
    { 658, 0x05AC, 0x024F, 0, "PC75B Plus-S (MAC白光版)"},
    {1374, 0x05AC, 0x024F, 0, "PC75B Plus-S (MAC白光版)"},
    { 676, 0x05AC, 0x024F, 0, "5087B Plus-S (MAC白光版)"},
    { 677, 0x05AC, 0x024F, 0, "PC98B Plus-S (MAC白光版)"},
    { 668, 0x3151, 0x4003, 0, "PC75S-S"},
    { 678, 0x3151, 0x4003, 0, "5108B Plus白光版"},
    { 695, 0x3151, 0x4003, 0, "PC75B Plus-S白光版"},
    { 710, 0x3151, 0x4003, 0, "PC75S-S白光版"},
    { 744, 0x3151, 0x4003, 0, "3068BP-ISO(DE)"},
    { 745, 0x3151, 0x4003, 0, "3068BP-ISO(NE)"},
    { 812, 0x3151, 0x4010, 0, "5108"},
    { 811, 0x3151, 0x4010, 0, "MG75"},
    { 771, 0x3151, 0x4003, 0, "3084BP-ISO-NE"},
    {2950, 0x3151, 0x4003, 0, "3084BP-ISO-DE"},
    { 826, 0x3151, 0x4003, 0, "5087-ISO-NE"},
    {2946, 0x3151, 0x4003, 0, "5087-ISO-DE"},
    { 773, 0x3151, 0x4003, 0, "5108BP-ISO-NE (旧灯控)"},
    {1868, 0x3151, 0x4003, 0, "5108BP-ISO-NE (新灯控)"},
    { 860, 0x25A7, 0x2302, 0, "AK108"},
    //{854,0x普通,0,"ACR TOP101"},
    { 821, 0x3151, 0x4003, 0, "5075B Plus-S白光版"},
    { 868, 0x3151, 0x4010, 0, "ACR TOP40"},
    { 848, 0x3151, 0x4003, 0, "SPR75"},
    { 848, 0x3151, 0x4003, 0, "SPR75-B"},
    { 859, 0x3151, 0x4002, 0, "M1"},
    { 870, 0x3151, 0x4010, 0, "M1"},
    {3153, 0x3151, 0x4010, 0, "K75-A"},
    {3368, 0x38EE, 0x000A, 0, "5075-A (5075S)"},
    {1134, 0x3151, 0x4010, 0, "M1"},
    {1340, 0x3151, 0x4010, 0, "M1 Pro"},
    { 885, 0x3151, 0x4010, 0, "5075S-ABNT2"},
    { 896, 0x25A7, 0x2301, 0, "MG75W"},
    {1555, 0x25A7, 0x2301, 0, "MG75W-A"},
    { 898, 0x25A7, 0x2301, 0, "MG108W"},
    { 898, 0x25A7, 0x2301, 0, "MG108W"},
    { 899, 0x3151, 0x4010, 0, "M2"},
    { 912, 0x3151, 0x4010, 0, "M5"},
    { 904, 0x3151, 0x4010, 0, "M3"},
    { 976, 0x3151, 0x4010, 0, "ACR TOP101-S"},
    { 978, 0x3151, 0x4003, 0, "3098BP-ISO-DE"},
    { 979, 0x3151, 0x4003, 0, "5087BP-ISO-DE"},
    { 980, 0x3151, 0x4003, 0, "5087BP-ISO-NE"},
    { 984, 0x3151, 0x4010, 0, "5075S-ISO-UK"},
    { 985, 0x3151, 0x4010, 0, "5075S-ISO-DE"},
    { 986, 0x3151, 0x4010, 0, "5075S-ISO-NE"},
    {1033, 0x3151, 0x4003, 0, "5075B Plus-S MAC白光版"},
    {1033, 0x3151, 0x4003, 0, "5075B Plus-S MAC白光版"},
    {1122, 0x25A7, 0x2301, 0, "3098RF"},
    {1101, 0x3151, 0x4010, 0, "M6"},
    {1205, 0x3151, 0x4010, 0, "M6"},
    {1073, 0x3151, 0x4010, 0, "MG108"},
    {1145, 0x3151, 0x4003, 0, "5108B Plus"},
    {1148, 0x3151, 0x4003, 0, "5108B Plus白光版"},
    {1146, 0x3151, 0x4003, 0, "5108S"},
    {1147, 0x3151, 0x4003, 0, "5108S白光版"},
    {1264, 0x3151, 0x4003, 0, "ACR Pro75-S"},
    {1267, 0x3151, 0x4002, 0, "3098B"},
    {1280, 0x3151, 0x4003, 0, "3098B Plus"},
    // 3121 SOC项目
    { 969, 0x3151, 0x4015, 0, "MG108B"},
    {1027, 0x3151, 0x4015, 0, "MG75M"},
    { 983, 0x3151, 0x4015, 0, "M1W RGB"},
    {3154, 0x3151, 0x4010, 0, "K75-B"},
    {3369, 0x38EE, 0x000A, 0, "5075-B (5075S)"},
    {1827, 0x3151, 0x4010, 0, "M1-B"},
    {1171, 0x3151, 0x4015, 0, "M1W"},
    {1413, 0x3151, 0x4015, 0, "M1W Pro"},
    {1204, 0x3151, 0x4015, 0, "M1W-ISO-UK"},
    {1272, 0x3151, 0x4015, 0, "M1W-ISO-UK"},
    {1691, 0x3151, 0x4015, 0, "MOD007B ISO"},
    //{1031,0x普通,0,"ICE1"},
    {1042, 0x3151, 0x4015, 0, "5075B Plus-S"},
    {2567, 0x3151, 0x4015, 0, "5075B Plus-S"},
    {1044, 0x3151, 0x4015, 0, "5075BP-ISO-NE"},
    {1170, 0x3151, 0x4015, 0, "5075BP-ISO-DE"},
    {1165, 0x3151, 0x4015, 0, "5075BP-ISO-UK"},
    {1023, 0x3151, 0x4015, 0, "5108B Plus白光版"},
    {1391, 0x3151, 0x4010, 0, "5108S白光版"},
    {1022, 0x3151, 0x4015, 0, "5087B Plus"},
    {1228, 0x3151, 0x4010, 0, "5087S"},
    {1064, 0x3151, 0x4010, 0, "6104 US"},
    {1068, 0x3151, 0x4010, 0, "3108S"},
    {1065, 0x3151, 0x4010, 0, "3087S (3087 V3)"},
    {1062, 0x3151, 0x4003, 0, "MOD007 V2-ISO-UK"},
    {1200, 0x3151, 0x4015, 0, "M3W"},
    {1219, 0x3151, 0x4015, 0, "M3W-ISO-UK"},
    {2073, 0x3151, 0x4015, 0, "M3W"},
    {2074, 0x3151, 0x4015, 0, ""},
    {1215, 0x3151, 0x4015, 0, "M7W(无侧灯)"},
    {1257, 0x3151, 0x4015, 0, "M7W(带侧灯)"},
    {1297, 0x3151, 0x4015, 0, "M7W-ISO-UK (带侧灯)"},
    {1291, 0x3151, 0x4015, 0, "5075B Plus-S"},
    {1281, 0x3151, 0x4015, 0, "3098B Plus"},
    {1282, 0x3151, 0x4015, 0, "3098BP-ISO-UK"},
    {1281, 0x3151, 0x4015, 0, "3098B Plus"},
    {1850, 0x3151, 0x4010, 0, "3098S"},
    {1301, 0x3151, 0x4015, 0, "M2W"},
    {1307, 0x3151, 0x4015, 0, "M2W-ISO-UK"},
    {1304, 0x3151, 0x4015, 0, "1570"},
    {1583, 0x3151, 0x4015, 0, "MOD007PC-CX"},
    {1308, 0x3151, 0x4015, 0, "M5W"},
    {1309, 0x3151, 0x4015, 0, "M5W-ISO-UK"},
    {1330, 0x3151, 0x4015, 0, "5087 Gasket"},
    {1495, 0x3151, 0x4010, 0, "5087S V2"},
    {1325, 0x3151, 0x4015, 0, "M1 HES"},
    {1400, 0x3151, 0x4015, 0, "M3W HES"},
    {1367, 0x3151, 0x4015, 0, "MOD007B-HE"},
    {1584, 0x3151, 0x4015, 0, "MOD007B-HE"},
    {1398, 0x3151, 0x4015, 0, "5098B"},
    {1395, 0x3151, 0x4015, 0, "MOD007B PC"},
    {1419, 0x3151, 0x4015, 0, "ICE75"},
    {1433, 0x3151, 0x4015, 0, "MG75S HES"},
    {1674, 0x3151, 0x4035, 0, "MG75S HES"},
    {1993, 0x3151, 0x4035, 0, "MG75S HES"},
    {1937, 0x3151, 0x4035, 0, "MG75B HE"},
    {1477, 0x3151, 0x4015, 0, "5087S-JP HE"},
    {1481, 0x3151, 0x4015, 0, "VSPO-007"},
    {1453, 0x3151, 0x4015, 0, "M1W HE"},
    {1498, 0x3151, 0x4015, 0, "MOD007B ISO HE"},
    {1499, 0x3151, 0x4015, 0, "M1W ISO HE"},
    {1501, 0x3151, 0x4015, 0, "M1 ISO HE"},
    {1500, 0x3151, 0x4015, 0, "MOD007 ISO HE"},
    {1522, 0x3151, 0x4015, 0, "M7 Pro"},
    {1487, 0x3151, 0x4015, 0, "5087B V2-JP HE"},
    {1488, 0x3151, 0x4015, 0, "5087S V2-JP HE"},
    {1599, 0x3151, 0x4015, 0, "5087S V2-JP HE"},
    {1540, 0x3151, 0x4035, 0, "3061S HE US"},
    {1540, 0x3151, 0x4035, 0, "3061S HE US奶黄轴"},
    {1791, 0x3151, 0x4035, 0, "3061S HE US樱粉轴"},
    {1783, 0x3151, 0x4035, 0, "3061S HE US高特大磁铁"},
    {2043, 0x3151, 0x4035, 0, "3061S ISO HE高特大磁铁"},
    {1574, 0x3151, 0x4035, 0, "5075S HE JP"},
    {1508, 0x3151, 0x4010, 0, "MG75S  ABNT2"},
    {1509, 0x3151, 0x4010, 0, "5075S  ABNT2"},
    {1510, 0x3151, 0x4010, 0, "3068S  ABNT2"},
    {1511, 0x3151, 0x4015, 0, "3068B Plus  ABNT2"},
    {1579, 0x3151, 0x4035, 0, "MOD007 ISO HE"},
    {1570, 0x3151, 0x4035, 0, "MOD007S HE"},
    {1604, 0x3151, 0x4035, 0, "M1W HE"},
    {3160, 0x3151, 0x4035, 0, "MOD007B HE-B"},
    {3161, 0x3151, 0x4035, 0, "MOD007B HE-C"},
    {1777, 0x3151, 0x4035, 0, "M1W ISO  HE"},
    {1778, 0x3151, 0x4035, 0, "MOD007B ISO  HE"},
    {1637, 0x3151, 0x4035, 0, "MOD007B V3 HE"},
    {1571, 0x3151, 0x4015, 0, "M2 Pro"},
    {1588, 0x3151, 0x4015, 0, "MU01"},
    {1696, 0x3151, 0x4015, 0, "MOD007B HE"},
    {1697, 0x3151, 0x4015, 0, "MOD007B ISO HE"},
    {1714, 0x3151, 0x4015, 0, "TOP75B"},
    {1714, 0x3151, 0x4015, 0, "TOP75B"},
    {1806, 0x3151, 0x4015, 0, "M1W-B"},
    {1798, 0x3151, 0x4015, 0, "MU01 UK"},
    {1890, 0x3151, 0x4035, 0, "5075B HE"},
    {1866, 0x3151, 0x4035, 0, "M3W HE"},
    {1866, 0x3151, 0x4035, 0, "M3W HE"},
    {1964, 0x3151, 0x4035, 0, "MU01 HE"},
    {1965, 0x3151, 0x4035, 0, "MU01 ISO HE"},
    {1878, 0x3151, 0x4035, 0, "3061S BR HE"},
    {1906, 0x3151, 0x4010, 0, "3061S BR"},
    {1934, 0x3151, 0x4010, 0, "3061S"},
    {1915, 0x3151, 0x4015, 0, "JIN01"},
    {1952, 0x3151, 0x4035, 0, "5087B V2-HE"},
    {1952, 0x3151, 0x4035, 0, "G87-HE"},
    {2015, 0x3151, 0x4010, 0, "MOD001S"},
    {1991, 0x3151, 0x4015, 0, "MU02"},
    {1923, 0x3151, 0x4015, 0, "5098B"},
    {2254, 0x3151, 0x4015, 0, "5098B DE)"},
    {1933, 0x3151, 0x4015, 0, "M1W V5"},
    {1988, 0x3151, 0x4015, 0, "M3W V5"},
    {1983, 0x3151, 0x4015, 0, "Mineral 02"},
    {2050, 0x3151, 0x4015, 0, "M2W V5"},
    {2260, 0x3151, 0x4015, 0, "MOD-K01"},
    {2402, 0x3151, 0x4010, 0, "AMICIS80"},
    {2735, 0x3151, 0x4010, 0, "TAC87"},
    {3207, 0x3151, 0x4010, 0, "TEST MODEL_2"},
    {3039, 0x3151, 0x4010, 0, "3108 V3"},
    {3296, 0x3151, 0x4010, 0, "3087S US"},
    // 高精度案子
    {2247, 0x3151, 0x5030, 0, "M1 V5 HE"},
    {2679, 0x3151, 0x5030, 0, "M1 V5 HE-B"},
    {2536, 0x3151, 0x5030, 0, "M1 V5 HE UK"},
    {2490, 0x3151, 0x5030, 0, "M1 V5 HE"},
    {2236, 0x3151, 0x5029, 0, "5087S V2 HE JP"},
    //{2263,0x普通,0,"5075S HE US"},
    //{2262,0x普通,0,"5075S HE UK"},
    {2365, 0x3151, 0x5029, 0, "5075S HE JP"},
    {2413, 0x3151, 0x502D, 0, "AMICIS80HE"},
    {2299, 0x3151, 0x502E, 0, "FUN60"},
    {2298, 0x3151, 0x502C, 0, "FUN60"},
    {2305, 0x3151, 0x5030, 0, "FUN60 PRO"},
    {2304, 0x3151, 0x502D, 0, "FUN60 PRO"},
    {2464, 0x3151, 0x502D, 0, "FUN60 PRO-R"},
    {2505, 0x3151, 0x502D, 0, "FUN60 PRO UK"},
    {2305, 0x3151, 0x5030, 0, "FUN60 PRO"},
    {2306, 0x3151, 0x5030, 0, "FUN60 MAX"},
    {2381, 0x3151, 0x502D, 0, "FUN60 Ultra"},
    {2506, 0x3151, 0x502D, 0, "FUN60 Ultra UK"},
    {2387, 0x3151, 0x5030, 0, "FUN60 Ultra"},
    {2352, 0x3151, 0x5029, 0, "FUN60 Ultra"},
    {2307, 0x3151, 0x5030, 0, "FUN60 Ultra"},
    {2543, 0x3151, 0x5029, 0, "FUN60 Ultra UK"},
    {2474, 0x3151, 0x5030, 0, "5087B V2 HE"},
    {2829, 0x3151, 0x502F, 0, "5087 V3 HE"},
    {3131, 0x3151, 0x5029, 0, "5087 V3 HE"},
    {2455, 0x3151, 0x5030, 0, "5087B V2 HE"},
    {2479, 0x3151, 0x5030, 0, "G87-HE Ultra"},
    {2473, 0x3151, 0x5030, 0, "5075B V2 HE"},
    {2807, 0x3151, 0x502F, 0, "5075 V3 HE"},
    {3129, 0x3151, 0x502D, 0, "5075 V3 HE"},
    {2452, 0x3151, 0x5030, 0, "5075B V2 HE"},
    {2453, 0x3151, 0x5030, 0, "MOD007 V5 HE"},
    {2643, 0x3151, 0x502F, 0, "FUN75"},
    {2648, 0x3151, 0x502D, 0, "FUN75"},
    {2782, 0x3151, 0x502D, 0, "TAC75 HE"},
    {2782, 0x3151, 0x502D, 0, "TAC75 HE"},
    {2839, 0x3151, 0x5030, 0, "MOD007 V5 HE"},
    {3113, 0x3151, 0x5029, 0, "MOD007 V5 HE"},
    {2601, 0x3151, 0x5030, 0, "M2 V5"},
    {2641, 0x3151, 0x5030, 0, "M2 V5 UK"},
    {2585, 0x3151, 0x5030, 0, "M3 V5"},
    {2640, 0x3151, 0x5030, 0, "M3 V5 UK"},
    {2581, 0x3151, 0x5030, 0, "Mineral 02"},
    {2619, 0x3151, 0x5030, 0, "Gem 02"},
    {3105, 0x3151, 0x5030, 0, "Armor Alice"},
    {2872, 0x3151, 0x5029, 0, "MOD007B V3-HE(UK)"},
    {2871, 0x3151, 0x5029, 0, "MOD007B V3-HE(US)"},
    {2704, 0x3151, 0x5029, 0, "MOD007S V3-HE UK"},
    {3322, 0x3151, 0x5029, 0, "MOD007S V3-HE UK-B"},
    {2683, 0x3151, 0x5029, 0, "MOD007S V3-HE US"},
    {3321, 0x3151, 0x5029, 0, "MOD007S V3-HE US-B"},
    {2785, 0x3151, 0x5030, 0, "FUN60 PRO-B"},
    {2600, 0x3151, 0x5029, 0, "FUN60 PRO-B"},
    {3299, 0x3151, 0x5030, 0, "FUN60 MAX-B"},
    {2721, 0x3151, 0x5030, 0, "K0038 (Verve68T)"},
    {3099, 0x3151, 0x5029, 0, "K0038 (Verve68)"},
    {2743, 0x3151, 0x5030, 0, "K0026 (RAY68)"},
    {2924, 0x3151, 0x5029, 0, "K0026 (RAY68)"},
    {2874, 0x3151, 0x5030, 0, "M3 V5"},
    {2845, 0x3151, 0x5030, 0, "M2 V5"},
    {2881, 0x3151, 0x5030, 0, "TAC87"},
    {3200, 0x3151, 0x5030, 0, "TAC87"},
    {2724, 0x3151, 0x5030, 0, "Shine60"},
    {2744, 0x3151, 0x5029, 0, "Shine60"},
    {2832, 0x3151, 0x5030, 0, "Shine60"},
    {2836, 0x3151, 0x5029, 0, "Shine60"},
    {2811, 0x3151, 0x5030, 0, "FUN68"},
    {3091, 0x3151, 0x5029, 0, "FUN68"},
    {3429, 0x38EE, 0x0009, 0, "FUN68 UK"},
    {3162, 0x3151, 0x5030, 0, "MOD68"},
    {3341, 0x38EE, 0x0009, 0, "MOD68 UK"},
    {3378, 0x38EE, 0x0009, 0, "MOD68 UK"},
    {3163, 0x3151, 0x5029, 0, "MOD68"},
    {3269, 0x3151, 0x5030, 0, "Shine68"},
    {3270, 0x3151, 0x5029, 0, "Shine68"},
    {3136, 0x3151, 0x5029, 0, "AMICIS65 HE"},
    {2819, 0x3151, 0x5030, 0, "M1 V5 HE"},
    {2903, 0x3151, 0x5030, 0, "Mineral 01"},
    {3275, 0x3151, 0x5030, 0, "Gem 01"},
    {2885, 0x3151, 0x5030, 0, "Mineral 02"},
    {3273, 0x3151, 0x5030, 0, "Gem 02"},
    {2949, 0x3151, 0x5030, 0, "M1 V5 HE"},
    {2995, 0x3151, 0x5030, 0, "MOD007 V5 HE"},
    {3052, 0x3151, 0x5029, 0, "FUN60 KizunaAI SE"},
    {3187, 0x3151, 0x5029, 0, "FUN87 KizunaAI PE"},
    {3359, 0x3151, 0x5030, 0, "5087 V3 HE"},
    {3357, 0x3151, 0x5030, 0, "5075 V3 HE"},
    {3300, 0x3151, 0x5030, 0, "M1 V5 HE"},
    {3397, 0x38EE, 0x0001, 0, "MOD68"},
    {3424, 0x38EE, 0x0001, 0, "5087 V3 HE TMR"},
    {3416, 0x38EE, 0x0001, 0, "5075 V3 HE TMR"},
    {3457, 0x38EE, 0x0006, 0, "Hitbox"},
    {3458, 0x3151, 0x504A, 0, "Hitbox"},
    // PAN1086
    // {2286,0x普通,0,"Mineral 02(微技机型KG118)"},
    // {2322,0x普通,0,"GEM 02(微技机型KG119)"},
    // {2295,0x普通,0,"GEM 01(微技机型KG123)"},
    // {2325,0x普通,0,"Mineral 01(微技机型KG126)"},
    // {2528,0x苹果,0,"KF105"},
    // {2529,0x苹果,0,"KF106"},
    // {2482,0x普通,0,"Air01"},
    {2655, 0x3151, 0x5025, 0, "Armor Alice"},
    // YC3123
    {2739, 0x3151, 0x5002, 0, "MG108B"},
    {3245, 0x3151, 0x5002, 0, "MG108B-B"},
    {2753, 0x3151, 0x5002, 0, "5075 V3"},
    {3127, 0x3151, 0x5002, 0, "5075 V3"},
    {2786, 0x3151, 0x5002, 0, "TAC87"},
    {3013, 0x3151, 0x5002, 0, "Air01"},
    {3452, 0x3151, 0x5002, 0, "Air01 UK"},
    {3031, 0x3151, 0x5002, 0, "KF105"},
    {3008, 0x3151, 0x5002, 0, "KF106"},
    {3034, 0x3151, 0x5002, 0, "MOD007 V5"},
    {3095, 0x3151, 0x5002, 0, "5087 V3"},
    {3130, 0x3151, 0x5002, 0, "5087 V3"},
    {3147, 0x3151, 0x5002, 0, "FUN68"},
    {3285, 0x3151, 0x5002, 0, "5108 V5"},
    // RY6609 双8K
    {3265, 0x3151, 0x504C, 0, "5087 V3"},
    {3263, 0x3151, 0x504C, 0, "5075 V3"},
    {3405, 0x38EE, 0x0003, 0, "5108 V5"},
    {3444, 0x38EE, 0x0003, 0, "5087 V5"},
    {3448, 0x38EE, 0x0003, 0, "5075 V5"},
    // RY6602
    {3543, 0x38EE, 0x0005, 0, "5108 V5"},
    {3579, 0x38EE, 0x0005, 0, "5087 V5"},
    {3582, 0x38EE, 0x0005, 0, "5075 V5"},
    //{还没调试,0x38EE,0x0005,0,"Air01 UK"},
    // 鼠标案子PAN1080
    {1393, 0x3151, 0x402A, 1, "AG ONE"}, // 有线
    {1393, 0x3151, 0x402D, 1, "AG ONE"}, // 2.4G:
    {2033, 0x3151, 0x402A, 1, "G ONE"},  // 有线：
    {2033, 0x3151, 0x402D, 1, "G ONE"},   // 2.4G:
    {   4, 0x38EE, 0x000F, 1, "Pulse 01"},
    {   5, 0x38EE, 0x000F, 1, "3108 RF"},
    {   6, 0x38EE, 0x000F, 1, "AG ONE"},
    {   7, 0x38EE, 0x000F, 1, "灵动V9 Max"},
    {   8, 0x38EE, 0x000F, 1, "泰坦N9 Max"},
    {   9, 0x38EE, 0x000F, 1, "灵动V9 Ultra"},
    {  10, 0x38EE, 0x000F, 1, "泰坦N9 Ultra"},
    {  11, 0x38EE, 0x000F, 1, "3087"},

    {5151, 0x320F, 0x5151, 0, "3108RF"},// 2.4G
    {5152, 0x320F, 0x5152, 0, "3108RF"} // 有线
};

AkkoDeviceInfo *getDevice(quint32 id)
{
    int nCount = s_AkkoDeviceTable.count();
    for (int i = 0; i < nCount; i++)
    {
        if (s_AkkoDeviceTable[i].id == id)
            return &s_AkkoDeviceTable[i];
    }
    return nullptr;
}

void MainWindow::addDevice(quint32 id)
{
    AkkoDeviceInfo *dev = getDevice(id);
    if (dev)
    {
        ui->labelLarge->hide();

        // FrameDeviceShow *device = new FrameDeviceShow(this);
        FrameDeviceShow *device = FrameDeviceShow::getFrameShow(m_layout->count(),this);
        device->m_sa = ui->scrollArea;
        device->m_device = dev;
        device->setName(dev->name);
        device->show();
        m_layout->addWidget(device);

        if (dev->type == 0)
        {
            device->setFixedWidth(970);
            device->setImage(rand() % 2 ? "./images/3405.png" : "./images/kb2.png");
        }

        if (dev->type == 1)
        {
            device->setFixedWidth(280);
            device->setImage(rand() % 2 ? "./images/mouse1.png" : "./images/0002.png");
        }

        if(!device->m_bConacted)
        {
            device->m_bConacted = true;
            connect(device,&FrameDeviceShow::onClicked,this,[=](void *device){
                AkkoDeviceInfo *dev = static_cast<AkkoDeviceInfo *>(device);
                settings.setValue("AkkoDeviceIndex",dev->type == 0 ? 5 : 9);

                HWND hParentWnd = (HWND)ui->frameEmb->winId();
                ::SetParent(s_hWndEmb,hParentWnd);
                ::SetForegroundWindow(s_hWndEmb);
                ::SetWindowPos(s_hWndEmb, HWND_TOP, 0, 0, ui->frameEmb->width(), ui->frameEmb->height(), SWP_SHOWWINDOW | SWP_FRAMECHANGED);

                QTimer::singleShot(50,this,[=]{
                    ui->frameEmb->show();
                    ui->frameEmb->raise();
                    ui->frameEmb->update();

                    static QTimer *pTmChk = new QTimer(this);
                    pTmChk->stop();
                    pTmChk->start(100);
                    connect(pTmChk,&QTimer::timeout,this,[=]{
                        ::ShowWindow(s_hWndEmb,SW_SHOW);
                        ::SetWindowPos(s_hWndEmb, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
                    });
                });
            });
        }
    }
}

void MainWindow::enumDevice()
{
    while (m_layout->count())
    {
        FrameDeviceShow *item = (FrameDeviceShow *)m_layout->takeAt(0)->widget();
        if (!item) break;
        m_layout->removeWidget(item);
        item->hide();
    }

    QStringList VIDList;
    QList<quint16> VidList = {0x3151, 0x38EE, 0x25A7, 0x05AC, 0x0461};
    foreach (quint16 VID, VidList)
    {
        hid_device_info *pRoot = hid_enumerate(VID, 0);
        hid_device_info *pTemp = pRoot;
        while (pTemp)
        {
            // qDebug()<< pTemp->path << pTemp->usage << pTemp->usage_page;
            if (pTemp->usage_page == 0xFFFF && pTemp->usage == 2)
            {
                hid_device *pDev = hid_open_path(pTemp->path);
                if (!pDev) continue;

                VIDList.push_back(QString::asprintf("%04X", pTemp->product_id));

                QByteArray cmd(120, 0);
                cmd[1] = 0x8F;
                cmd[8] = 0xFF - 0x8F;

                hid_send_feature_report(pDev, (quint8 *)cmd.data(), 65);
                QThread::msleep(5);

                char buf[1024] = {0};
                int nlen = hid_get_feature_report(pDev, (quint8 *)buf, 65);
                if (nlen > 0)
                {
                    QByteArray data(buf + 1, nlen - 1);
                    quint32 id = *(quint32 *)(data.data() + 1);
                    qDebug() << "get_:" << data.left(16).toHex(' ').toUpper() << id << Qt::hex << pTemp->product_id;

                    addDevice(id);
                }

                hid_close(pDev);
            }
            pTemp = pTemp->next;
        }
        hid_free_enumeration(pRoot);
    }

    QString strCmd0("04 00 00 1a 06 00 00 00");
    QString strCmd1("04 00 00 30 06 00 00 00");
    QList<quint16>VShengs={0x38EE,0x320F};
    foreach (quint16 VID, VShengs)
    {
        hid_device_info *pRoot = hid_enumerate(VID, 0);
        hid_device_info *pTemp = pRoot;
        while (pTemp)
        {
            //qDebug()<< pTemp->path << pTemp->usage << pTemp->usage_page;
            if(pTemp->usage == 146 && pTemp->usage_page == 65308)
            {
                hid_device *pDev = hid_open_path(pTemp->path);
                if(!pDev) continue;

                VIDList.push_back(QString::asprintf("%04X", pTemp->product_id));

                quint8 szBuf[64]={0};
                QByteArray cmd=QByteArray::fromHex(strCmd0.toLatin1());
                hid_write(pDev,(quint8*)cmd.data(),cmd.size());
                QThread::msleep(5);
                int len = hid_read(pDev,szBuf,16);
                QByteArray Log;
                Log.append((char *)szBuf,len);
                qDebug() << "read:" << Log.left(16).toHex(' ').toUpper() << Qt::hex << pTemp->product_id;
                Log.clear();

                cmd=QByteArray::fromHex(strCmd1.toLatin1());
                hid_write(pDev,(quint8*)cmd.data(),cmd.size());
                QThread::msleep(5);
                hid_read(pDev,szBuf,16);
                Log.append((char *)szBuf,len);
                qDebug() << "read:" << Log.left(16).toHex(' ').toUpper() << Qt::hex << pTemp->product_id;

                if(pTemp->product_id == 0xf)
                    addDevice(9);
                if(pTemp->product_id == 0x5151)
                    addDevice(5151);
            }
            pTemp = pTemp->next;
        }
        hid_free_enumeration(pRoot);
    }

    QString strInfo = QString(tr("我的设备")) + QString("(%1)").arg(VIDList.count());
    ui->labelLarge1->setText(strInfo);
}

MainWindow::~MainWindow() { delete ui; }

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonRelease)
    {
        QScrollBar *sb = ui->scrollArea->horizontalScrollBar();
        if (ui->labelPrev == obj) {
            sb->setValue(sb->value() - ui->scrollArea->width());
        }

        if (ui->labelNext == obj) {
            sb->setValue(sb->value() + ui->scrollArea->width());
        }

        ui->scrollArea->update();
    }

    return QMainWindow::eventFilter(obj, e);
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
    if (ui->scrollArea->isHidden())
        p.drawImage(this->rect(), QImage("./images/MainPicture.png"));
    else
        p.fillRect(this->rect(), Qt::white);

    p.setPen(Qt::blue);
    p.drawRoundedRect(this->rect(), 12, 12);

    QMainWindow::paintEvent(event);
}

void MainWindow::on_pushButtonExit_clicked() { qApp->exit(); }

void MainWindow::mousePressEvent(QMouseEvent *event) {
    m_pLangMenu->hide();
    if (event->button() == Qt::LeftButton) {
        if (event->pos().y() < 200) {
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
            m_dragging = true;
            return;
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && m_dragging) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
    // QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
    // QMainWindow::mouseReleaseEvent(event);
}

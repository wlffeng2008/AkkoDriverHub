#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <windows.h>
#include <dbt.h>

#include <QCoreApplication>
#include <QAbstractNativeEventFilter>

#include <QMainWindow>
#include <QLayout>

#include "ModuleLangMenu.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class USBNotifier : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit USBNotifier(QObject *parent = nullptr) : QObject(parent) {
        //QCoreApplication::instance()->installNativeEventFilter(this) ;
    }

signals:
    void devicePluggined(bool in=true);

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        Q_UNUSED(eventType)
        Q_UNUSED(result)
        MSG* msg = reinterpret_cast<MSG*>(message);
        if (msg->message == WM_DEVICECHANGE)
        {
            qDebug() << "USBNotifier::nativeEventFilter: " << msg->wParam << msg->lParam;
            //if(msg->wParam == DBT_DEVICEARRIVAL       )  emit devicePluggined(true);
            //if(msg->wParam == DBT_DEVICEREMOVECOMPLETE)  emit devicePluggined(false);
            emit devicePluggined(true);
            return true ;
        }
        return false ;
    }
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

    void enumDevice();
    void addDevice(quint32 id);

private slots:
    void on_pushButtonExit_clicked();

private:
    Ui::MainWindow *ui;

    ModuleLangMenu *m_pLangMenu=nullptr;

    QLayout *m_layout = nullptr;

    QPoint m_dragPosition;
    bool m_dragging = false;
};
#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "bomquotefile.h"
#include <QMainWindow>
#include <QTreeWidgetItem>
QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void load_bomquote_file_into_gui();
    private slots:
    void on_actionopen_triggered();

    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

    void on_actionsave_observer_file_triggered();

    void on_actionopen_observer_file_triggered();

    void on_actionupdate_Observer_file_from_bomQuote_triggered();

    private:
    Ui::MainWindow *ui;
    BomQuoteFile bomquote_file;
    void setTreevewItemBGColor(QTreeWidgetItem *item, int color);
};
#endif // MAINWINDOW_H

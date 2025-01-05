#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QtSql>
#include <QModelIndex>
#include <QMenu>
#include <QtCharts>
#include "database_management.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void on_choix_db_clicked();
    void on_add_row_clicked();

    void on_creation_db_clicked();

private:
    Ui::MainWindow *ui;
    Database_management *db_manag;
    QSqlTableModel *model = nullptr;

    void afficher_donnees();
    void create_graph();
    void add_line();
    void show_context_menu(const QPoint &pos);
    void modify_row(int index);
    void delete_row(int index);
};
#endif // MAINWINDOW_H

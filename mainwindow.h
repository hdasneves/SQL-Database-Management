#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QtSql>
#include <QInputDialog>
#include <QDateEdit>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QModelIndex>
#include <QMenu>
#include <QtCharts>

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

    ~MainWindow();

private slots:
    void on_choix_db_clicked();
    void on_add_row_clicked();

    void on_creation_db_clicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlTableModel *model = nullptr;

    void afficher_donnees();
    void create_graph();
    void add_line();
    void show_context_menu(const QPoint &pos);
    void modify_row(int index);
    void delete_row(int index);
    QMap<QString, QVariant> dialog_box(const QMap<QString, QVariant> &initialValues);
};
#endif // MAINWINDOW_H

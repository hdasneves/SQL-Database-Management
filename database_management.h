#ifndef DATABASE_MANAGEMENT_H
#define DATABASE_MANAGEMENT_H

#include <QObject>
#include <QtSql>

class Database_management : public QObject
{
    Q_OBJECT
public:
    explicit Database_management(QObject *parent = nullptr);
    bool open_database(QString filename);
    bool create_tables();
    bool add_row(QString ticker, QString typeact, double amount);
    bool update_table(QString ticker, QString typeact, double amount, int id);
    bool delete_row(int index);
    QMap<QString, double> get_graph_data();

    QSqlDatabase get_db() const;

private:
    QSqlDatabase db;

signals:
};

#endif // DATABASE_MANAGEMENT_H

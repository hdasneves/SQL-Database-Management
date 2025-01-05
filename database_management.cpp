#include "database_management.h"

Database_management::Database_management(QObject *parent)
    : QObject{parent}
{
    qInfo() << this << "constructed !";
}

bool Database_management::open_database(QString filename)
{

    qInfo() << "Base de données importée avec succés !";

    if (db.isOpen()) {
        db.close();
        QSqlDatabase::removeDatabase(db.connectionName());
        qInfo() << "Connexion existante fermée.";
    }

    // Vérification et recréation de la connexion uniquement si nécessaire
    if (!QSqlDatabase::contains("main_connection")) {
        db = QSqlDatabase::addDatabase("QSQLITE", "main_connection");  // Connexion persistante
    } else {
        db = QSqlDatabase::database("main_connection");  // Récupération de la connexion existante
    }

    db.setDatabaseName(filename);

    qInfo() << "Base de données importée avec succés !";

    if (!db.open()){
        return false;
    }
    return true;
}

bool Database_management::create_tables()
{
    qInfo() << "Création des tables";
    QSqlQuery creation_tables(db);
    QString create_table = "CREATE TABLE IF NOT EXISTS portfolio("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "ticker VARCHAR(10) NOT NULL,"
                           "typeact VARCHAR(15) NOT NULL,"
                           "price DOUBLE,"
                           "quantity INTEGER)";

    if (!creation_tables.exec(create_table)){ //si erreur lors du query
        return false;
    }
    return true;
}

bool Database_management::add_row(QString ticker, QString typeact, double amount)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO portfolio (ticker, typeact, amount)"
                "VALUES(:ticker, :typeact, :amount)");

    query.bindValue(":ticker", ticker);
    query.bindValue(":typeact", typeact);
    query.bindValue(":amount", amount);

    if (!query.exec()){
        return false;
    }
    return true;
}

bool Database_management::update_table(QString ticker, QString typeact, double amount, int id)
{

    QSqlQuery query(db);
    query.prepare("UPDATE portfolio SET ticker = :ticker, typeact = :typeact, amount  = :amount WHERE id = :id");
    query.bindValue(":ticker", ticker);
    query.bindValue(":typeact", typeact);
    query.bindValue(":amount", amount);
    query.bindValue(":id", id);

    if (!query.exec()){
        return false;
    }
    return true;
}

bool Database_management::delete_row(int index)
{
    QSqlQuery suppr(db);
    suppr.prepare("DELETE FROM portfolio WHERE id = :id");
    suppr.bindValue(":id", index);


    if (!suppr.exec()){
        return false;
    }
    return true;
}

QMap<QString, double> Database_management::get_graph_data()
{
    QSqlQuery query(db);
    query.prepare("SELECT typeact, COUNT(*) AS nombre FROM portfolio GROUP BY typeact");

    QMap<QString, double> dict;

    if (query.exec()) {
        while (query.next()) {
            QString typeact = query.value(0).toString();
            int count = query.value(1).toInt();
            dict.insert(typeact, count);
        }
        return dict;
    }
    qWarning() << "Erreur lors de la requête : " << query.lastError().text();
    return dict;
}

QSqlDatabase Database_management::get_db() const
{
    return db;
}

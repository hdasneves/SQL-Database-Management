#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_choix_db_clicked() //fonction qui prend en compte la database choisie, vérifie si elle est ouvrable puis actualise le label qui informe l'utilisateur du chemin d'accès chosit
{
    QString filename = QFileDialog::getOpenFileName(this, QObject::tr("Importer une base SQL"), QDir::homePath(),QObject::tr("Fichiers SQL (*.db *.sqlite);"));

    if (filename.isEmpty()){ //Vérifie que l'utilisateur a bien choisi un fichier
        return;
    }

    ui->affich_db->setText("Base de donnée sélectionnée : " + filename);

    if (model) {
        delete model;
        model = nullptr;
    }

    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) { //supprime la connexion s'il y en a déjà une
        {
            QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection);
            db.close();
        }
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"); //Ajout de la base de données (type de fichier : SQLite, localisation : filename)
    db.setDatabaseName(filename);

    if (!db.open()){ //Vérifie si le fichier éxiste bien et est ouvrable
        QMessageBox::critical(this, QObject::tr("Erreur dans l'ouverture de la base de données"), "Impossible d'ouvrir la base de données !");
        return;
    }
    else{
        QMessageBox::information(this, QObject::tr("Base ouverte"), "Vôtre base de données a été chargée avec succès !");
    }

    afficher_donnees();
    create_graph();
}

void MainWindow::afficher_donnees()
{
    if (model){
        delete model; //permet de supprimer le modèle éxistant s'il existe pour actualiser le TableView
    }


    QSqlQuery table_creation(db); //créer une variable permettant de recueillir

    QString create_table = "CREATE TABLE IF NOT EXISTS transactions("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "date VARCHAR(20),"
                          "ticker VARCHAR(10) NOT NULL,"
                          "typeact VARCHAR(15) NOT NULL,"
                          "price DOUBLE,"
                          "quantity INTEGER)";

    if (!table_creation.exec(create_table)){ //si erreur lors du query
        QMessageBox::critical(this, "Erreur", "Impossible de créer la table : " + table_creation.lastError().text());
        return;
    }

    QList<QString> col({"id", "date", "ticker", "typeact", "price", "quantity"});

    model = new QSqlTableModel(this, db); //association d'un modèle à une database sql
    model->setTable("transactions");
    model->select();

    for (int i = 0; i < col.length(); i++){ //ajoute les en-têtes de colonne
        model->setHeaderData(i, Qt::Horizontal, col[i]);
    }

    ui->transactions->setModel(model); //ajoute le modèle SQL au TableView
    ui->transactions->setEditTriggers(QAbstractItemView::NoEditTriggers); //fais en sorte que les données ne soient pas modifiables manuellement
    ui->transactions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->transactions->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->transactions, &QTableView::customContextMenuRequested, this, &MainWindow::show_context_menu);
}

void MainWindow::create_graph()
{
    if (model->rowCount() == 0) {
        QChart *emptyChart = new QChart();
        ui->graph_sectoriel->setChart(emptyChart);
    }



    QSqlQuery query(db);
    query.prepare("SELECT typeact, COUNT(*) AS nombre FROM transactions GROUP BY typeact");
    QMap<QString, int> dict;

    if (query.exec()) {
        while (query.next()) {
            QString typeact = query.value(0).toString();
            int count = query.value(1).toInt();
            dict.insert(typeact, count);
    }
    } else {
        qWarning() << "Erreur lors de la requête : " << query.lastError().text();
        return;
    }

    QPieSeries *series = new QPieSeries(this);

    for (auto [key, value] : dict.asKeyValueRange()) {
        series->append((QString)key, (int)value);
    }

    series->setLabelsVisible(true);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition sectorielle des différents actifs");
    chart->legend()->hide();

    ui->graph_sectoriel->setChart(chart);
}

void MainWindow::show_context_menu(const QPoint &pos) //fonction qui prend la position du curseur de l'individu lorsqu'il clique droit et permet de lancer les fonctions pour modifier ou supprimer
{
    QModelIndex index = ui->transactions->indexAt(pos);

    QMenu modif_ou_suppr(this); //Création du menu et des boutons

    QAction *modifier = new QAction("Modifier", this);
    QAction *supprimer = new QAction("Supprimer", this);

    connect(modifier, &QAction::triggered, this, [this, index, &modifier]() {modify_row(index.row());}); //associe le bouton modifier à la fonction modify_row

    connect(supprimer, &QAction::triggered, this, [this, index, &supprimer]() {delete_row(index.row());}); //associe le bouton supprimer à la fonction delete_row

    modif_ou_suppr.addAction(modifier); //ajoute les boutons modifier et supprimer au menu
    modif_ou_suppr.addAction(supprimer);

    modif_ou_suppr.exec(ui->transactions->viewport()->mapToGlobal(pos));

}

void MainWindow::modify_row(int index)
{
    QSqlRecord record = model->record(index);
    QList<QVariant> val({
        record.value("date"),
        record.value("ticker"),
        record.value("typeact"),
        record.value("price"),
        record.value("quantity"),
    });


    QMap<QString, QVariant> initial_values;


    QList<QString> col({"date", "ticker", "typeact", "price", "quantity"});

    for (int i = 0; i < 5; i++){
        initial_values.insert(col[i], val[i]);
    }


    QMap<QString, QVariant> values =  dialog_box(initial_values);

    if (values.isEmpty()){
        return;
    }

    QSqlQuery mod(db);
    mod.prepare("UPDATE transactions SET date = :date, ticker = :ticker, typeact = :typeact, price = :price, quantity = :quantity WHERE id = :id");
    mod.bindValue(":date", values["date"].toDate());
    mod.bindValue(":ticker", values["ticker"].toString());

    mod.bindValue(":typeact", values["typeact"].toString());
    mod.bindValue(":price", values["price"].toDouble());
    mod.bindValue(":quantity", values["quantity"].toInt());
    mod.bindValue(":id", record.value("id"));

    if (!mod.exec()){
        QMessageBox::critical(this, "Erreur", "La modification a échoué : " + mod.lastError().text());
    }
    model->select();
    create_graph();
}



void MainWindow::delete_row(int index)
{
    QSqlRecord record = model->record(index);
    QVariant ind = record.value("id");

    QSqlQuery suppr(db);
    suppr.prepare("DELETE FROM transactions WHERE id = :id");
    suppr.bindValue(":id", ind);


    if (!suppr.exec()){
        QMessageBox::critical(this, "Erreur", "La suppression a échoué : " + suppr.lastError().text());
    }
    model->select();
    create_graph();
}

QMap<QString, QVariant> MainWindow::dialog_box(const QMap<QString, QVariant> &initial_values) //automatise la sortie de la dialogbox
{
    QDialog dialog(this); //création d'une boîte de dialogue
    dialog.setWindowTitle("Ajouter une transaction");

    QDateEdit *date_tr = new QDateEdit(QDate::currentDate(), &dialog); //Création des widgets qui seront mis dans l'interface (&dialog) permet de supprimer les pointers après la fermeture de la boîte de dialogues
    QLineEdit *ticker = new QLineEdit(&dialog);
    QComboBox *type_dactif = new QComboBox(&dialog);
    QDoubleSpinBox *prix = new QDoubleSpinBox(&dialog);
    QSpinBox *quantite = new QSpinBox(&dialog);

    date_tr->setMaximumDate(QDate::currentDate());
    type_dactif->addItems({"Actions", "Obligations", "ETF", "Cash"});
    prix->setRange(0.0, 1000.0);
    prix->setDecimals(2);
    quantite->setMinimum(0);

    if (!initial_values.isEmpty()) { //dans le cas où on modifie une ligne, on rajoute les valeurs présentes dans celle-ci
        date_tr->setDate(initial_values["date"].toDate());
        ticker->setText(initial_values["ticker"].toString());
        type_dactif->setCurrentText(initial_values["typeact"].toString());
        prix->setValue(initial_values["price"].toDouble());
        quantite->setValue(initial_values["quantity"].toInt());
    }


    QFormLayout *dispo = new QFormLayout(&dialog); //utilisation d'un QFormLayout pour agencer les widgets
    dispo->addRow("Date :", date_tr);
    dispo->addRow("Ticker :", ticker);
    dispo->addRow("Type d'actif", type_dactif);
    dispo->addRow("Prix :", prix);
    dispo->addRow("Quantité :", quantite);

    QWidget *formWidget = new QWidget(&dialog); //insertion de la disposition dans un formwidget qui nous permettra d'avoir les widgets
    formWidget->setLayout(dispo);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog); //insertion des boutons annuler et confirmer
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog); //combinaison du formwidget et des boutons pour l'interface finale
    mainLayout->addWidget(formWidget);
    mainLayout->addWidget(buttonBox);

    dialog.setLayout(mainLayout); //ajout du tout à la boîte de dialogue

    QMap<QString, QVariant> result;


    connect(buttonBox, &QDialogButtonBox::accepted, [&]() {
        if (ticker->text().isEmpty() || prix->value() == 0 || quantite->value() == 0) {
            QMessageBox::warning(&dialog, "Champ requis", "Les champs 'Ticker', 'Prix' et 'quantité' ne peut pas être vide ou égal à 0");
        } else {
            dialog.accept();
        }
    });

    if (dialog.exec() == QDialog::Accepted) { //retourne un dictionnaire avec les valeurs afin de faire la requête SQL
        result["date"] = date_tr->date();
        result["ticker"] = ticker->text();
        result["typeact"] = type_dactif->currentText();
        result["price"] = prix->value();
        result["quantity"] = quantite->value();
    }
    return result;
}

void MainWindow::on_add_row_clicked()
{


    const QMap<QString, QVariant> initial_value;
    QMap<QString, QVariant> values = dialog_box(initial_value);

    if (values.isEmpty()){
        return;
    }

    QSqlQuery add(db);

    add.prepare("INSERT INTO transactions (date, ticker, typeact, price, quantity)"
                "VALUES(:date, :ticker, :typeact, :price, :quantity)");

    add.bindValue(":date", values["date"].toString());
    add.bindValue(":ticker", values["ticker"].toString());
    add.bindValue(":typeact", values["typeact"].toString());
    add.bindValue(":price", values["price"].toDouble());
    add.bindValue(":quantity", values["quantity"].toInt());

    if (!add.exec()){
        QMessageBox::critical(this, "Problème lors de l'ajout", "Erreur lors de l'ajout : database probablement non ouverte !");
        return;
    }
    model->select();
    create_graph();
}

void MainWindow::on_creation_db_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Create File"), QDir::homePath(), tr("SQL (*.db *.sqlite)"));

    if (filename.isEmpty()){
        return;
    }
    QFile file(filename);

    if (file.exists()){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Fichier existant"), tr("Le fichier existe déjà. Voulez-vous le remplacer ?"), QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No){
            return;
        }
    }

    if (file.open(QIODevice::WriteOnly)){
        file.close();
    }
    else{
        QMessageBox::warning(this, "Erreur", "Erreur lors de la création du fichier.");
    }
}


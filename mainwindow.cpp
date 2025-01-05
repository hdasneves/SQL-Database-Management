#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database_management.h"
#include "dialog_box.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    db_manag = new Database_management(this);
}


void MainWindow::on_choix_db_clicked() //fonction qui prend en compte la database choisie, vérifie si elle est ouvrable puis actualise le label qui informe l'utilisateur du chemin d'accès chosit
{
    QString filename = QFileDialog::getOpenFileName(this, QObject::tr("Importer une base SQL"), QDir::homePath(),QObject::tr("Fichiers SQL (*.db *.sqlite);"));

    if (filename.isEmpty()){ //Vérifie que l'utilisateur a bien choisi un fichier
        return;
    }

    if (model) { //remplace le modèle prééxistant s'il y en a déjà un
        delete model;
        model = nullptr;
    }

    if (!db_manag->open_database(filename)){
        QMessageBox::critical(this, QObject::tr("Erreur dans l'ouverture de la base de données"), "Impossible d'ouvrir la base de données !" + db_manag->get_db().lastError().text());
        return;
    }
    QMessageBox::information(this, QObject::tr("Base de données ouverte !"), "Vôtre base de donnée a été importée !");

    ui->affich_db->setText("Base de donnée sélectionnée : " + filename);
    afficher_donnees();
    create_graph();
}

void MainWindow::afficher_donnees()
{
    if (model){
        delete model; //permet de supprimer le modèle éxistant s'il existe pour actualiser le TableView
    }



    if (!db_manag->create_tables()){
        QMessageBox::critical(this, "Erreur", "Impossible de créer la table : " + db_manag->get_db().lastError().text());
        return;
    }
    model = new QSqlTableModel(this, db_manag->get_db()); //association d'un modèle à une database sql
    model->setTable("portfolio");
    model->select();

    QList<QString> col({"id", "ticker", "typeact", "amount"});

    for (int i = 0; i < col.length(); i++){ //ajoute les en-têtes de colonne
        model->setHeaderData(i, Qt::Horizontal, col[i]);
    }

    ui->portf->setModel(model); //ajoute le modèle SQL au TableView
    ui->portf->setEditTriggers(QAbstractItemView::NoEditTriggers); //fais en sorte que les données ne soient pas modifiables manuellement
    ui->portf->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->portf->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->portf, &QTableView::customContextMenuRequested, this, &MainWindow::show_context_menu);
}

void MainWindow::create_graph()
{
    if (model->rowCount() == 0) {
        QChart *emptyChart = new QChart();
        ui->graph_type->setChart(emptyChart);
        return;
    }

    QMap<QString, double> dict = db_manag->get_graph_data();

    QPieSeries *series = new QPieSeries(this);

    for (auto [key, value] : dict.asKeyValueRange()) {
        series->append((QString)key, (int)value);
    }

    series->setLabelsVisible(true);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition sectorielle des différents actifs");
    chart->legend()->hide();

    ui->graph_type->setChart(chart);
}

void MainWindow::show_context_menu(const QPoint &pos) //fonction qui prend la position du curseur de l'individu lorsqu'il clique droit et permet de lancer les fonctions pour modifier ou supprimer
{
    QModelIndex index = ui->portf->indexAt(pos);

    QMenu modif_ou_suppr(this); //Création du menu et des boutons

    QAction *modifier = new QAction("Modifier", this);
    QAction *supprimer = new QAction("Supprimer", this);

    connect(modifier, &QAction::triggered, this, [this, index, &modifier]() {modify_row(index.row());}); //associe le bouton modifier à la fonction modify_row

    connect(supprimer, &QAction::triggered, this, [this, index, &supprimer]() {delete_row(index.row());}); //associe le bouton supprimer à la fonction delete_row

    modif_ou_suppr.addAction(modifier); //ajoute les boutons modifier et supprimer au menu
    modif_ou_suppr.addAction(supprimer);

    modif_ou_suppr.exec(ui->portf->viewport()->mapToGlobal(pos));

}

void MainWindow::modify_row(int index)
{
    QSqlRecord record = model->record(index);
    QList<QVariant> val({
        record.value("ticker"),
        record.value("typeact"),
        record.value("amount"),
        record.value("id")
    });

    if (val[3] == 0){
        QMessageBox::critical(this, "Erreur dans la modification", "Veuillez choisir une ligne valable");
        return;
    }

    qInfo() << val;

    QMap<QString, QVariant> initial_values;
    QList<QString> col({"ticker", "typeact", "amount", "id"});

    for (int i = 0; i < 4; i++){
        initial_values.insert(col[i], val[i]);
    }

    Dialog_box dialog(this, initial_values);

    if (dialog.exec() == QDialog::Accepted) {
        QMap<QString, QVariant> values = dialog.get_result();
        if (values.isEmpty()) {
            return;
        }
        if (!db_manag->update_table(values["ticker"].toString(), values["typeact"].toString(), values["amount"].toDouble(), val[3].toInt())){
            QMessageBox::critical(this, "Erreur dans la modification", "Erreur dans la modification");
        }

        model->select();
        create_graph();
    }

}



void MainWindow::delete_row(int index)
{
    QSqlRecord record = model->record(index);
    QVariant ind = record.value("id");

    if (!db_manag->delete_row(ind.toInt())){
        QMessageBox::critical(this, "Impossible de supprimer la ligne", "Impossible de supprimer la ligne");
    }



    model->select();
    create_graph();
}


void MainWindow::on_add_row_clicked()
{

    const QMap<QString, QVariant> initial_value;
    Dialog_box dialog(this , initial_value);


    if (dialog.exec() == QDialog::Accepted) {

        QMap<QString, QVariant> values = dialog.get_result();

        if (values.isEmpty()){
            return;
        }

        if(!db_manag->add_row(values["ticker"].toString(), values["typeact"].toString(), values["amount"].toDouble())){
            QMessageBox::critical(this, "Problème lors de l'ajout", "Erreur lors de l'ajout : database probablement non ouverte !");
        }

        model->select();
        create_graph();
    }
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


#include "dialog_box.h"


Dialog_box::Dialog_box(QWidget *parent, const QMap<QString, QVariant> &initial_values)
    : QDialog{parent}
{
    ticker = new QLineEdit(this);  // Initialisation des widgets
    type_dactif = new QComboBox(this);
    amount = new QSpinBox(this);

    setup_ui(initial_values);
}


void Dialog_box::setup_ui(const QMap<QString, QVariant> &initial_values)
{
    setWindowTitle("Ajouter une transaction");

    type_dactif->addItems({"Actions", "Obligations", "ETF", "Cash"});

    if (!initial_values.isEmpty()) {
        ticker->setText(initial_values["ticker"].toString());
        type_dactif->setCurrentText(initial_values["typeact"].toString());
        amount->setValue(initial_values["amount"].toInt());
    }


    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(("Ticker :"), ticker);
    formLayout->addRow(("Type d'actif"), type_dactif);
    formLayout->addRow(("Montant :"), amount);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &Dialog_box::accept_dialog);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &Dialog_box::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void Dialog_box::accept_dialog()
{
    if (ticker->text().isEmpty() || amount->value() == 0) {
        QMessageBox::warning(this, "Champ requis", "Les champs 'Ticker' et 'Montant' ne peuvent pas être vides ou égaux à 0.");
        return;
    }

    result["ticker"] = ticker->text();
    result["typeact"] = type_dactif->currentText();
    result["amount"] = amount->value();

    accept();
}

QMap<QString, QVariant> Dialog_box::get_result() const
{
    return result;
}

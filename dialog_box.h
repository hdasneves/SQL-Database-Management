#ifndef DIALOG_BOX_H
#define DIALOG_BOX_H

#include <QObject>
#include <QDialog>
#include <QDateEdit>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <qcombobox.h>


class Dialog_box : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog_box(QWidget *parent = nullptr, const QMap<QString, QVariant> &initial_values = {});

    QMap<QString, QVariant> get_result() const;

private :
    void setup_ui(const QMap<QString, QVariant> &initial_values);
    void accept_dialog();
    QLineEdit *ticker;
    QComboBox *type_dactif;
    QSpinBox *amount;
    QMap<QString, QVariant> result;

signals:
};

#endif // DIALOG_BOX_H

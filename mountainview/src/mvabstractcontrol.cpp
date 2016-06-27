/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "mvabstractcontrol.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSet>

class MVAbstractControlPrivate {
public:
    MVAbstractControl* q;
    MVContext* m_context;
    MVMainWindow* m_main_window;

    QMap<QString, QLineEdit*> m_int_controls;
    QMap<QString, QLineEdit*> m_double_controls;
    QMap<QString, QComboBox*> m_choices_controls;
    QMap<QString, QCheckBox*> m_checkbox_controls;

    QMap<QString, QWidget*> all_control_widgets();
};

MVAbstractControl::MVAbstractControl(MVContext* context, MVMainWindow* mw)
{
    d = new MVAbstractControlPrivate;
    d->q = this;
    d->m_context = context;
    d->m_main_window = mw;
}

MVAbstractControl::~MVAbstractControl()
{
    delete d;
}

MVContext* MVAbstractControl::mvContext()
{
    return d->m_context;
}

MVMainWindow* MVAbstractControl::mainWindow()
{
    return d->m_main_window;
}

QVariant MVAbstractControl::controlValue(QString name) const
{
    if (d->m_int_controls.contains(name)) {
        return d->m_int_controls[name]->text().toInt();
    }
    if (d->m_double_controls.contains(name)) {
        return d->m_double_controls[name]->text().toDouble();
    }
    if (d->m_choices_controls.contains(name)) {
        return d->m_choices_controls[name]->currentText();
    }
    if (d->m_checkbox_controls.contains(name)) {
        return d->m_checkbox_controls[name]->isChecked();
    }
    return QVariant();
}

void MVAbstractControl::setControlValue(QString name, QVariant val)
{
    if (controlValue(name) == val)
        return;
    if (d->m_int_controls.contains(name)) {
        QString txt = QString("%1").arg(val.toInt());
        d->m_int_controls[name]->setText(txt);
    }
    if (d->m_double_controls.contains(name)) {
        QString txt = QString("%1").arg(val.toDouble());
        d->m_double_controls[name]->setText(txt);
    }
    if (d->m_choices_controls.contains(name)) {
        QString txt = QString("%1").arg(val.toString());
        d->m_choices_controls[name]->setCurrentText(txt);
    }
    if (d->m_checkbox_controls.contains(name)) {
        d->m_checkbox_controls[name]->setChecked(val.toBool());
    }
}

QWidget* MVAbstractControl::createIntControl(QString name)
{
    QLineEdit* X = new QLineEdit;
    X->setText("0");
    d->m_int_controls[name] = X;
    QObject::connect(X, SIGNAL(textEdited(QString)), this, SLOT(updateContext()));
    return X;
}

QWidget* MVAbstractControl::createDoubleControl(QString name)
{
    QLineEdit* X = new QLineEdit;
    X->setText("0");
    d->m_double_controls[name] = X;
    QObject::connect(X, SIGNAL(textEdited(QString)), this, SLOT(updateContext()));
    return X;
}

QComboBox* MVAbstractControl::createChoicesControl(QString name)
{
    QComboBox* X = new QComboBox;
    d->m_choices_controls[name] = X;
    QObject::connect(X, SIGNAL(currentTextChanged(QString)), this, SLOT(updateContext()), Qt::QueuedConnection);
    return X;
}

QCheckBox* MVAbstractControl::createCheckBoxControl(QString name)
{
    QCheckBox* X = new QCheckBox;
    d->m_checkbox_controls[name] = X;
    QObject::connect(X, SIGNAL(clicked(bool)), this, SLOT(updateContext()));
    return X;
}

void MVAbstractControl::setChoices(QString name, const QStringList& choices)
{
    if (!d->m_choices_controls.contains(name))
        return;
    QComboBox* X = d->m_choices_controls[name];
    QStringList strings;
    for (int i = 0; i < X->count(); i++) {
        strings << X->itemText(i);
    }
    if (strings.toSet() == choices.toSet())
        return;
    QString val = X->currentText();
    X->clear();
    for (int i = 0; i < choices.count(); i++) {
        X->addItem(choices[i]);
    }
    if ((!val.isEmpty()) && (choices.contains(val))) {
        X->setCurrentText(val);
    }
}

void MVAbstractControl::setControlEnabled(QString name, bool val)
{
    QMap<QString, QWidget*> all = d->all_control_widgets();
    if (all.contains(name)) {
        all[name]->setEnabled(val);
    }
}

void add_to(QMap<QString, QWidget*>& A, QMap<QString, QLineEdit*>& B)
{
    QStringList keys = B.keys();
    foreach (QString key, keys) {
        A[key] = B[key];
    }
}

void add_to(QMap<QString, QWidget*>& A, QMap<QString, QComboBox*>& B)
{
    QStringList keys = B.keys();
    foreach (QString key, keys) {
        A[key] = B[key];
    }
}

void add_to(QMap<QString, QWidget*>& A, QMap<QString, QCheckBox*>& B)
{
    QStringList keys = B.keys();
    foreach (QString key, keys) {
        A[key] = B[key];
    }
}

QMap<QString, QWidget*> MVAbstractControlPrivate::all_control_widgets()
{
    QMap<QString, QWidget*> ret;
    add_to(ret, m_int_controls);
    add_to(ret, m_double_controls);
    add_to(ret, m_choices_controls);
    add_to(ret, m_checkbox_controls);
    return ret;
}
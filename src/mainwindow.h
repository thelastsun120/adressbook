#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
class QTableWidget;
QT_END_NAMESPACE

struct Person {
    QString name;
    QString gender;
    QString phone;
    QString qq;
    QString category;

    QString serialize() const;
    static bool deserialize(const QString &line, Person &out);
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void addContact();
    void modifyContact();
    void deleteContact();
    void showAllContacts();
    void showByCategory();
    void findByName();
    void findByPhone();
    void saveContacts();

private:
    QVector<Person> contacts;
    QString dataFile;

    QTableWidget *table;
    QLineEdit *nameEdit;
    QComboBox *genderBox;
    QLineEdit *phoneEdit;
    QLineEdit *qqEdit;
    QLineEdit *categoryEdit;
    QLineEdit *searchEdit;

    void setupUi();
    void loadContacts();
    void refreshTable(const QVector<Person> &data);
    Person formPerson() const;
    int findIndexByName(const QString &name) const;
    void clearForm();
};

#endif

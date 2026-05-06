#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
class QTableWidget;
class QLabel;
QT_END_NAMESPACE

struct Person {
    QString name;
    QString gender;
    QString phone;
    QString qq;
    QString category;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

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
    QString dataFile;
    QString dbHost;
    int dbPort;
    QString dbName;
    QString dbUser;
    QString dbPassword;

    QTableWidget *table;
    QLineEdit *nameEdit;
    QComboBox *genderBox;
    QLineEdit *phoneEdit;
    QLineEdit *qqEdit;
    QLineEdit *categoryEdit;
    QLineEdit *searchEdit;
    QLabel *statusLabel;

    void setupUi();
    void loadDbConfig();
    bool initDatabase();
    void loadFromTextIfDbEmpty();
    void refreshTable(const QList<Person> &data);
    QList<Person> queryContacts(const QString &whereClause = QString(), const QVariantList &args = {}) const;
    int selectedContactId() const;
    Person formPerson() const;
    void clearForm();
};

#endif

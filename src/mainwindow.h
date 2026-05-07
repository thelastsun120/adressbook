#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <exception>
#include <iosfwd>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
class QTableWidget;
class QLabel;
QT_END_NAMESPACE

class DbException : public std::exception {
public:
    explicit DbException(QString message);
    const char *what() const noexcept override;

private:
    std::string msg;
};

class ContactRecord {
public:
    virtual ~ContactRecord() = default;
    virtual QString displayTag() const = 0;
};

struct Person : public ContactRecord {
    QString name;
    QString gender;
    QString phone;
    QString qq;
    QString category;

    QString displayTag() const override;

    friend std::ostream &operator<<(std::ostream &out, const Person &person);
    friend std::istream &operator>>(std::istream &in, Person &person);
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

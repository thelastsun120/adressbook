#include "mainwindow.h"

#include <QComboBox>
#include <QFile>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), dataFile("contacts.txt") {
    setupUi();
    if (!initDatabase()) {
        QMessageBox::critical(this, "错误", "MySQL 数据库连接失败，请检查主机/端口/用户名/密码。程序将退出。");
        close();
        return;
    }
    loadFromTextIfDbEmpty();
    showAllContacts();
}

MainWindow::~MainWindow() {
    QSqlDatabase::database().close();
}

void MainWindow::setupUi() {
    setWindowTitle("通讯录管理系统（Qt + MySQL）");
    resize(980, 640);

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    auto *title = new QLabel("📘 通讯录管理系统", this);
    title->setStyleSheet("font-size: 28px; font-weight: 700; color: #1f3a5f;");
    mainLayout->addWidget(title);

    auto *formBox = new QGroupBox("联系人信息", this);
    formBox->setStyleSheet("QGroupBox{font-weight:600;border:1px solid #b8c4d2;border-radius:8px;margin-top:8px;} QGroupBox::title{subcontrol-origin: margin;left:10px;padding:0 6px;}");
    auto *formLayout = new QFormLayout(formBox);
    nameEdit = new QLineEdit(this);
    genderBox = new QComboBox(this);
    genderBox->addItems({"男", "女", "其他"});
    phoneEdit = new QLineEdit(this);
    qqEdit = new QLineEdit(this);
    categoryEdit = new QLineEdit(this);
    formLayout->addRow("姓名", nameEdit);
    formLayout->addRow("性别", genderBox);
    formLayout->addRow("电话", phoneEdit);
    formLayout->addRow("QQ", qqEdit);
    formLayout->addRow("类别", categoryEdit);

    auto *searchBox = new QGroupBox("查询 / 过滤", this);
    searchBox->setStyleSheet(formBox->styleSheet());
    auto *searchLayout = new QHBoxLayout(searchBox);
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("输入姓名 / 电话 / 类别");
    auto *findNameBtn = new QPushButton("按姓名查找", this);
    auto *findPhoneBtn = new QPushButton("按电话查找", this);
    auto *showCategoryBtn = new QPushButton("分类显示", this);
    searchLayout->addWidget(searchEdit, 1);
    searchLayout->addWidget(findNameBtn);
    searchLayout->addWidget(findPhoneBtn);
    searchLayout->addWidget(showCategoryBtn);

    auto *buttonLayout = new QHBoxLayout();
    auto *addBtn = new QPushButton("添加", this);
    auto *modifyBtn = new QPushButton("修改", this);
    auto *deleteBtn = new QPushButton("删除", this);
    auto *showAllBtn = new QPushButton("显示全部", this);
    auto *saveBtn = new QPushButton("同步到文本", this);
    for (auto *btn : {addBtn, modifyBtn, deleteBtn, showAllBtn, saveBtn, findNameBtn, findPhoneBtn, showCategoryBtn}) {
        btn->setStyleSheet("QPushButton{background:#2f80ed;color:white;border:none;border-radius:6px;padding:8px 14px;} QPushButton:hover{background:#2368be;}");
    }
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(modifyBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(showAllBtn);
    buttonLayout->addWidget(saveBtn);

    auto *topLayout = new QHBoxLayout();
    topLayout->addWidget(formBox, 2);
    topLayout->addWidget(searchBox, 3);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(buttonLayout);

    table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"姓名", "性别", "电话", "QQ", "类别"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setAlternatingRowColors(true);
    table->setStyleSheet("QTableWidget{background:#f8fbff;alternate-background-color:#edf4ff;gridline-color:#d3dff0;border:1px solid #c5d3e6;border-radius:6px;} QHeaderView::section{background:#2f80ed;color:white;font-weight:600;padding:6px;border:none;}");
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(table, 1);

    setCentralWidget(central);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addContact);
    connect(modifyBtn, &QPushButton::clicked, this, &MainWindow::modifyContact);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteContact);
    connect(showAllBtn, &QPushButton::clicked, this, &MainWindow::showAllContacts);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveContacts);
    connect(findNameBtn, &QPushButton::clicked, this, &MainWindow::findByName);
    connect(findPhoneBtn, &QPushButton::clicked, this, &MainWindow::findByPhone);
    connect(showCategoryBtn, &QPushButton::clicked, this, &MainWindow::showByCategory);
}

bool MainWindow::initDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("addressbook_db");
    db.setUserName("root");
    db.setPassword("123456");
    if (!db.open()) return false;

    QSqlQuery query;
    return query.exec("CREATE TABLE IF NOT EXISTS contacts ("
                      "id INT PRIMARY KEY AUTO_INCREMENT,"
                      "name VARCHAR(50) NOT NULL,"
                      "gender VARCHAR(10) NOT NULL,"
                      "phone VARCHAR(20) NOT NULL UNIQUE,"
                      "qq VARCHAR(20),"
                      "category VARCHAR(50)) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
}

void MainWindow::loadFromTextIfDbEmpty() {
    QSqlQuery countQ("SELECT COUNT(*) FROM contacts");
    if (!countQ.next() || countQ.value(0).toInt() > 0) return;

    QFile file(dataFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    QSqlQuery insert;
    insert.prepare("INSERT INTO contacts(name, gender, phone, qq, category) VALUES(?,?,?,?,?)");
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        const QStringList parts = line.split('|');
        if (parts.size() != 5) continue;
        for (const auto &v : parts) insert.addBindValue(v);
        insert.exec();
    }
}

QList<Person> MainWindow::queryContacts(const QString &whereClause, const QVariantList &args) const {
    QList<Person> data;
    QString sql = "SELECT name, gender, phone, qq, category FROM contacts";
    if (!whereClause.isEmpty()) sql += " WHERE " + whereClause;
    sql += " ORDER BY id ASC";

    QSqlQuery query;
    query.prepare(sql);
    for (const auto &arg : args) query.addBindValue(arg);
    if (!query.exec()) return data;

    while (query.next()) {
        data.push_back(Person{query.value(0).toString(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString(), query.value(4).toString()});
    }
    return data;
}

void MainWindow::refreshTable(const QList<Person> &data) {
    table->setRowCount(data.size());
    for (int i = 0; i < data.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(data[i].name));
        table->setItem(i, 1, new QTableWidgetItem(data[i].gender));
        table->setItem(i, 2, new QTableWidgetItem(data[i].phone));
        table->setItem(i, 3, new QTableWidgetItem(data[i].qq));
        table->setItem(i, 4, new QTableWidgetItem(data[i].category));
    }
}

Person MainWindow::formPerson() const { return Person{nameEdit->text().trimmed(), genderBox->currentText(), phoneEdit->text().trimmed(), qqEdit->text().trimmed(), categoryEdit->text().trimmed()}; }

void MainWindow::clearForm() {
    nameEdit->clear(); phoneEdit->clear(); qqEdit->clear(); categoryEdit->clear(); genderBox->setCurrentIndex(0);
}

void MainWindow::addContact() {
    Person p = formPerson();
    if (p.name.isEmpty() || p.phone.isEmpty()) return QMessageBox::warning(this, "提示", "姓名和电话不能为空。");
    QSqlQuery query;
    query.prepare("INSERT INTO contacts(name, gender, phone, qq, category) VALUES(?,?,?,?,?)");
    query.addBindValue(p.name); query.addBindValue(p.gender); query.addBindValue(p.phone); query.addBindValue(p.qq); query.addBindValue(p.category);
    if (!query.exec()) return QMessageBox::warning(this, "提示", "添加失败（电话可能重复）。");
    showAllContacts();
    clearForm();
}

void MainWindow::modifyContact() {
    Person p = formPerson();
    if (p.name.isEmpty() || p.phone.isEmpty()) return QMessageBox::warning(this, "提示", "姓名和电话不能为空。");
    QSqlQuery query;
    query.prepare("UPDATE contacts SET gender=?, qq=?, category=? WHERE name=? AND phone=?");
    query.addBindValue(p.gender); query.addBindValue(p.qq); query.addBindValue(p.category); query.addBindValue(p.name); query.addBindValue(p.phone);
    if (!query.exec() || query.numRowsAffected() == 0) return QMessageBox::information(this, "提示", "未找到匹配记录（请确保姓名和电话一致）。");
    showAllContacts();
}

void MainWindow::deleteContact() {
    const QString name = nameEdit->text().trimmed();
    if (name.isEmpty()) return QMessageBox::warning(this, "提示", "请输入姓名后再删除。");
    QSqlQuery query;
    query.prepare("DELETE FROM contacts WHERE name = ?");
    query.addBindValue(name);
    query.exec();
    showAllContacts();
    clearForm();
}

void MainWindow::showAllContacts() { refreshTable(queryContacts()); }

void MainWindow::showByCategory() { refreshTable(queryContacts("LOWER(category)=LOWER(?)", {searchEdit->text().trimmed()})); }

void MainWindow::findByName() { refreshTable(queryContacts("LOWER(name)=LOWER(?)", {searchEdit->text().trimmed()})); }

void MainWindow::findByPhone() { refreshTable(queryContacts("phone=?", {searchEdit->text().trimmed()})); }

void MainWindow::saveContacts() {
    QFile file(dataFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return QMessageBox::critical(this, "错误", "无法写入 contacts.txt。");
    QTextStream out(&file);
    const auto data = queryContacts();
    for (const auto &p : data) out << p.name << '|' << p.gender << '|' << p.phone << '|' << p.qq << '|' << p.category << '\n';
    QMessageBox::information(this, "成功", "已将数据库同步到 contacts.txt。");
}

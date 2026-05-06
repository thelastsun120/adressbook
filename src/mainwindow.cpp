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
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dataFile("contacts.txt"), dbPort(3306) {
    loadDbConfig();
    setupUi();
    if (!initDatabase()) {
        QMessageBox::critical(this, "错误", "MySQL 数据库连接失败，请检查 config.ini。程序将退出。");
        close();
        return;
    }
    loadFromTextIfDbEmpty();
    showAllContacts();
}

MainWindow::~MainWindow() { QSqlDatabase::database().close(); }

void MainWindow::loadDbConfig() {
    QSettings settings("config.ini", QSettings::IniFormat);
    dbHost = settings.value("database/host", "127.0.0.1").toString();
    dbPort = settings.value("database/port", 3306).toInt();
    dbName = settings.value("database/name", "addressbook_db").toString();
    dbUser = settings.value("database/user", "root").toString();
    dbPassword = settings.value("database/password", "123456").toString();
}

void MainWindow::setupUi() {
    setWindowTitle("通讯录管理系统（Qt + MySQL）");
    resize(1020, 680);

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
    auto *modifyBtn = new QPushButton("按选中行修改", this);
    auto *deleteBtn = new QPushButton("按选中行删除", this);
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
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"ID", "姓名", "性别", "电话", "QQ", "类别"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setColumnHidden(0, true);
    table->setAlternatingRowColors(true);
    table->setStyleSheet("QTableWidget{background:#f8fbff;alternate-background-color:#edf4ff;gridline-color:#d3dff0;border:1px solid #c5d3e6;border-radius:6px;} QHeaderView::section{background:#2f80ed;color:white;font-weight:600;padding:6px;border:none;}");
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(table, 1);

    statusLabel = new QLabel("状态：就绪", this);
    statusLabel->setStyleSheet("color:#1f3a5f; font-weight:600;");
    mainLayout->addWidget(statusLabel);

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
    db.setHostName(dbHost);
    db.setPort(dbPort);
    db.setDatabaseName(dbName);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);
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

void MainWindow::loadFromTextIfDbEmpty() { QSqlQuery countQ("SELECT COUNT(*) FROM contacts"); if (!countQ.next() || countQ.value(0).toInt() > 0) return; QFile file(dataFile); if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return; QTextStream in(&file); QSqlQuery insert; insert.prepare("INSERT INTO contacts(name, gender, phone, qq, category) VALUES(?,?,?,?,?)"); while (!in.atEnd()) { const QStringList parts = in.readLine().trimmed().split('|'); if (parts.size()!=5) continue; for (const auto &v:parts) insert.addBindValue(v); insert.exec(); } }

QList<Person> MainWindow::queryContacts(const QString &whereClause, const QVariantList &args) const {
    QList<Person> data;
    QString sql = "SELECT name, gender, phone, qq, category FROM contacts";
    if (!whereClause.isEmpty()) sql += " WHERE " + whereClause;
    sql += " ORDER BY id ASC";
    QSqlQuery query;
    query.prepare(sql);
    for (const auto &arg : args) query.addBindValue(arg);
    if (!query.exec()) return data;
    while (query.next()) data.push_back(Person{query.value(0).toString(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString(), query.value(4).toString()});
    return data;
}

int MainWindow::selectedContactId() const {
    const int row = table->currentRow();
    if (row < 0) return -1;
    auto *item = table->item(row, 0);
    if (!item) return -1;
    return item->text().toInt();
}

void MainWindow::refreshTable(const QList<Person> &data) {
    QSqlQuery idQ("SELECT id, name, gender, phone, qq, category FROM contacts ORDER BY id ASC");
    QList<QVariantList> rows;
    while (idQ.next()) rows.append({idQ.value(0),idQ.value(1),idQ.value(2),idQ.value(3),idQ.value(4),idQ.value(5)});
    table->setRowCount(rows.size());
    for (int i=0;i<rows.size();++i) for (int c=0;c<6;++c) table->setItem(i,c,new QTableWidgetItem(rows[i][c].toString()));
    Q_UNUSED(data);
    statusLabel->setText(QString("状态：当前显示 %1 条记录").arg(table->rowCount()));
}

Person MainWindow::formPerson() const { return Person{nameEdit->text().trimmed(), genderBox->currentText(), phoneEdit->text().trimmed(), qqEdit->text().trimmed(), categoryEdit->text().trimmed()}; }
void MainWindow::clearForm() { nameEdit->clear(); phoneEdit->clear(); qqEdit->clear(); categoryEdit->clear(); genderBox->setCurrentIndex(0); }

void MainWindow::addContact() { Person p=formPerson(); if (p.name.isEmpty()||p.phone.isEmpty()) return QMessageBox::warning(this,"提示","姓名和电话不能为空。"); QSqlQuery q; q.prepare("INSERT INTO contacts(name,gender,phone,qq,category) VALUES(?,?,?,?,?)"); q.addBindValue(p.name);q.addBindValue(p.gender);q.addBindValue(p.phone);q.addBindValue(p.qq);q.addBindValue(p.category); if(!q.exec()) return QMessageBox::warning(this,"提示","添加失败（电话可能重复）。"); showAllContacts(); clearForm(); }

void MainWindow::modifyContact() { int id=selectedContactId(); if(id<0) return QMessageBox::information(this,"提示","请先选中一行再修改。"); Person p=formPerson(); if(p.name.isEmpty()||p.phone.isEmpty()) return QMessageBox::warning(this,"提示","姓名和电话不能为空。"); QSqlQuery q; q.prepare("UPDATE contacts SET name=?,gender=?,phone=?,qq=?,category=? WHERE id=?"); q.addBindValue(p.name);q.addBindValue(p.gender);q.addBindValue(p.phone);q.addBindValue(p.qq);q.addBindValue(p.category);q.addBindValue(id); if(!q.exec()) return QMessageBox::warning(this,"提示","修改失败（电话可能重复）。"); showAllContacts(); }

void MainWindow::deleteContact() { int id=selectedContactId(); if(id<0) return QMessageBox::information(this,"提示","请先选中一行再删除。"); if(QMessageBox::question(this,"确认删除","确定删除当前选中联系人吗？",QMessageBox::Yes|QMessageBox::No)!=QMessageBox::Yes) return; QSqlQuery q; q.prepare("DELETE FROM contacts WHERE id=?"); q.addBindValue(id); q.exec(); showAllContacts(); clearForm(); }

void MainWindow::showAllContacts() { refreshTable(queryContacts()); }
void MainWindow::showByCategory() { refreshTable(queryContacts("LOWER(category)=LOWER(?)", {searchEdit->text().trimmed()})); }
void MainWindow::findByName() { refreshTable(queryContacts("LOWER(name)=LOWER(?)", {searchEdit->text().trimmed()})); }
void MainWindow::findByPhone() { refreshTable(queryContacts("phone=?", {searchEdit->text().trimmed()})); }

void MainWindow::saveContacts() { QFile file(dataFile); if(!file.open(QIODevice::WriteOnly|QIODevice::Text)) return QMessageBox::critical(this,"错误","无法写入 contacts.txt。"); QTextStream out(&file); QSqlQuery q("SELECT name,gender,phone,qq,category FROM contacts ORDER BY id ASC"); while(q.next()) out<<q.value(0).toString()<<'|'<<q.value(1).toString()<<'|'<<q.value(2).toString()<<'|'<<q.value(3).toString()<<'|'<<q.value(4).toString()<<'\n'; QMessageBox::information(this,"成功","已将数据库同步到 contacts.txt。"); }

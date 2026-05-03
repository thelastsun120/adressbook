#include "mainwindow.h"

#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>

QString Person::serialize() const {
    return name + "|" + gender + "|" + phone + "|" + qq + "|" + category;
}

bool Person::deserialize(const QString &line, Person &out) {
    const QStringList parts = line.split('|');
    if (parts.size() != 5) return false;
    out = Person{parts[0], parts[1], parts[2], parts[3], parts[4]};
    return true;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), dataFile("contacts.txt") {
    setupUi();
    loadContacts();
    showAllContacts();
}

void MainWindow::setupUi() {
    setWindowTitle("通讯录管理系统（Qt UI）");
    resize(920, 600);

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *title = new QLabel("📒 通讯录管理系统", this);
    title->setStyleSheet("font-size: 24px; font-weight: bold; padding: 8px;");
    mainLayout->addWidget(title);

    auto *formBox = new QGroupBox("联系人信息", this);
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
    mainLayout->addWidget(formBox);

    auto *searchBox = new QGroupBox("查询", this);
    auto *searchLayout = new QHBoxLayout(searchBox);
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("输入姓名 / 电话 / 类别");
    searchLayout->addWidget(searchEdit);
    auto *findNameBtn = new QPushButton("按姓名查找", this);
    auto *findPhoneBtn = new QPushButton("按电话查找", this);
    auto *showCategoryBtn = new QPushButton("分类显示", this);
    searchLayout->addWidget(findNameBtn);
    searchLayout->addWidget(findPhoneBtn);
    searchLayout->addWidget(showCategoryBtn);
    mainLayout->addWidget(searchBox);

    auto *buttonLayout = new QHBoxLayout();
    auto *addBtn = new QPushButton("添加", this);
    auto *modifyBtn = new QPushButton("修改", this);
    auto *deleteBtn = new QPushButton("删除", this);
    auto *showAllBtn = new QPushButton("显示全部", this);
    auto *saveBtn = new QPushButton("保存", this);
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(modifyBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(showAllBtn);
    buttonLayout->addWidget(saveBtn);
    mainLayout->addLayout(buttonLayout);

    table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"姓名", "性别", "电话", "QQ", "类别"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(table);

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

void MainWindow::loadContacts() {
    QFile file(dataFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "提示", "未找到 contacts.txt，已创建空通讯录。");
        return;
    }

    contacts.clear();
    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        Person person;
        if (Person::deserialize(line, person)) {
            contacts.push_back(person);
        }
    }
}

void MainWindow::refreshTable(const QVector<Person> &data) {
    table->setRowCount(data.size());
    for (int i = 0; i < data.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(data[i].name));
        table->setItem(i, 1, new QTableWidgetItem(data[i].gender));
        table->setItem(i, 2, new QTableWidgetItem(data[i].phone));
        table->setItem(i, 3, new QTableWidgetItem(data[i].qq));
        table->setItem(i, 4, new QTableWidgetItem(data[i].category));
    }
}

Person MainWindow::formPerson() const {
    return Person{nameEdit->text().trimmed(), genderBox->currentText(), phoneEdit->text().trimmed(), qqEdit->text().trimmed(), categoryEdit->text().trimmed()};
}

int MainWindow::findIndexByName(const QString &name) const {
    for (int i = 0; i < contacts.size(); ++i) {
        if (contacts[i].name.compare(name, Qt::CaseInsensitive) == 0) return i;
    }
    return -1;
}

void MainWindow::clearForm() {
    nameEdit->clear();
    phoneEdit->clear();
    qqEdit->clear();
    categoryEdit->clear();
    genderBox->setCurrentIndex(0);
}

void MainWindow::addContact() {
    Person p = formPerson();
    if (p.name.isEmpty() || p.phone.isEmpty()) {
        QMessageBox::warning(this, "提示", "姓名和电话不能为空。");
        return;
    }
    contacts.push_back(p);
    showAllContacts();
    clearForm();
}

void MainWindow::modifyContact() {
    const QString name = nameEdit->text().trimmed();
    const int index = findIndexByName(name);
    if (index < 0) {
        QMessageBox::information(this, "提示", "未找到同名联系人。");
        return;
    }
    Person p = formPerson();
    if (p.name.isEmpty() || p.phone.isEmpty()) {
        QMessageBox::warning(this, "提示", "姓名和电话不能为空。");
        return;
    }
    contacts[index] = p;
    showAllContacts();
}

void MainWindow::deleteContact() {
    const QString name = nameEdit->text().trimmed();
    const int index = findIndexByName(name);
    if (index < 0) {
        QMessageBox::information(this, "提示", "未找到同名联系人。");
        return;
    }
    contacts.remove(index);
    showAllContacts();
    clearForm();
}

void MainWindow::showAllContacts() {
    refreshTable(contacts);
}

void MainWindow::showByCategory() {
    const QString target = searchEdit->text().trimmed();
    QVector<Person> filtered;
    for (const auto &p : contacts) {
        if (p.category.compare(target, Qt::CaseInsensitive) == 0) filtered.push_back(p);
    }
    refreshTable(filtered);
}

void MainWindow::findByName() {
    const QString target = searchEdit->text().trimmed();
    QVector<Person> filtered;
    for (const auto &p : contacts) {
        if (p.name.compare(target, Qt::CaseInsensitive) == 0) filtered.push_back(p);
    }
    refreshTable(filtered);
}

void MainWindow::findByPhone() {
    const QString target = searchEdit->text().trimmed();
    QVector<Person> filtered;
    for (const auto &p : contacts) {
        if (p.phone == target) filtered.push_back(p);
    }
    refreshTable(filtered);
}

void MainWindow::saveContacts() {
    QFile file(dataFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "保存失败，无法写入 contacts.txt。");
        return;
    }
    QTextStream out(&file);
    for (const auto &p : contacts) out << p.serialize() << '\n';
    QMessageBox::information(this, "成功", "通讯录保存成功。");
}

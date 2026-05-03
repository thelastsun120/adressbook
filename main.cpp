#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Person {
public:
    string name;
    string gender;
    string phone;
    string qq;
    string category;

    Person() = default;
    Person(string name, string gender, string phone, string qq, string category)
        : name(std::move(name)),
          gender(std::move(gender)),
          phone(std::move(phone)),
          qq(std::move(qq)),
          category(std::move(category)) {}

    friend istream& operator>>(istream& in, Person& p) {
        cout << "姓名: ";
        getline(in >> ws, p.name);
        cout << "性别: ";
        getline(in, p.gender);
        cout << "电话: ";
        getline(in, p.phone);
        cout << "QQ号: ";
        getline(in, p.qq);
        cout << "类别(亲人/同学/朋友等): ";
        getline(in, p.category);
        return in;
    }

    friend ostream& operator<<(ostream& out, const Person& p) {
        out << left << setw(12) << p.name << setw(8) << p.gender << setw(16) << p.phone << setw(14)
            << p.qq << setw(10) << p.category;
        return out;
    }

    string serialize() const {
        return name + "|" + gender + "|" + phone + "|" + qq + "|" + category;
    }

    static bool deserialize(const string& line, Person& p) {
        stringstream ss(line);
        vector<string> fields;
        string part;
        while (getline(ss, part, '|')) {
            fields.push_back(part);
        }
        if (fields.size() != 5) {
            return false;
        }
        p = Person(fields[0], fields[1], fields[2], fields[3], fields[4]);
        return true;
    }
};

class AddressBook {
private:
    vector<Person> contacts;
    string filename;

    static string toLower(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
        return s;
    }

public:
    explicit AddressBook(string file) : filename(std::move(file)) {}

    bool loadFromFile() {
        ifstream fin(filename);
        if (!fin.is_open()) {
            return false;
        }

        contacts.clear();
        string line;
        while (getline(fin, line)) {
            if (line.empty()) {
                continue;
            }
            Person p;
            if (Person::deserialize(line, p)) {
                contacts.push_back(p);
            }
        }
        return true;
    }

    bool saveToFile() const {
        ofstream fout(filename);
        if (!fout.is_open()) {
            return false;
        }
        for (const auto& p : contacts) {
            fout << p.serialize() << '\n';
        }
        return true;
    }

    void addPerson(const Person& p) { contacts.push_back(p); }

    void showAll() const {
        if (contacts.empty()) {
            cout << "通讯录为空。\n";
            return;
        }
        cout << left << setw(12) << "姓名" << setw(8) << "性别" << setw(16) << "电话" << setw(14) << "QQ号"
             << setw(10) << "类别" << '\n';
        cout << string(60, '-') << '\n';
        for (const auto& p : contacts) {
            cout << p << '\n';
        }
    }

    void showByCategory(const string& category) const {
        bool found = false;
        for (const auto& p : contacts) {
            if (toLower(p.category) == toLower(category)) {
                if (!found) {
                    cout << left << setw(12) << "姓名" << setw(8) << "性别" << setw(16) << "电话" << setw(14)
                         << "QQ号" << setw(10) << "类别" << '\n';
                    cout << string(60, '-') << '\n';
                }
                found = true;
                cout << p << '\n';
            }
        }
        if (!found) {
            cout << "未找到该类别联系人。\n";
        }
    }

    void findByName(const string& name) const {
        bool found = false;
        for (const auto& p : contacts) {
            if (toLower(p.name) == toLower(name)) {
                if (!found) {
                    cout << "找到以下联系人:\n";
                }
                found = true;
                cout << p << '\n';
            }
        }
        if (!found) {
            cout << "未找到姓名为 " << name << " 的联系人。\n";
        }
    }

    void findByPhone(const string& phone) const {
        bool found = false;
        for (const auto& p : contacts) {
            if (p.phone == phone) {
                if (!found) {
                    cout << "找到以下联系人:\n";
                }
                found = true;
                cout << p << '\n';
            }
        }
        if (!found) {
            cout << "未找到电话为 " << phone << " 的联系人。\n";
        }
    }

    bool deleteByName(const string& name) {
        auto oldSize = contacts.size();
        contacts.erase(remove_if(contacts.begin(), contacts.end(),
                                 [&](const Person& p) { return toLower(p.name) == toLower(name); }),
                       contacts.end());
        return contacts.size() != oldSize;
    }

    bool modifyByName(const string& name) {
        for (auto& p : contacts) {
            if (toLower(p.name) == toLower(name)) {
                cout << "请输入新的信息：\n";
                cin >> p;
                return true;
            }
        }
        return false;
    }
};

void showMenu() {
    cout << "\n===== 通讯录管理系统 =====\n"
         << "1. 添加联系人\n"
         << "2. 根据姓名修改记录\n"
         << "3. 显示所有记录\n"
         << "4. 分类显示\n"
         << "5. 查找记录(按姓名/电话)\n"
         << "6. 删除记录\n"
         << "7. 保存记录\n"
         << "0. 退出\n"
         << "请选择功能: ";
}

int main() {
    AddressBook book("contacts.txt");

    if (book.loadFromFile()) {
        cout << "已从 contacts.txt 初始化通讯录。\n";
    } else {
        cout << "未找到初始化文件，请确认 contacts.txt 是否存在。\n";
    }

    while (true) {
        showMenu();

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "输入无效，请输入数字。\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) {
            cout << "退出系统。\n";
            break;
        }

        if (choice == 1) {
            Person p;
            cin >> p;
            book.addPerson(p);
            cout << "添加成功。\n";
        } else if (choice == 2) {
            string name;
            cout << "请输入要修改的姓名: ";
            getline(cin, name);
            if (book.modifyByName(name)) {
                cout << "修改成功。\n";
            } else {
                cout << "未找到该联系人。\n";
            }
        } else if (choice == 3) {
            book.showAll();
        } else if (choice == 4) {
            string category;
            cout << "请输入类别: ";
            getline(cin, category);
            book.showByCategory(category);
        } else if (choice == 5) {
            int sub;
            cout << "1. 按姓名查找  2. 按电话查找\n请选择: ";
            cin >> sub;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (sub == 1) {
                string name;
                cout << "姓名: ";
                getline(cin, name);
                book.findByName(name);
            } else if (sub == 2) {
                string phone;
                cout << "电话: ";
                getline(cin, phone);
                book.findByPhone(phone);
            } else {
                cout << "无效选择。\n";
            }
        } else if (choice == 6) {
            string name;
            cout << "请输入要删除的姓名: ";
            getline(cin, name);
            if (book.deleteByName(name)) {
                cout << "删除成功。\n";
            } else {
                cout << "未找到该联系人。\n";
            }
        } else if (choice == 7) {
            if (book.saveToFile()) {
                cout << "保存成功。\n";
            } else {
                cout << "保存失败。\n";
            }
        } else {
            cout << "无效菜单选项。\n";
        }
    }

    return 0;
}

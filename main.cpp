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
        cout << "  姓名    : ";
        getline(in >> ws, p.name);
        cout << "  性别    : ";
        getline(in, p.gender);
        cout << "  电话    : ";
        getline(in, p.phone);
        cout << "  QQ号    : ";
        getline(in, p.qq);
        cout << "  类别    : ";
        getline(in, p.category);
        return in;
    }

    friend ostream& operator<<(ostream& out, const Person& p) {
        out << "| " << left << setw(10) << p.name << " | " << setw(6) << p.gender << " | " << setw(13) << p.phone
            << " | " << setw(10) << p.qq << " | " << setw(8) << p.category << " |";
        return out;
    }

    string serialize() const { return name + "|" + gender + "|" + phone + "|" + qq + "|" + category; }

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

    static void printTableHeader() {
        cout << "+------------+--------+---------------+------------+----------+\n";
        cout << "| 姓名       | 性别   | 电话          | QQ号       | 类别     |\n";
        cout << "+------------+--------+---------------+------------+----------+\n";
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
            if (line.empty()) continue;
            Person p;
            if (Person::deserialize(line, p)) contacts.push_back(p);
        }
        return true;
    }

    bool saveToFile() const {
        ofstream fout(filename);
        if (!fout.is_open()) return false;
        for (const auto& p : contacts) fout << p.serialize() << '\n';
        return true;
    }

    void addPerson(const Person& p) { contacts.push_back(p); }

    void showAll() const {
        if (contacts.empty()) {
            cout << "\n[提示] 通讯录为空。\n";
            return;
        }
        printTableHeader();
        for (const auto& p : contacts) cout << p << '\n';
        cout << "+------------+--------+---------------+------------+----------+\n";
    }

    void showByCategory(const string& category) const {
        bool found = false;
        for (const auto& p : contacts) {
            if (toLower(p.category) == toLower(category)) {
                if (!found) printTableHeader();
                found = true;
                cout << p << '\n';
            }
        }
        if (found) {
            cout << "+------------+--------+---------------+------------+----------+\n";
        } else {
            cout << "\n[提示] 未找到该类别联系人。\n";
        }
    }

    void findByName(const string& name) const {
        bool found = false;
        for (const auto& p : contacts) {
            if (toLower(p.name) == toLower(name)) {
                if (!found) {
                    cout << "\n[查询结果]\n";
                    printTableHeader();
                }
                found = true;
                cout << p << '\n';
            }
        }
        if (found) cout << "+------------+--------+---------------+------------+----------+\n";
        if (!found) cout << "\n[提示] 未找到姓名为 " << name << " 的联系人。\n";
    }

    void findByPhone(const string& phone) const {
        bool found = false;
        for (const auto& p : contacts) {
            if (p.phone == phone) {
                if (!found) {
                    cout << "\n[查询结果]\n";
                    printTableHeader();
                }
                found = true;
                cout << p << '\n';
            }
        }
        if (found) cout << "+------------+--------+---------------+------------+----------+\n";
        if (!found) cout << "\n[提示] 未找到电话为 " << phone << " 的联系人。\n";
    }

    bool deleteByName(const string& name) {
        auto oldSize = contacts.size();
        contacts.erase(remove_if(contacts.begin(), contacts.end(), [&](const Person& p) { return toLower(p.name) == toLower(name); }), contacts.end());
        return contacts.size() != oldSize;
    }

    bool modifyByName(const string& name) {
        for (auto& p : contacts) {
            if (toLower(p.name) == toLower(name)) {
                cout << "\n[原记录]\n";
                printTableHeader();
                cout << p << '\n';
                cout << "+------------+--------+---------------+------------+----------+\n";
                cout << "\n请输入新的信息：\n";
                cin >> p;
                return true;
            }
        }
        return false;
    }
};

void printTitle() {
    cout << "\n====================================================\n";
    cout << "               通 讯 录 管 理 系 统               \n";
    cout << "====================================================\n";
}

void showMenu() {
    printTitle();
    cout << "  [1] 添加联系人\n";
    cout << "  [2] 根据姓名修改记录\n";
    cout << "  [3] 显示所有记录\n";
    cout << "  [4] 分类显示\n";
    cout << "  [5] 查找记录 (按姓名/电话)\n";
    cout << "  [6] 删除记录\n";
    cout << "  [7] 保存记录\n";
    cout << "  [0] 退出系统\n";
    cout << "----------------------------------------------------\n";
    cout << "请选择功能: ";
}

int main() {
    AddressBook book("contacts.txt");

    if (book.loadFromFile()) cout << "[系统] 已从 contacts.txt 初始化通讯录。\n";
    else cout << "[系统] 未找到初始化文件，请确认 contacts.txt 是否存在。\n";

    while (true) {
        showMenu();

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\n[错误] 输入无效，请输入数字。\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) {
            cout << "\n[系统] 已退出。\n";
            break;
        }

        if (choice == 1) {
            Person p;
            cout << "\n[添加联系人]\n";
            cin >> p;
            book.addPerson(p);
            cout << "[成功] 添加成功。\n";
        } else if (choice == 2) {
            string name;
            cout << "请输入要修改的姓名: ";
            getline(cin, name);
            if (book.modifyByName(name)) cout << "[成功] 修改成功。\n";
            else cout << "[提示] 未找到该联系人。\n";
        } else if (choice == 3) {
            cout << "\n[全部联系人]\n";
            book.showAll();
        } else if (choice == 4) {
            string category;
            cout << "请输入类别: ";
            getline(cin, category);
            cout << "\n[类别: " << category << "]\n";
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
                cout << "[提示] 无效选择。\n";
            }
        } else if (choice == 6) {
            string name;
            cout << "请输入要删除的姓名: ";
            getline(cin, name);
            if (book.deleteByName(name)) cout << "[成功] 删除成功。\n";
            else cout << "[提示] 未找到该联系人。\n";
        } else if (choice == 7) {
            if (book.saveToFile()) cout << "[成功] 保存成功。\n";
            else cout << "[错误] 保存失败。\n";
        } else {
            cout << "[提示] 无效菜单选项。\n";
        }
    }

    return 0;
}

#include <QApplication>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

QLineEdit *nameInput;
QLineEdit *ageInput;
QLineEdit *courseInput;
QWidget *mainWindow;

// ---------- Simple function like your sayHello() style ----------
void addStudent() {
    QString name = nameInput->text();
    QString age = ageInput->text();
    QString course = courseInput->text();

    if (name.isEmpty() || age.isEmpty() || course.isEmpty()) {
        QMessageBox::warning(mainWindow, "Error", "Please fill all fields!");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO students (name, age, course) VALUES (:n, :a, :c)");
    query.bindValue(":n", name);
    query.bindValue(":a", age.toInt());
    query.bindValue(":c", course);

    if (!query.exec()) {
        QMessageBox::critical(mainWindow, "DB Error", query.lastError().text());
        return;
    }

    QMessageBox::information(mainWindow, "Success", "Student added!");

    nameInput->clear();
    ageInput->clear();
    courseInput->clear();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // ---------- Database Connection ----------
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("school_db");
    db.setUserName("root");
    db.setPassword("");

    if (!db.open()) {
        qDebug() << "DB ERROR:" << db.lastError().text();
        return -1;
    }

    // ---------- UI ----------
    mainWindow = new QWidget;
    mainWindow->setWindowTitle("Add Student");

    nameInput = new QLineEdit();
    ageInput = new QLineEdit();
    courseInput = new QLineEdit();

    QPushButton *btn = new QPushButton("Add Student");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel("Name:"));
    layout->addWidget(nameInput);

    layout->addWidget(new QLabel("Age:"));
    layout->addWidget(ageInput);

    layout->addWidget(new QLabel("Course:"));
    layout->addWidget(courseInput);

    layout->addWidget(btn);

    mainWindow->setLayout(layout);

    // ---------- Connect button just like your example ----------
    QObject::connect(btn, &QPushButton::clicked, &addStudent);

    mainWindow->show();
    return app.exec();
}
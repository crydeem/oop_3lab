#include <QApplication>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <algorithm>

// Модель хранит числа, следит за порядком и сохраняет их.
class NumbersModel : public QObject {
    Q_OBJECT
private:
    static constexpr int minValue = 0;
    static constexpr int maxValue = 100;

    int a = 10;
    int b = 50;
    int c = 90;

    void save() const {
        QSettings settings;
        settings.setValue("a", a);
        settings.setValue("b", b);
        settings.setValue("c", c);
    }

    void notifyIfChanged(int oldA, int oldB, int oldC) {
        if (a == oldA && b == oldB && c == oldC)
            return;
        save();
        emit valuesChanged();
    }

public:
    NumbersModel() {
        QSettings settings;
        a = std::clamp(settings.value("a", a).toInt(), minValue, maxValue);
        b = std::clamp(settings.value("b", b).toInt(), a, maxValue);
        c = std::clamp(settings.value("c", c).toInt(), b, maxValue);
    }

    int valueA() const { return a; }
    int valueB() const { return b; }
    int valueC() const { return c; }
    int minimumValue() const { return minValue; }
    int maximumValue() const { return maxValue; }
    int minimumB() const { return a; }
    int maximumB() const { return c; }

    void initialize() { emit valuesChanged(); }

    void setA(int value) {
        int oldA = a, oldB = b, oldC = c;
        a = std::clamp(value, minValue, maxValue);
        b = std::max(a, b);
        c = std::max(b, c);
        notifyIfChanged(oldA, oldB, oldC);
    }

    void setB(int value) {
        int oldA = a, oldB = b, oldC = c;
        b = std::clamp(value, a, c);
        notifyIfChanged(oldA, oldB, oldC);
    }

    void setC(int value) {
        int oldA = a, oldB = b, oldC = c;
        c = std::clamp(value, minValue, maxValue);
        b = std::min(b, c);
        a = std::min(a, b);
        notifyIfChanged(oldA, oldB, oldC);
    }

    ~NumbersModel() { save(); }

signals:
    void valuesChanged();
};

class MainWindow : public QMainWindow {
private:
    NumbersModel model;
    QLineEdit* text[3];
    QSpinBox* number[3];
    QSlider* slider[3];
    QLabel* updateCountLabel;
    int updateCount = 0;
    bool updating = false;

    void editNumber(int index, int value) {
        if (index == 0) model.setA(value);
        else if (index == 1) model.setB(value);
        else model.setC(value);
    }

    void finishTextEdit(int index) {
        bool ok = false;
        int value = text[index]->text().toInt(&ok);
        if (ok)
            editNumber(index, value);
        else
            showValues();
    }

    void showValues() {
        ++updateCount;
        updating = true;

        int values[3] = {model.valueA(), model.valueB(), model.valueC()};
        for (int i = 0; i < 3; ++i) {
            number[i]->setRange(model.minimumValue(), model.maximumValue());
            slider[i]->setRange(model.minimumValue(), model.maximumValue());
        }
        number[1]->setRange(model.minimumB(), model.maximumB());

        for (int i = 0; i < 3; ++i) {
            text[i]->setText(QString::number(values[i]));
            number[i]->setValue(values[i]);
            slider[i]->setValue(values[i]);
        }

        updating = false;
        updateCountLabel->setText(QString("Обновлений формы: %1").arg(updateCount));
        qInfo("Обновление формы: %d", updateCount);
    }

public:
    MainWindow() {
        setWindowTitle("ЛР3. Часть 2 — MVC");

        auto* central = new QWidget;
        auto* layout = new QVBoxLayout(central);
        auto* title = new QLabel("A   ≤   B   ≤   C");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        auto* form = new QFormLayout;
        const char* names[3] = {"A", "B", "C"};

        for (int i = 0; i < 3; ++i) {
            text[i] = new QLineEdit;
            number[i] = new QSpinBox;
            slider[i] = new QSlider(Qt::Horizontal);
            slider[i]->setTracking(i == 1);

            auto* row = new QWidget;
            auto* rowLayout = new QVBoxLayout(row);
            rowLayout->setContentsMargins(0, 0, 0, 0);
            rowLayout->addWidget(text[i]);
            rowLayout->addWidget(number[i]);
            rowLayout->addWidget(slider[i]);
            form->addRow(names[i], row);

            connect(text[i], &QLineEdit::editingFinished, this, [this, i] { finishTextEdit(i); });
            connect(number[i], &QSpinBox::valueChanged, this, [this, i](int value) {
                if (!updating) editNumber(i, value);
            });
            connect(slider[i], &QSlider::valueChanged, this, [this, i](int value) {
                if (updating)
                    return;

                editNumber(i, value);
                if (i == 1 && value != model.valueB()) {
                    updating = true;
                    slider[i]->setValue(model.valueB());
                    updating = false;
                }
            });
        }

        layout->addLayout(form);
        updateCountLabel = new QLabel;
        updateCountLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(updateCountLabel);
        setCentralWidget(central);

        connect(&model, &NumbersModel::valuesChanged, this, &MainWindow::showValues);
        model.initialize();
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("SultanovTimur");
    QCoreApplication::setApplicationName("OOPLab3MVC");
    MainWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"

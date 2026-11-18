#include <QApplication>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QStatusBar>
#include <QWidget>
#include <functional>
#include <vector>

// Круг хранит свои координаты и сам проверяет попадание мыши.
class CCircle {
private:
    QPoint center;
    static constexpr int radius = 30;
    bool selected;

public:
    CCircle(int x, int y) : center(x, y), selected(true) {}

    bool contains(const QPoint& point) const {
        int dx = point.x() - center.x();
        int dy = point.y() - center.y();
        return dx * dx + dy * dy <= radius * radius;
    }

    void setSelected(bool value) { selected = value; }
    void toggleSelected() { selected = !selected; }
    bool isSelected() const { return selected; }

    void draw(QPainter& painter) const {
        painter.setBrush(selected ? QColor(255, 170, 90) : QColor(120, 190, 240));
        painter.setPen(selected ? QPen(Qt::red, 3) : QPen(Qt::darkBlue, 2));
        painter.drawEllipse(center, radius, radius);
    }
};

// Собственный контейнер хранит объекты и скрывает внутренний vector от формы.
class CircleStorage {
private:
    std::vector<CCircle> circles;

public:
    void add(int x, int y) { circles.emplace_back(x, y); }

    bool selectAt(const QPoint& point, bool controlPressed) {
        bool found = false;
        for (CCircle& circle : circles) {
            bool hit = circle.contains(point);
            if (hit) {
                found = true;
                if (controlPressed)
                    circle.toggleSelected();
                else
                    circle.setSelected(true);
            } else if (!controlPressed) {
                circle.setSelected(false);
            }
        }
        return found;
    }

    void drawAll(QPainter& painter) const {
        for (const CCircle& circle : circles)
            circle.draw(painter);
    }

    void deleteSelected() {
        for (auto it = circles.begin(); it != circles.end();) {
            if (it->isSelected())
                it = circles.erase(it);
            else
                ++it;
        }
    }
};

class CircleCanvas : public QWidget {
private:
    CircleStorage storage;
    int resizeCount = 0;

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);
        storage.drawAll(painter);
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() != Qt::LeftButton)
            return;

        bool controlPressed = event->modifiers().testFlag(Qt::ControlModifier);
        if (!storage.selectAt(event->pos(), controlPressed))
            storage.add(event->pos().x(), event->pos().y());
        update();
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Delete) {
            storage.deleteSelected();
            update();
        } else {
            QWidget::keyPressEvent(event);
        }
    }

    void resizeEvent(QResizeEvent* event) override {
        ++resizeCount;
        QWidget::resizeEvent(event);
        if (onResize)
            onResize(event->size(), resizeCount);
    }

public:
    std::function<void(const QSize&, int)> onResize;
};

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        auto* canvas = new CircleCanvas;
        canvas->setFocusPolicy(Qt::StrongFocus);
        setCentralWidget(canvas);
        resize(800, 600);
        setWindowTitle("ЛР3. Часть 1 — Круги на форме");
        statusBar()->showMessage("ЛКМ — создать/выбрать, Ctrl+ЛКМ — добавить к выбору, Del — удалить");
        canvas->onResize = [this](const QSize& size, int count) {
            statusBar()->showMessage(QString("Размер: %1 × %2 | изменений размера: %3")
                                     .arg(size.width()).arg(size.height()).arg(count));
        };
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}


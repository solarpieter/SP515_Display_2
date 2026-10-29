#include "numerickeypad.h"

#include <QApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QAbstractSpinBox>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QTimer>

NumericKeypad::NumericKeypad(QWidget *parentPage)
    : QFrame(parentPage)
{
    setObjectName("numericKeypad");
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);          // opaque, so it covers what is behind it
    setFocusPolicy(Qt::NoFocus);
    buildButtons();
    hide();                               // hidden until an edit is tapped
}

void NumericKeypad::buildButtons()
{
    auto *grid = new QGridLayout(this);
    grid->setSpacing(6);
    m_grid = grid;

    // Helper: creates a button that never takes focus, so the line edit
    // keeps focus (and the cursor) while the keypad is used.
    auto makeButton = [this](const QString &label) {
        auto *b = new QPushButton(label, this);
        b->setFocusPolicy(Qt::NoFocus);
        b->setMinimumSize(70, 60);
        m_buttons.append(b);
        return b;
    };

    // Digits 7 8 9 / 4 5 6 / 1 2 3
    const int rows[3][3] = {{7, 8, 9}, {4, 5, 6}, {1, 2, 3}};
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            const int d = rows[r][c];
            QPushButton *b = makeButton(QString::number(d));
            connect(b, &QPushButton::clicked, this, [this, d]() {
                sendKey(Qt::Key_0 + d, QString::number(d));
            });
            grid->addWidget(b, r, c);
        }
    }

    // Bottom row: - 0 .
    QPushButton *minus = makeButton("-");
    connect(minus, &QPushButton::clicked, this, [this]() { sendKey(Qt::Key_Minus, "-"); });
    grid->addWidget(minus, 3, 0);

    QPushButton *zero = makeButton("0");
    connect(zero, &QPushButton::clicked, this, [this]() { sendKey(Qt::Key_0, "0"); });
    grid->addWidget(zero, 3, 1);

    QPushButton *dot = makeButton(".");
    connect(dot, &QPushButton::clicked, this, [this]() { sendKey(Qt::Key_Period, "."); });
    grid->addWidget(dot, 3, 2);

    // Right column: backspace and enter
    QPushButton *back = makeButton(QString::fromUtf8("\xE2\x8C\xAB")); // erase-to-left symbol
    back->setAutoRepeat(true);                                          // hold to keep deleting
    connect(back, &QPushButton::clicked, this, [this]() { sendKey(Qt::Key_Backspace, QString()); });
    grid->addWidget(back, 0, 3);

    QPushButton *enter = makeButton("Enter");
    enter->setObjectName("keypadEnter");
    enter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);   // stretch to fill its 3 rows
    connect(enter, &QPushButton::clicked, this, [this]() {
        if (m_target) {
            sendKey(Qt::Key_Return, "\r");   // fires returnPressed() / editingFinished()
            m_target->clearFocus();
        }
        hide();
    });
    grid->addWidget(enter, 1, 3, 3, 1);       // tall button spanning three rows
}

void NumericKeypad::setKeySize(int width, int height)
{
    for (QPushButton *b : m_buttons)
        b->setMinimumSize(width, height);

    m_grid->setSpacing(qMax(6, height / 9));

    // The theme sets the button text size, so set it here the same way
    setStyleSheet(QString("QPushButton { font-size: %1px; }").arg(qRound(height * 0.38)));

    adjustSize();
    if (isVisible())
        reposition();
}

void NumericKeypad::registerEdit(QWidget *edit)
{
    edit->setFocusPolicy(Qt::ClickFocus);     // only take focus when tapped
    edit->installEventFilter(this);
    // A spin box does its typing in an internal line edit, which is the
    // widget that actually receives focus, so watch that one too.
    if (auto *spin = qobject_cast<QAbstractSpinBox *>(edit))
        if (QLineEdit *inner = spin->findChild<QLineEdit *>())
            inner->installEventFilter(this);
    m_edits.insert(edit);
}

// Returns the registered edit that "obj" is, or is the internal editor of.
QWidget *NumericKeypad::registeredFor(QObject *obj) const
{
    auto *w = qobject_cast<QWidget *>(obj);
    if (!w)
        return nullptr;
    if (m_edits.contains(w))
        return w;
    QWidget *p = w->parentWidget();           // spin box -> its internal line edit
    if (p && m_edits.contains(p) && qobject_cast<QAbstractSpinBox *>(p))
        return p;
    return nullptr;
}

bool NumericKeypad::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *edit = registeredFor(watched);
    if (edit) {
        if (event->type() == QEvent::FocusIn) {
            showForEdit(edit);
        } else if (event->type() == QEvent::FocusOut) {
            // Wait a moment: if focus moved to another registered edit the
            // keypad stays; if it went anywhere else, hide it.
            QTimer::singleShot(0, this, [this]() { hideIfNoEditFocused(); });
        }
    }
    return QFrame::eventFilter(watched, event);
}

void NumericKeypad::showForEdit(QWidget *edit)
{
    m_target = edit;
    reposition();
    show();
    raise();
    // Select the old value so the first key typed replaces it.
    QTimer::singleShot(0, edit, [edit]() {
        if (auto *dt = qobject_cast<QDateTimeEdit *>(edit))
            dt->setSelectedSection(dt->currentSection());   // date/time: select the tapped section
        else if (auto *spin = qobject_cast<QAbstractSpinBox *>(edit))
            spin->selectAll();
        else if (auto *line = qobject_cast<QLineEdit *>(edit))
            line->selectAll();
    });
}

void NumericKeypad::hideIfNoEditFocused()
{
    if (!registeredFor(QApplication::focusWidget()))
        hide();
}

void NumericKeypad::reposition()
{
    QWidget *page = parentWidget();
    if (!page)
        return;
    adjustSize();

    // Work out which part of the page is really visible: the page can be
    // taller than the window, which would cut the keypad off.
    QRect visible = page->rect();
    if (QWidget *win = window()) {
        const QPoint winTopLeft = page->mapFromGlobal(win->mapToGlobal(QPoint(0, 0)));
        visible &= QRect(winTopLeft, win->size());
    }

    const int margin = 10;
    const int x = visible.x() + visible.width() - width() - margin;
    const int y = visible.y() + visible.height() - height() - margin;
    move(qMax(visible.x(), x), qMax(visible.y(), y));
}

void NumericKeypad::sendKey(int key, const QString &text)
{
    if (!m_target)
        return;
    QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, text);
    QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier, text);
    QApplication::sendEvent(m_target, &press);    // synchronous, so order is guaranteed
    QApplication::sendEvent(m_target, &release);
}

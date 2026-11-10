#include "textkeyboard.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace {

// Characters on the four character rows, for the letter layer and the symbol layer.
// An empty entry is a key that is not used on that layer.
const char *const letterRows[4] = { "1234567890", "qwertyuiop", "asdfghjkl", "zxcvbnm,." };
const char *const symbolRows[4] = { "!@#$%^&*()", "-_=+[]{}\\|", ";:'\"~`<>/", ",.?      " };

} // namespace

TextKeyboard::TextKeyboard(QWidget *parentPage)
    : QFrame(parentPage)
{
    setObjectName("textKeyboard");
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);            // opaque, so it covers what is behind it
    setFocusPolicy(Qt::NoFocus);
    buildKeys();
    setKeyHeight(60);
    hide();                                 // hidden until an edit is tapped
}

void TextKeyboard::buildKeys()
{
    auto *outer = new QVBoxLayout(this);
    outer->setSpacing(6);

    // Buttons never take focus, so the line edit keeps focus (and the cursor) while typing
    auto makeButton = [this](const QString &label) {
        auto *b = new QPushButton(label, this);
        b->setFocusPolicy(Qt::NoFocus);
        b->setMinimumWidth(40);
        m_allButtons.append(b);
        return b;
    };

    const int rowLength[4] = { 10, 10, 9, 9 };
    for (int row = 0; row < 4; ++row) {
        auto *line = new QHBoxLayout;
        line->setSpacing(6);

        if (row == 2)
            line->addStretch(1);            // the third row is indented a little
        if (row == 3) {
            m_shift = makeButton("Shift");
            connect(m_shift, &QPushButton::clicked, this, [this]() {
                // once -> caps lock -> off
                m_shiftState = m_shiftState == ShiftOff ? ShiftOnce
                             : (m_shiftState == ShiftOnce ? ShiftLock : ShiftOff);
                refreshLabels();
            });
            line->addWidget(m_shift, 3);
        }

        for (int col = 0; col < rowLength[row]; ++col) {
            Key k;
            k.row = row;
            k.column = col;
            k.button = makeButton(QString());
            const int index = m_keys.size();
            connect(k.button, &QPushButton::clicked, this, [this, index]() { typeKey(index); });
            m_keys.append(k);
            line->addWidget(k.button, 2);
        }

        if (row == 0) {
            QPushButton *back = makeButton(QString::fromUtf8("\xE2\x8C\xAB"));   // erase-to-left symbol
            back->setAutoRepeat(true);                                             // hold to keep deleting
            connect(back, &QPushButton::clicked, this, [this]() { sendKey(Qt::Key_Backspace, QString()); });
            line->addWidget(back, 3);
        }
        if (row == 2)
            line->addStretch(1);
        outer->addLayout(line);
    }

    // Bottom row: layer switch, space, hide, enter
    auto *bottom = new QHBoxLayout;
    bottom->setSpacing(6);
    m_layer = makeButton("?123");
    connect(m_layer, &QPushButton::clicked, this, [this]() {
        m_symbols = !m_symbols;
        refreshLabels();
    });
    bottom->addWidget(m_layer, 3);

    QPushButton *space = makeButton("Space");
    connect(space, &QPushButton::clicked, this, [this]() { sendKey(Qt::Key_Space, " "); });
    bottom->addWidget(space, 12);

    QPushButton *hideKey = makeButton("Hide");
    hideKey->setObjectName("keyboardHide");
    connect(hideKey, &QPushButton::clicked, this, [this]() { closeKeyboard(false); });
    bottom->addWidget(hideKey, 3);

    QPushButton *enter = makeButton("Enter");
    enter->setObjectName("keyboardEnter");
    connect(enter, &QPushButton::clicked, this, [this]() { closeKeyboard(true); });
    bottom->addWidget(enter, 3);
    outer->addLayout(bottom);

    refreshLabels();
}

void TextKeyboard::setKeyHeight(int height)
{
    for (QPushButton *b : m_allButtons)
        b->setMinimumHeight(height);
    setStyleSheet(QString("QPushButton { font-size: %1px; padding: 0px; }").arg(qRound(height * 0.40)));
    adjustSize();
    if (isVisible())
        reposition();
}

// Put the right character on every key for the current layer and shift state
void TextKeyboard::refreshLabels()
{
    const bool upper = m_shiftState != ShiftOff;
    for (const Key &k : m_keys) {
        const char *row = m_symbols ? symbolRows[k.row] : letterRows[k.row];
        const QChar c = QChar::fromLatin1(row[k.column]);
        const bool used = c != QLatin1Char(' ');
        QString label = used ? QString(c) : QString();
        if (used && !m_symbols && c.isLetter() && upper)
            label = label.toUpper();
        k.button->setText(label);
        k.button->setEnabled(used);
        k.button->setVisible(true);
    }
    m_layer->setText(m_symbols ? "ABC" : "?123");
    m_shift->setText(m_shiftState == ShiftLock ? "CAPS" : "Shift");
    m_shift->setEnabled(!m_symbols);
}

void TextKeyboard::typeKey(int index)
{
    const QString text = m_keys.at(index).button->text();
    if (text.isEmpty())
        return;
    const QChar c = text.at(0);
    sendKey(c.toUpper().unicode(), text);
    if (m_shiftState == ShiftOnce && c.isLetter()) {
        m_shiftState = ShiftOff;            // one capital letter, then back to small letters
        refreshLabels();
    }
}

void TextKeyboard::registerEdit(QLineEdit *edit)
{
    edit->setFocusPolicy(Qt::ClickFocus);   // only take focus when tapped
    edit->installEventFilter(this);
    m_edits.insert(edit);
}

bool TextKeyboard::eventFilter(QObject *watched, QEvent *event)
{
    auto *edit = qobject_cast<QLineEdit *>(watched);
    if (edit && m_edits.contains(edit)) {
        if (event->type() == QEvent::FocusIn) {
            showForEdit(edit);
        } else if (event->type() == QEvent::FocusOut) {
            // Wait a moment: if focus moved to another registered edit the keyboard stays;
            // if it went anywhere else (a button, say), hide it.
            QTimer::singleShot(0, this, [this]() { hideIfNoEditFocused(); });
        }
    }
    return QFrame::eventFilter(watched, event);
}

void TextKeyboard::showForEdit(QLineEdit *edit)
{
    m_target = edit;
    m_shiftState = ShiftOff;
    m_symbols = false;
    refreshLabels();
    reposition();
    show();
    raise();
    // Select the old text, so the first key typed replaces it (a Wi-Fi password is retyped, not edited)
    QTimer::singleShot(0, edit, [edit]() { edit->selectAll(); });
}

void TextKeyboard::closeKeyboard(bool pressEnter)
{
    if (m_target) {
        if (pressEnter)
            sendKey(Qt::Key_Return, "\r");  // fires returnPressed() / editingFinished()
        m_target->clearFocus();
    }
    hide();
}

void TextKeyboard::hideIfNoEditFocused()
{
    auto *focus = qobject_cast<QLineEdit *>(QApplication::focusWidget());
    if (!focus || !m_edits.contains(focus))
        hide();
}

void TextKeyboard::reposition()
{
    QWidget *page = parentWidget();
    if (!page)
        return;

    // The visible part of the page: the page can be taller than the window
    QRect visible = page->rect();
    if (QWidget *win = window()) {
        const QPoint winTopLeft = page->mapFromGlobal(win->mapToGlobal(QPoint(0, 0)));
        visible &= QRect(winTopLeft, win->size());
    }

    const int margin = 10;
    setFixedWidth(visible.width() - 2 * margin);
    adjustSize();
    move(visible.x() + margin, qMax(visible.y(), visible.y() + visible.height() - height() - margin));
}

void TextKeyboard::sendKey(int key, const QString &text)
{
    if (!m_target)
        return;
    QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, text);
    QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier, text);
    QApplication::sendEvent(m_target, &press);      // synchronous, so the order is guaranteed
    QApplication::sendEvent(m_target, &release);
}

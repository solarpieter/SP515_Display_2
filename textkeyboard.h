#ifndef TEXTKEYBOARD_H
#define TEXTKEYBOARD_H

#include <QFrame>
#include <QList>
#include <QPointer>
#include <QSet>

class QLineEdit;
class QPushButton;

// On-screen keyboard for typing text such as a Wi-Fi password: letters, digits and symbols, with
// Shift (tap once for one capital, twice for caps lock), backspace, space, Hide and Enter.
// It stays hidden until a registered edit is tapped, and hides again when Enter or Hide is pressed
// (or the user taps outside the edits). It is shown across the bottom of its page.
class TextKeyboard : public QFrame
{
    Q_OBJECT

public:
    explicit TextKeyboard(QWidget *parentPage);

    // Make a line edit open this keyboard when tapped.
    void registerEdit(QLineEdit *edit);

    // Height of each key in pixels (default 60). The text grows with the key height.
    void setKeyHeight(int height);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum ShiftState { ShiftOff, ShiftOnce, ShiftLock };

    void buildKeys();
    void refreshLabels();
    void typeKey(int index);
    void sendKey(int key, const QString &text);
    void showForEdit(QLineEdit *edit);
    void closeKeyboard(bool pressEnter);
    void hideIfNoEditFocused();
    void reposition();

    struct Key
    {
        QPushButton *button = nullptr;
        int row = 0;
        int column = 0;
    };

    QList<Key> m_keys;                      // the character keys, in row order
    QPushButton *m_shift = nullptr;
    QPushButton *m_layer = nullptr;
    QList<QPushButton *> m_allButtons;
    QPointer<QLineEdit> m_target;
    QSet<QLineEdit *> m_edits;
    ShiftState m_shiftState = ShiftOff;
    bool m_symbols = false;
};

#endif // TEXTKEYBOARD_H

#ifndef NUMERICKEYPAD_H
#define NUMERICKEYPAD_H

#include <QFrame>
#include <QList>
#include <QPointer>
#include <QSet>

class QGridLayout;
class QPushButton;
class QWidget;

// On-screen numeric keypad: 0-9, minus, decimal point, backspace and enter.
// It stays hidden until a registered edit is tapped, and hides again
// when Enter is pressed (or the user taps outside the edits).
class NumericKeypad : public QFrame
{
    Q_OBJECT

public:
    // parentPage: the page the keypad is shown on (it is placed at the
    // bottom-right of that page and raised above the other widgets).
    explicit NumericKeypad(QWidget *parentPage);

    // Make an edit open this keypad when tapped. Accepts a QLineEdit or any
    // spin box (QSpinBox / QDoubleSpinBox).
    void registerEdit(QWidget *edit);

    // Size of each key in pixels (default 70 x 60). The text grows with the key height.
    void setKeySize(int width, int height);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void buildButtons();
    void sendKey(int key, const QString &text);
    void showForEdit(QWidget *edit);
    QWidget *registeredFor(QObject *obj) const;
    void hideIfNoEditFocused();
    void reposition();

    QList<QPushButton *> m_buttons;
    QGridLayout *m_grid = nullptr;
    QPointer<QWidget> m_target;
    QSet<QWidget *> m_edits;
};

#endif // NUMERICKEYPAD_H

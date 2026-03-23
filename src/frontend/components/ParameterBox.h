#ifndef PARAMETERBOX_H
#define PARAMETERBOX_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>

class ParameterBox : public QWidget {
    Q_OBJECT
public:
    explicit ParameterBox(QWidget *parent = nullptr);
    void updateForTask(int taskIndex);

signals:
    // This signal tells the controller which "Add" button was clicked
    void quickActionRequested(int taskIndex);

private:
    QGridLayout* layout;
    void clearLayout();
    
    // Helper declarations
    void addSpinBox(const QString& id, const QString& labelText, int min, int max, int defaultVal, int row, int col);
    void addSlider(const QString& id, const QString& labelText, int min, int max, int defaultVal, int row, int col, bool isFloat);
    
    // FIXED: Added this declaration so ParameterBox.cpp can find it
    void addButton(const QString& text, int targetTask, int row, int col);
};

#endif

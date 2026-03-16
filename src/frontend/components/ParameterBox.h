#ifndef PARAMETERBOX_H
#define PARAMETERBOX_H

#include <QWidget>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>

class ParameterBox : public QWidget {
    Q_OBJECT

public:
    explicit ParameterBox(QWidget *parent = nullptr);
    void updateForTask(int taskIndex);

private:
    QGridLayout* layout;
    
    // Helper UI builders to keep the code clean
    void clearLayout();
    void addSpinBox(const QString& id, const QString& label, int min, int max, int defaultVal, int row, int col);
    void addSlider(const QString& id, const QString& label, int min, int max, int defaultVal, int row, int col, bool isFloat = false);
};

#endif // PARAMETERBOX_H
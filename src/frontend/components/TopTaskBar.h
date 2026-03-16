#ifndef TOPTASKBAR_H
#define TOPTASKBAR_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "ParameterBox.h"

class TopTaskBar : public QWidget {
    Q_OBJECT

public:
    explicit TopTaskBar(QWidget *parent = nullptr);

    int getSelectedOperation() const;
    ParameterBox* getParameterBox() { return paramBox; }
    
    void setProcessing(bool isProcessing);
    void setStatus(const QString& message, bool success = true);

signals:
    void taskChanged(int taskIndex);
    void applyRequested();
    void clearRequested();
    void saveRequested();

private slots:
    void onSelectionChanged(int index);

private:
    QComboBox* opSelector;
    ParameterBox* paramBox;
    QPushButton* applyBtn;
    QPushButton* clearBtn;
    QPushButton* saveBtn;
    QLabel* statusLabel;
};

#endif // TOPTASKBAR_H
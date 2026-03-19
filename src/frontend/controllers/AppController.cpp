#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include "../MainWindow.h"
#include "ImageStateManager.h"

class AppController : public QObject {
    Q_OBJECT
public:
    AppController(MainWindow* window, QObject *parent = nullptr);

private slots:
    void handleTaskChange(int taskIndex);
    void handleApply();
    void handleSave();
    void handleClear();

private:
    MainWindow* mainWindow;
    ImageStateManager stateManager;
};

#endif // APPCONTROLLER_H

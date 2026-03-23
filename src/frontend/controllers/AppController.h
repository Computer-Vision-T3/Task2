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
    
    // FIXED: Added "int taskIndexOverride = -1" 
    // This allows it to work for both the main Apply button AND the "Add" buttons
    void handleApply(int taskIndexOverride = -1);
    
    void handleSave();
    void handleClear();

private:
    MainWindow* mainWindow;
    ImageStateManager stateManager;
};

#endif

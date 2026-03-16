cmake_minimum_required(VERSION 3.10)
project(Task2_VisionStudio VERSION 1.0 LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export map for VS Code IntelliSense (Removes red squiggles)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ─── Qt5 Configuration ──────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# ─── OpenCV Configuration ───────────────────────────────────────────
find_package(OpenCV REQUIRED)

# ─── Include Directories ────────────────────────────────────────────
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

# ─── Source Files ───────────────────────────────────────────────────
# Main App
set(MAIN_SOURCE "src/main.cpp")

# Auto-detect all frontend files
file(GLOB_RECURSE FRONTEND_SOURCES 
    "src/frontend/*.cpp" 
    "src/frontend/*.h" 
    "src/frontend/ui/*.ui"
)

# Auto-detect all backend files (Team workspaces)
file(GLOB_RECURSE BACKEND_SOURCES 
    "src/backend/*.cpp" 
    "src/backend/*.h"
)

# ─── Executable & Linking ───────────────────────────────────────────
# We changed the target name here to "launch"
add_executable(launch 
    ${MAIN_SOURCE} 
    ${FRONTEND_SOURCES} 
    ${BACKEND_SOURCES}
)

# And we link the libraries to "launch"
target_link_libraries(launch PRIVATE
    Qt5::Widgets
    ${OpenCV_LIBS}
)
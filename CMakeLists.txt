cmake_minimum_required(VERSION 3.10)
project(Task2_VisionStudio VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ─── Qt5 Configuration ──────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# ─── OpenCV, CURL, TIFF ──────────────────────────────────────────────
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
find_package(TIFF REQUIRED)

# ─── Include Directories ────────────────────────────────────────────
# Explicitly adding Qt5 include dirs helps fix the "No such file" error
include_directories(
    src
    ${Qt5Widgets_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# ─── Source Files ───────────────────────────────────────────────────
file(GLOB_RECURSE FRONTEND_SOURCES "src/frontend/*.cpp" "src/frontend/*.h" "src/frontend/ui/*.ui")
file(GLOB_RECURSE BACKEND_SOURCES "src/backend/*.cpp" "src/backend/*.h")

# ─── Executable & Linking ───────────────────────────────────────────
add_executable(launch src/main.cpp ${FRONTEND_SOURCES} ${BACKEND_SOURCES})

target_link_libraries(launch PRIVATE
    Qt5::Widgets
    ${OpenCV_LIBS}
    ${CURL_LIBRARIES}
    ${TIFF_LIBRARIES}
    -lcurl
)

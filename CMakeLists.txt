cmake_minimum_required(VERSION 3.31)
project(SeatShuffler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "D:/Qt/6.9.1/mingw_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

set(APP_RC "")
if (WIN32)
    set(APP_RC app.rc)
endif ()

add_executable(SeatShuffler
        main.cpp
        resources.qrc
        ${APP_RC}
        src/storage/DataManager.cpp
        src/storage/DataManager.h
        src/ui/page/MainWindow.cpp
        src/ui/page/MainWindow.h
        src/ui/common/Common.h
        src/models/Classroom.h
        src/models/PersonGroup.h
        src/models/ShuffleRecord.h
        src/models/Rule.h
        src/ui/widgets/ClassroomGridWidget.cpp
        src/ui/widgets/ClassroomGridWidget.h
        src/ui/widgets/ResultGridWidget.cpp
        src/ui/widgets/ResultGridWidget.h
        src/ui/page/ClassroomEditorPage.cpp
        src/ui/page/ClassroomEditorPage.h
        src/ui/page/ClassroomListPage.cpp
        src/ui/page/ClassroomListPage.h
        src/ui/page/HistoryDetailPage.cpp
        src/ui/page/HistoryDetailPage.h
        src/ui/page/HistoryPage.cpp
        src/ui/page/HistoryPage.h
        src/ui/page/PersonGroupDetailPage.cpp
        src/ui/page/PersonGroupDetailPage.h
        src/ui/page/PersonGroupListPage.cpp
        src/ui/page/PersonGroupListPage.h
        src/ui/page/RuleEditorPage.cpp
        src/ui/page/RuleEditorPage.h
        src/ui/page/RuleListPage.cpp
        src/ui/page/RuleListPage.h
        src/ui/page/ShufflePage.cpp
        src/ui/page/ShufflePage.h
        src/engine/ShuffleEngine.cpp
        src/engine/ShuffleEngine.h
        src/ui/common/Appstyle.h
        src/ui/widgets/SeatRuleGridWidget.cpp
        src/ui/widgets/SeatRuleGridWidget.h
)
target_link_libraries(SeatShuffler
        Qt::Core
        Qt::Gui
        Qt::Widgets
)
target_include_directories(
        SeatShuffler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

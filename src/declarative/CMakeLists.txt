add_definitions(-DTRANSLATION_DOMAIN="timekeeper")

set(plasmatimekeeper_qmlplugins_SRCS
   activitymodel.cpp
   activitysortmodel.cpp
   qmlplugins.cpp
)

add_library(plasmatimekeeper_qmlplugins SHARED ${plasmatimekeeper_qmlplugins_SRCS})

target_link_libraries(plasmatimekeeper_qmlplugins
    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    Qt5::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WindowSystem
)

install(TARGETS plasmatimekeeper_qmlplugins DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/timekeeper)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/timekeeper)

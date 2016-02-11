add_definitions(-DTRANSLATION_DOMAIN="activitymonitor")

set(plasmaactivitymonitor_qmlplugins_SRCS
   activitymodel.cpp
   activitysortmodel.cpp
   qmlplugins.cpp
)

add_library(plasmaactivitymonitor_qmlplugins SHARED ${plasmaactivitymonitor_qmlplugins_SRCS})

target_link_libraries(plasmaactivitymonitor_qmlplugins
    Qt5::Core
    Qt5::Qml
    Qt5::Widgets
    KF5::I18n
    KF5::CoreAddons
    KF5::WindowSystem
)

install(TARGETS plasmaactivitymonitor_qmlplugins DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/activitymonitor)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/activitymonitor)

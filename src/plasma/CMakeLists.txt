configure_file(${CMAKE_CURRENT_SOURCE_DIR}/metadata.desktop.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/metadata.desktop
)

plasma_install_package(. org.kde.plasma.activitymonitor)

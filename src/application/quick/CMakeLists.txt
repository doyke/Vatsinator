find_package (Qt5Quick 5.7 REQUIRED) # 5.7 for QtQuick.Controls.2
include_directories (${Qt5Quick_INCLUDE_DIRS})

find_package (Qt5Xml REQUIRED)

set (vatsinator_QUICK_QMLS
    AirportDetailsPage.qml
    AtcDetailsPage.qml
    FirDetailsPage.qml
    FlightDetailsPage.qml
    main.qml
)

add_library (vatsinator SHARED main.cpp qml.qrc ${vatsinator_QUICK_QMLS})
target_link_libraries (vatsinator
    Qt5::Quick
    Qt5::Xml
    vtrgui
)
add_dependencies (vatsinator i18n)
install (TARGETS vatsinator DESTINATION libs/${ANDROID_ABI})

include (AndroidApkUtils)

file (READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION_CODE ANDROID_VERSION_CODE)
string (REPLACE "\n" "" ANDROID_VERSION_CODE ${ANDROID_VERSION_CODE})

set_target_properties (vatsinator PROPERTIES
    ANDROID_APP_NAME "Vatsinator"
    ANDROID_VERSION_CODE ${ANDROID_VERSION_CODE}
    ANDROID_VERSION_NAME ${vatsinator_VERSION}
    ANDROID_PACKAGE_NAME "org.eu.vatsinator.vatsinator"
)

android_generate_manifest(vatsinator
    ANDROID_PERMISSIONS
        ACCESS_NETWORK_STATE
        INTERNET
    TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/android/AndroidManifest.xml.in
    ANDROID_ACTIVITY_CLASS "org.eu.vatsinator.VatsinatorActivity"
)

android_deploy_apk(vatsinator
    RESOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/android/res/drawable
        ${CMAKE_CURRENT_SOURCE_DIR}/android/res/drawable-ldpi
        ${CMAKE_CURRENT_SOURCE_DIR}/android/res/drawable-mdpi
        ${CMAKE_CURRENT_SOURCE_DIR}/android/res/drawable-hdpi
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/android/java/org
    QT_QML_MODULES
        QtGraphicalEffects
        QtQuick.2
        QtQuick/Controls.2
        QtQuick/Controls.2/Material
        QtQuick/Templates.2
        QtQuick/Layouts
        QtQuick/Window.2
    QML_PLUGINS
        componentsplugin
        nativeutilsplugin
        vatsimplugin
    QT_PLUGINS
        imageformats/libqsvg.so
    PLUGINS
        euroutenotamprovider
        tiledmapdrawer
)
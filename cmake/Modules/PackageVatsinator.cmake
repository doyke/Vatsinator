set (CPACK_PACKAGE_NAME "Vatsinator")
set (CPACK_PACKAGE_VENDOR "Vatsinator Team")
set (CPACK_PACKAGE_CONTACT "Michał Garapich michal@garapich.pl")
set (CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION ${vatsinator_VERSION})
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vatsim monitor")
set (CPACK_PACKAGE_EXECUTABLES "vatsinator;Vatsinator")

if (WIN32)
    set (CPACK_GENERATOR "NSIS")
    
    set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}Setup-${vatsinator_VERSION}")
    set (CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
    
    set (CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
    set (CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME}")
    set (CPACK_NSIS_HELP_LINK "http://vatsinator.eu.org/")
    set (CPACK_NSIS_URL_INFO_ABOUT "http://vatsinator.eu.org/")
    set (CPACK_NSIS_CONTACT "michal@garapich.pl")
    set (CPACK_NSIS_MUI_FINISHPAGE_RUN "vatsinator.exe")
    set (CPACK_NSIS_MENU_LINKS "http://vatsinator.eu.org/" "${CPACK_PACKAGE_NAME} Homepage")
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        CreateShortCut \\\"$DESKTOP\\\\Vatsinator.lnk\\\" \\\"$INSTDIR\\\\vatsinator.exe\\\"
    ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        Delete \\\"$DESKTOP\\\\Vatsinator.lnk\\\"
    ")
elseif (APPLE)
    set (CPACK_GENERATOR "DragNDrop")

    set (CPACK_DMG_FORMAT "UDZO")
    set (CPACK_DMG_VOLUME_NAME "${CPACK_PACKAGE_NAME}")
    set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${vatsinator_VERSION}")
endif ()

include (CPack)

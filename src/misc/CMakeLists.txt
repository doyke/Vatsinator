find_package (Qt5 5.7 REQUIRED COMPONENTS Core Network Gui)

set (misc_SOURCES
    airlinelogoreply.cpp
    atcbooking.cpp
    atcbookinglistmodel.cpp
    atcbookingreply.cpp
    atclistmodel.cpp
    clientlistmodel.cpp
    filecache.cpp
    flightlistmodel.cpp
    metarlistmodel.cpp
    notam.cpp
    notamlistmodel.cpp
    notamreply.cpp
)

set (misc_HEADERS
    airlinelogoprovider.h
    airlinelogoreply.h
    atcbooking.h
    atcbookinglistmodel.h
    atcbookingprovider.h
    atcbookingreply.h
    atclistmodel.h
    clientlistmodel.h
    filecache.h
    flightlistmodel.h
    metarlistmodel.h
    notam.h
    notamlistmodel.h
    notamprovider.h
    notamreply.h
    
    misc.h
    roles.h
)

add_library (vtrmisc SHARED ${misc_SOURCES} ${misc_HEADERS})
target_link_libraries (vtrmisc vtrcore Qt5::Gui)
set_target_properties (vtrmisc PROPERTIES CXX_VISIBILITY_PRESET hidden)

include (GenerateExportHeader)
generate_export_header(vtrmisc)

install (TARGETS vtrmisc
    EXPORT vtrmisc
    RUNTIME DESTINATION ${vatsinator_BINARY_TARGET_DIR}
    LIBRARY DESTINATION ${vatsinator_LIBRARY_TARGET_DIR}
    ARCHIVE DESTINATION ${vatsinator_ARCHIVE_TARGET_DIR} CONFIGURATIONS Debug
)

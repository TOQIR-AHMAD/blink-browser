add_library(pb_utils STATIC
    logging.cpp
    logging.h
)

target_include_directories(pb_utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

if(PB_DEV_LOGGING)
    target_compile_definitions(pb_utils PUBLIC PB_DEV_LOGGING)
endif()

add_library(PrivacyBrowser::Utils ALIAS pb_utils)

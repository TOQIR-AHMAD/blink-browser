# Core libraries are Qt-free so they can be built and tested without Qt.
add_subdirectory(utils)

# Subsystems added in later phases:
#   browser/     Phase 4   browser + window + navigation controllers
#   chromium/    Phase 1   Qt WebEngine integration layer
#   tabs/        Phase 4   tab model and tab manager
#   privacy/     Phase 5   PrivacyManager, cookies, storage, cache, cleanup
#   network/     Phase 6   request interception, filter lists
#   permissions/ Phase 4   permission manager (default: ask)
#   downloads/   Phase 4   download manager
#   settings/    Phase 9   settings model

if(PB_BUILD_UI)
    add_subdirectory(app)
endif()

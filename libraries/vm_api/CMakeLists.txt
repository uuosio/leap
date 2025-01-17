add_library( vm_api SHARED
    vm_api.cpp
    action.cpp
    chain.cpp
    crypto.cpp
    db.cpp
    permission.cpp
    print.cpp
    privileged.cpp
    system.cpp
    transaction.cpp
)

if (APPLE)
else()
    target_compile_options(vm_api PRIVATE -fPIC -D__NATIVE)
endif()

if (APPLE)
      set(_symbols_list "${CMAKE_CURRENT_SOURCE_DIR}/symbols.list")
      set(vm_api_LINK_FLAGS "${LINK_FLAGS} -Wl,-exported_symbols_list,'${_symbols_list}'")
else()
      set(_version_script "${CMAKE_CURRENT_SOURCE_DIR}/version.script")
      set(vm_api_LINK_FLAGS "${LINK_FLAGS} -Wl,--version-script,\"${_version_script}\"")
endif (APPLE)

set_target_properties(vm_api  PROPERTIES LINK_FLAGS "${vm_api_LINK_FLAGS}")

target_include_directories(vm_api PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libraries/chain/vm_api
    ${CMAKE_SOURCE_DIR}/libraries/chain/chain_api
)

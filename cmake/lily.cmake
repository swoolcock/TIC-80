################################
# LILY
################################

option(BUILD_WITH_LILY "Lily Enabled" ${BUILD_WITH_ALL})
message("BUILD_WITH_LILY: ${BUILD_WITH_LILY}")

#if(BUILD_WITH_LILY AND PREFER_SYSTEM_LIBRARIES)
#    find_path(squirrel_INLCUDE_DIR NAMES squirrel.h PATH_SUFFIXES squirrel)
#    find_library(squirrel_LIBRARY NAMES squirrel)
#    find_library(sqstdlib_LIBRARY NAMES sqstdlib)
#    if(squirrel_INLCUDE_DIR AND squirrel_LIBRARY AND sqstdlib_LIBRARY)
#        add_library(squirrel STATIC
#            ${CMAKE_SOURCE_DIR}/src/api/squirrel.c
#            ${CMAKE_SOURCE_DIR}/src/api/parse_note.c
#        )
#        target_compile_definitions(squirrel INTERFACE TIC_BUILD_WITH_LILY)
#        target_link_libraries(squirrel PRIVATE runtime ${squirrel_LIBRARY} ${sqstdlib_LIBRARY})
#        target_include_directories(squirrel
#            PUBLIC ${squirrel_INLCUDE_DIR}
#            PRIVATE
#                ${CMAKE_SOURCE_DIR}/include
#                ${CMAKE_SOURCE_DIR}/src
#        )
#        message(STATUS "Use system library: squirrel")
#        return()
#    else()
#        message(WARNING "System library squirrel not found")
#    endif()
#endif()


if(BUILD_WITH_LILY)

    set(LILY_DIR ${THIRDPARTY_DIR}/lily)
    set(LILY_SRC
        ${LILY_DIR}/src/csiphash.c
        ${LILY_DIR}/src/lily_alloc.c
        ${LILY_DIR}/src/lily_api.c
        ${LILY_DIR}/src/lily_buffer_u16.c
        ${LILY_DIR}/src/lily_build_error.c
        ${LILY_DIR}/src/lily_closure.c
        ${LILY_DIR}/src/lily_code_iter.c
        ${LILY_DIR}/src/lily_emitter.c
        ${LILY_DIR}/src/lily_expr.c
        ${LILY_DIR}/src/lily_generic_pool.c
        ${LILY_DIR}/src/lily_import.c
        ${LILY_DIR}/src/lily_lexer.c
        ${LILY_DIR}/src/lily_library.c
        ${LILY_DIR}/src/lily_msgbuf.c
        ${LILY_DIR}/src/lily_parser.c
        ${LILY_DIR}/src/lily_pkg_core.c
        ${LILY_DIR}/src/lily_pkg_coroutine.c
        ${LILY_DIR}/src/lily_pkg_fs.c
        ${LILY_DIR}/src/lily_pkg_introspect.c
        ${LILY_DIR}/src/lily_pkg_math.c
        ${LILY_DIR}/src/lily_pkg_prelude.c
        ${LILY_DIR}/src/lily_pkg_random.c
        ${LILY_DIR}/src/lily_pkg_subprocess.c
        ${LILY_DIR}/src/lily_pkg_sys.c
        ${LILY_DIR}/src/lily_pkg_time.c
        ${LILY_DIR}/src/lily_pkg_utf8.c
        ${LILY_DIR}/src/lily_raiser.c
        ${LILY_DIR}/src/lily_string_pile.c
        ${LILY_DIR}/src/lily_symtab.c
        ${LILY_DIR}/src/lily_type_maker.c
        ${LILY_DIR}/src/lily_type_system.c
        ${LILY_DIR}/src/lily_utf8.c
        ${LILY_DIR}/src/lily_virt.c
        ${LILY_DIR}/src/lily_vm.c
        ${LILY_DIR}/src/st.c
    )

    list(APPEND LILY_SRC ${CMAKE_SOURCE_DIR}/src/api/lily.c)
#    list(APPEND LILY_SRC ${CMAKE_SOURCE_DIR}/src/api/lily_pkg_tic80.c)
    list(APPEND LILY_SRC ${CMAKE_SOURCE_DIR}/src/api/parse_note.c)

    add_library(lily ${TIC_RUNTIME} ${LILY_SRC})
    target_compile_options(lily PRIVATE -g -O0) # TODO: remove

    if(NOT BUILD_STATIC)
        set_target_properties(lily PROPERTIES PREFIX "")
    else()
        target_compile_definitions(lily INTERFACE TIC_BUILD_WITH_LILY=1)
    endif()

    target_link_libraries(lily PRIVATE runtime)

#    set_target_properties(lily PROPERTIES LINKER_LANGUAGE CXX)

    target_include_directories(lily
        PUBLIC ${LILY_DIR}/src
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
    )


endif()

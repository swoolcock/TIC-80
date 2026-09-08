################################
# GRAVITY
################################

option(BUILD_WITH_GRAVITY "Gravity Enabled" ${BUILD_WITH_ALL})
message("BUILD_WITH_GRAVITY: ${BUILD_WITH_GRAVITY}")

#if(BUILD_WITH_SQUIRREL AND PREFER_SYSTEM_LIBRARIES)
#    find_path(squirrel_INLCUDE_DIR NAMES squirrel.h PATH_SUFFIXES squirrel)
#    find_library(squirrel_LIBRARY NAMES squirrel)
#    find_library(sqstdlib_LIBRARY NAMES sqstdlib)
#    if(squirrel_INLCUDE_DIR AND squirrel_LIBRARY AND sqstdlib_LIBRARY)
#        add_library(squirrel STATIC
#            ${CMAKE_SOURCE_DIR}/src/api/squirrel.c
#            ${CMAKE_SOURCE_DIR}/src/api/parse_note.c
#        )
#        target_compile_definitions(squirrel INTERFACE TIC_BUILD_WITH_SQUIRREL)
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


if(BUILD_WITH_GRAVITY)

    set(GRAVITY_DIR ${THIRDPARTY_DIR}/gravity/src)
    set(GRAVITY_SRC
        ${GRAVITY_DIR}/compiler/gravity_ast.c
        ${GRAVITY_DIR}/compiler/gravity_codegen.c
        ${GRAVITY_DIR}/compiler/gravity_compiler.c
        ${GRAVITY_DIR}/compiler/gravity_ircode.c
        ${GRAVITY_DIR}/compiler/gravity_lexer.c
        ${GRAVITY_DIR}/compiler/gravity_optimizer.c
        ${GRAVITY_DIR}/compiler/gravity_parser.c
        ${GRAVITY_DIR}/compiler/gravity_semacheck1.c
        ${GRAVITY_DIR}/compiler/gravity_semacheck2.c
        ${GRAVITY_DIR}/compiler/gravity_symboltable.c
        ${GRAVITY_DIR}/compiler/gravity_token.c
        ${GRAVITY_DIR}/compiler/gravity_visitor.c
        ${GRAVITY_DIR}/optionals/gravity_opt_env.c
        ${GRAVITY_DIR}/optionals/gravity_opt_file.c
        ${GRAVITY_DIR}/optionals/gravity_opt_json.c
        ${GRAVITY_DIR}/optionals/gravity_opt_math.c
        ${GRAVITY_DIR}/runtime/gravity_core.c
        ${GRAVITY_DIR}/runtime/gravity_vm.c
        ${GRAVITY_DIR}/shared/gravity_hash.c
        ${GRAVITY_DIR}/shared/gravity_memory.c
        ${GRAVITY_DIR}/shared/gravity_value.c
        ${GRAVITY_DIR}/utils/gravity_debug.c
        ${GRAVITY_DIR}/utils/gravity_json.c
        ${GRAVITY_DIR}/utils/gravity_utils.c
    )

    list(APPEND GRAVITY_SRC ${CMAKE_SOURCE_DIR}/src/api/gravity.c)
    list(APPEND GRAVITY_SRC ${CMAKE_SOURCE_DIR}/src/api/parse_note.c)

    add_library(gravity ${TIC_RUNTIME} ${GRAVITY_SRC})
    target_compile_options(gravity PRIVATE -g -O0) # TODO: remove

    if(NOT BUILD_STATIC)
        set_target_properties(gravity PROPERTIES PREFIX "")
    else()
        target_compile_definitions(gravity INTERFACE TIC_BUILD_WITH_GRAVITY=1)
    endif()

    target_link_libraries(gravity PRIVATE
            runtime
            shlwapi
    )

    set_target_properties(gravity PROPERTIES LINKER_LANGUAGE CXX)

    target_include_directories(gravity
        PUBLIC
            ${GRAVITY_DIR}/compiler
            ${GRAVITY_DIR}/optionals
            ${GRAVITY_DIR}/runtime
            ${GRAVITY_DIR}/shared
            ${GRAVITY_DIR}/utils
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
    )


endif()

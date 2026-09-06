################################
# ANGELSCRIPT
################################

option(BUILD_WITH_ANGELSCRIPT "AngelScript Enabled" ${BUILD_WITH_ALL})
message("BUILD_WITH_ANGELSCRIPT: ${BUILD_WITH_ANGELSCRIPT}")

#if(BUILD_WITH_ANGELSCRIPT AND PREFER_SYSTEM_LIBRARIES)
#    find_path(angelscript_INCLUDE_DIR NAMES squirrel.h PATH_SUFFIXES squirrel)
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
#            PUBLIC ${angelscript_INCLUDE_DIR}
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


if(BUILD_WITH_ANGELSCRIPT)

    # TODO: platform specific callfunc
    set(ANGELSCRIPT_DIR ${THIRDPARTY_DIR}/angelscript/sdk)
    set(ANGELSCRIPT_SRC
        ${ANGELSCRIPT_DIR}/angelscript/source/as_atomic.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_builder.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_bytecode.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_callfunc.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_callfunc_mips.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_callfunc_x86.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_callfunc_x64_gcc.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_callfunc_x64_msvc.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_callfunc_x64_mingw.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_compiler.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_configgroup.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_context.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_datatype.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_gc.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_generic.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_globalproperty.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_memory.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_module.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_objecttype.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_outputbuffer.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_parser.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_restore.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_scriptcode.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_scriptengine.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_scriptfunction.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_scriptnode.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_scriptobject.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_string.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_string_util.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_thread.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_tokenizer.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_typeinfo.cpp
        ${ANGELSCRIPT_DIR}/angelscript/source/as_variablescope.cpp
#        ${ANGELSCRIPT_DIR}/angelscript/source/angelscript_tic80.cpp
    )

#    list(APPEND ANGELSCRIPT_SRC ${CMAKE_SOURCE_DIR}/src/api/angelscript_wrapper.cpp)
    list(APPEND ANGELSCRIPT_SRC ${CMAKE_SOURCE_DIR}/src/api/angelscript.cpp)
    list(APPEND ANGELSCRIPT_SRC ${CMAKE_SOURCE_DIR}/src/api/parse_note.c)

    add_library(angelscript ${TIC_RUNTIME} ${ANGELSCRIPT_SRC})

    if(NOT BUILD_STATIC)
        set_target_properties(angelscript PROPERTIES PREFIX "")
    else()
        target_compile_definitions(angelscript INTERFACE TIC_BUILD_WITH_ANGELSCRIPT=1)
    endif()

    target_link_libraries(angelscript PRIVATE runtime)

    set_target_properties(angelscript PROPERTIES LINKER_LANGUAGE CXX)

    target_include_directories(angelscript
            PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
    )

    target_include_directories(angelscript PUBLIC ${ANGELSCRIPT_DIR}/angelscript/include)
    target_include_directories(angelscript PRIVATE ${ANGELSCRIPT_DIR}/angelscript/source)

#    target_include_directories(angelscript PUBLIC ${ANGELSCRIPT_DIR}/tic80/include)

endif()

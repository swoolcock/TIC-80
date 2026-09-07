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

    set(ANGELSCRIPT_DIR ${THIRDPARTY_DIR}/angelscript/sdk)

    file(READ ${ANGELSCRIPT_DIR}/angelscript/include/angelscript.h ANGELSCRIPT_H)
    string(REGEX MATCH "#define ANGELSCRIPT_VERSION_STRING \"([0-9]*).([0-9]*).([0-9]*)" ANGELSCRIPT_VERSION_REGEX ${ANGELSCRIPT_H})
    set(ANGELSCRIPT_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(ANGELSCRIPT_VERSION_MINOR ${CMAKE_MATCH_2})
    set(ANGELSCRIPT_VERSION_PATCH ${CMAKE_MATCH_3})

    set(AS_MAX_PORTABILITY, TRUE)

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
        ${ANGELSCRIPT_DIR}/add_on/scriptany/scriptany.cpp
        ${ANGELSCRIPT_DIR}/add_on/scriptarray/scriptarray.cpp
        ${ANGELSCRIPT_DIR}/add_on/scriptdictionary/scriptdictionary.cpp
#        ${ANGELSCRIPT_DIR}/add_on/scriptgrid/scriptgrid.cpp
        ${ANGELSCRIPT_DIR}/add_on/scripthandle/scripthandle.cpp
        ${ANGELSCRIPT_DIR}/add_on/scripthelper/scripthelper.cpp
        ${ANGELSCRIPT_DIR}/add_on/scriptmath/scriptmath.cpp
#        ${ANGELSCRIPT_DIR}/add_on/scriptmath/scriptmathcomplex.cpp
        ${ANGELSCRIPT_DIR}/add_on/scriptstdstring/scriptstdstring.cpp
        ${ANGELSCRIPT_DIR}/add_on/scriptstdstring/scriptstdstring_utils.cpp
        ${ANGELSCRIPT_DIR}/add_on/weakref/weakref.cpp
    )

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

    target_include_directories(angelscript PRIVATE ${ANGELSCRIPT_DIR}/angelscript/source)

    target_include_directories(angelscript
            PUBLIC
            ${ANGELSCRIPT_DIR}/angelscript/include
            ${ANGELSCRIPT_DIR}/add_on/scriptany
            ${ANGELSCRIPT_DIR}/add_on/scriptarray
            ${ANGELSCRIPT_DIR}/add_on/scriptdictionary
#            ${ANGELSCRIPT_DIR}/add_on/scriptgrid
            ${ANGELSCRIPT_DIR}/add_on/scripthandle
            ${ANGELSCRIPT_DIR}/add_on/scripthelper
            ${ANGELSCRIPT_DIR}/add_on/scriptmath
            ${ANGELSCRIPT_DIR}/add_on/scriptstdstring
            ${ANGELSCRIPT_DIR}/add_on/weakref
    )

endif()

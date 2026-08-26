function(mehlissa_configure_target target_name)
    target_compile_features(${target_name} PUBLIC cxx_std_20)
    set_target_properties(${target_name} PROPERTIES CXX_EXTENSIONS OFF)

    if(MSVC)
        target_compile_options(
            ${target_name}
            PRIVATE /W4 /permissive- /Zc:__cplusplus
        )
        if(MEHLISSA_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    else()
        target_compile_options(
            ${target_name}
            PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
        )
        if(MEHLISSA_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()

    if(MEHLISSA_ENABLE_SANITIZERS)
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /fsanitize=address)
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(
                ${target_name}
                PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer
            )
            target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
        else()
            message(FATAL_ERROR "Sanitizers are not configured for this compiler")
        endif()
    endif()

    if(MEHLISSA_ENABLE_CLANG_TIDY)
        find_program(MEHLISSA_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
        set_target_properties(
            ${target_name}
            PROPERTIES CXX_CLANG_TIDY "${MEHLISSA_CLANG_TIDY_EXECUTABLE}"
        )
    endif()
endfunction()

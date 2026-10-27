# Applies the project's warning policy to first-party targets only.
# Third-party targets fetched via FetchContent keep their own flags.
function(wind_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

#Extra libraries that are not essential... I might move this so its project specific. Just experimenting with the idea for now

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.13.0
)
message("Fetching googletest")
FetchContent_MakeAvailable(googletest)

# ##############################################################################
# Ruby
# ##############################################################################
include(FetchContent)

FetchContent_Declare(switch_ruby
    GIT_REPOSITORY git@github.com:maxmarsc/switch-ruby
    GIT_TAG        master
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(switch_ruby)
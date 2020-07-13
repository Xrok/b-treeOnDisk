cmake_minimum_required(VERSION 3.15)
project(b_tree_v2_0)

include(ExternalProject)
ExternalProject_Add(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG master
        SOURCE_DIR "/home/sergio/Documents/paralela/b-tree-on-disk/b+tree/cmake-build-debug/googletest-src"
        BINARY_DIR "/home/sergio/Documents/paralela/b-tree-on-disk/b+tree/cmake-build-debug/googletest-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
        )

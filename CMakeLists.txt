cmake_minimum_required(VERSION 3.16)
project(mmm_memory)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SOURCES
        AttrChangePopup.cpp
        AttrChangePopup.h
        colors.h
        CopyMovePopup.cpp
        CopyMovePopup.h
        DiskPopup.cpp
        DiskPopup.h
        EditableText.cpp
        EditableText.h
        FilePanel.cpp
        FilePanel.h
        LineEdit.cpp
        LineEdit.h
        Lines.cpp
        Lines.h
        main.cpp
        MakeDirPopup.cpp
        MakeDirPopup.h
        MemoryViewPanel.cpp
        MemoryViewPanel.h
        MessagePopup.cpp
        MessagePopup.h
        Popup.cpp
        Popup.h
        RemoveDirPopup.cpp
        RemoveDirPopup.h
        Screen.cpp
        Screen.h
        MakeFilePopup.cpp
        MakeFilePopup.h
        utils.cpp
        utils.h
)

add_executable(mmm_memory ${SOURCES})

cmake_minimum_required(VERSION 3.16)
project(mmm_memory)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SOURCES
        colors.h
        EditableText.cpp
        EditableText.h
        FPWBasePanel.cpp
        FPWBasePanel.h
        LeftPanel.cpp
        LeftPanel.h
        LegendPanel.cpp
        LegendPanel.h
        LineEdit.cpp
        LineEdit.h
        Lines.cpp
        Lines.h
        main.cpp
        MemAllocPanel.cpp
        MemAllocPanel.h
        MemFreePanel.cpp
        MemFreePanel.h
        MemoryRegionsList.cpp
        MemoryRegionsList.h
        MemoryViewPanel.cpp
        MemoryViewPanel.h
        MemProtectPanel.cpp
        MemProtectPanel.h
        MemWritePanel.cpp
        MemWritePanel.h
        MessagePopup.cpp
        MessagePopup.h
        Popup.cpp
        Popup.h
        regions.cpp
        regions.h
        Screen.cpp
        Screen.h
        SystemInfoPanel.cpp
        SystemInfoPanel.h
        utils.cpp
        utils.h
)

add_executable(mmm_memory ${SOURCES})

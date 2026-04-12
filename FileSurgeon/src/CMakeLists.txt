# В src/CMakeLists.txt
qt_add_executable(appFileSurgeon
    main.cpp
    pemodel.h
    pemodel.cpp
    fileprocessor.h
    fileprocessor.cpp
)

target_link_libraries(appFileSurgeon PRIVATE Qt6::Quick Qt6::QuickControls2)

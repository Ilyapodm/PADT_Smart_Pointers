add_executable(smart_pointers_app main.cpp)

target_link_libraries(smart_pointers_app
    PRIVATE
        PADT::smart_pointers
)

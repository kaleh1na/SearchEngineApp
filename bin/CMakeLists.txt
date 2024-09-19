add_executable(index_launcher index_launcher.cpp)
add_executable(search_launcher search_launcher.cpp)

target_link_libraries(index_launcher
  PUBLIC search
  PUBLIC index
)

target_link_libraries(search_launcher
  PUBLIC search
  PUBLIC index
)

target_include_directories(index_launcher PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(search_launcher PUBLIC ${PROJECT_SOURCE_DIR})
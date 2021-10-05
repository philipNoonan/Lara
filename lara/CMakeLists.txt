cmake_minimum_required(VERSION 3.14)
project(lara2i)

if(CMAKE_COMPILER_IS_GNUCXX)
  list( APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -pthread -DBOOST_ERROR_CODE_HEADER_ONLY")
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(websocketpp REQUIRED)
find_package(k4a REQUIRED)

find_package(Boost REQUIRED)
find_package(PortAudio REQUIRED)

file(GLOB_RECURSE SRC "src/*")
file(GLOB_RECURSE SHD "../shaders/*")
file(GLOB_RECURSE HDR "include/*")
file(GLOB_RECURSE MOD "../models/*")

list(APPEND SRC ${SHD})
list(APPEND SRC ${GFX})
#list(APPEND SRC ${HDR})

### COPY SHADER FILES TO BUILD AND BINARY LOCATIONS IF THEY HAVE BEEN CHANGED ###
add_custom_target(
  shaders ALL
  DEPENDS ${SHD}
  COMMENT "Copying shader files"
  VERBATIM
)
foreach(ShaderFile ${SHD})
get_filename_component(ShaderName ${ShaderFile} NAME)
	add_custom_command(
		TARGET shaders
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ShaderFile}
			${PROJECT_BINARY_DIR}/bin/$(Configuration)/shaders/${ShaderName}
	)
		add_custom_command(
		TARGET shaders
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ShaderFile}
			${PROJECT_BINARY_DIR}/shaders/${ShaderName}
	)
endforeach()

message(STATUS "boooosting")

message(STATUS ${Boost_LIBRARY_DIRS})


### COPY MODEL FILES TO BUILD AND BINARY LOCATIONS IF THEY HAVE BEEN CHANGED ###
add_custom_target(
  models ALL
  DEPENDS ${MOD}
  COMMENT "Copying models files"
  VERBATIM
)
foreach(ModelFile ${MOD})
get_filename_component(ModelName ${ModelFile} NAME)
	add_custom_command(
		TARGET models
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ModelFile}
			${PROJECT_BINARY_DIR}/bin/$(Configuration)/models/${ModelName}
	)
		add_custom_command(
		TARGET models
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ModelFile}
			${PROJECT_BINARY_DIR}/models/${ModelName}
	)
endforeach()

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} 
					PRIVATE
						${SRC}
					PUBLIC
						${HDR}
			  )

add_dependencies(${PROJECT_NAME} shaders models)


target_include_directories(${PROJECT_NAME} PUBLIC "include")


target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} k4a::k4a)
target_link_libraries(${PROJECT_NAME} portaudio)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
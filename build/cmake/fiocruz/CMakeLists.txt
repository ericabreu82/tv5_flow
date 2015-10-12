if(WIN32)
  add_definitions(-DFIOCRUZDLL)
endif()

set(FIOCRUZ_ABSOLUTE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

include_directories(
  ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src
  ${terralib_INCLUDE_DIRS}
  ${terralib_DIR}
  ${Boost_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}/qwt
)

file(GLOB FIOCRUZ_SRC_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/*.cpp)
file(GLOB FIOCRUZ_HDR_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/*.h)

source_group("Source Files"  FILES ${FIOCRUZ_SRC_FILES})
source_group("Header Files"  FILES ${FIOCRUZ_HDR_FILES})

#regionalization
file(GLOB FIOCRUZ_REG_SRC_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/regionalization/*.cpp)
file(GLOB FIOCRUZ_REG_HDR_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/regionalization/*.h)

source_group("Source Files\\regionalization"  FILES ${FIOCRUZ_REG_SRC_FILES})
source_group("Header Files\\regionalization"  FILES ${FIOCRUZ_REG_HDR_FILES})

file(GLOB FIOCRUZ_REG_CORE_SRC_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/regionalization/core/*.cpp)
file(GLOB FIOCRUZ_REG_CORE_HDR_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/regionalization/core/*.h)

source_group("Source Files\\regionalization\\core"  FILES ${FIOCRUZ_REG_CORE_SRC_FILES})
source_group("Header Files\\regionalization\\core"  FILES ${FIOCRUZ_REG_CORE_HDR_FILES})

file(GLOB FIOCRUZ_REG_QT_SRC_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/regionalization/qt/*.cpp)
file(GLOB FIOCRUZ_REG_QT_HDR_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/regionalization/qt/*.h)

source_group("Source Files\\regionalization\\qt"  FILES ${FIOCRUZ_REG_QT_SRC_FILES})
source_group("Header Files\\regionalization\\qt"  FILES ${FIOCRUZ_REG_QT_HDR_FILES})

file(GLOB FIOCRUZ_REG_UI_FILES  ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/regionalization/qt/ui/*.ui)

source_group("UI Files\\regionalization"  FILES ${FIOCRUZ_REG_UI_FILES})

#flow
file(GLOB FIOCRUZ_FLOW_SRC_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/flow/*.cpp)
file(GLOB FIOCRUZ_FLOW_HDR_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/flow/*.h)

source_group("Source Files\\flow"  FILES ${FIOCRUZ_FLOW_SRC_FILES})
source_group("Header Files\\flow"  FILES ${FIOCRUZ_FLOW_HDR_FILES})

file(GLOB FIOCRUZ_FLOW_CORE_SRC_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/flow/core/*.cpp)
file(GLOB FIOCRUZ_FLOW_CORE_HDR_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/flow/core/*.h)

source_group("Source Files\\flow\\core"  FILES ${FIOCRUZ_FLOW_CORE_SRC_FILES})
source_group("Header Files\\flow\\core"  FILES ${FIOCRUZ_FLOW_CORE_HDR_FILES})

file(GLOB FIOCRUZ_FLOW_QT_SRC_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/flow/qt/*.cpp)
file(GLOB FIOCRUZ_FLOW_QT_HDR_FILES ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/flow/qt/*.h)

source_group("Source Files\\flow\\qt"  FILES ${FIOCRUZ_FLOW_QT_SRC_FILES})
source_group("Header Files\\flow\\qt"  FILES ${FIOCRUZ_FLOW_QT_HDR_FILES})

file(GLOB FIOCRUZ_FLOW_UI_FILES  ${FIOCRUZ_ABSOLUTE_ROOT_DIR}/src/fiocruz/flow/qt/ui/*.ui)

source_group("UI Files\\flow"  FILES ${FIOCRUZ_FLOW_UI_FILES})

# uic'ing
QT5_WRAP_UI(FIOCRUZ_GEN_HDR_FILES ${FIOCRUZ_REG_UI_FILES}
								   ${FIOCRUZ_FLOW_UI_FILES})

set(FIOCRUZ_FILES ${FIOCRUZ_SRC_FILES} 
				   ${FIOCRUZ_HDR_FILES} 
				   ${FIOCRUZ_REG_SRC_FILES} 
				   ${FIOCRUZ_REG_HDR_FILES} 
				   ${FIOCRUZ_REG_CORE_SRC_FILES} 
				   ${FIOCRUZ_REG_CORE_HDR_FILES}
				   ${FIOCRUZ_REG_QT_SRC_FILES} 
				   ${FIOCRUZ_REG_QT_HDR_FILES}
				   ${FIOCRUZ_FLOW_SRC_FILES} 
				   ${FIOCRUZ_FLOW_HDR_FILES} 
				   ${FIOCRUZ_FLOW_CORE_SRC_FILES} 
				   ${FIOCRUZ_FLOW_CORE_HDR_FILES}
				   ${FIOCRUZ_FLOW_QT_SRC_FILES} 
				   ${FIOCRUZ_FLOW_QT_HDR_FILES}
				   ${FIOCRUZ_GEN_HDR_FILES})
				   
add_library(fiocruz SHARED ${FIOCRUZ_FILES})

target_link_libraries(fiocruz terralib_mod_plugin terralib_mod_qt_apf)

qt5_use_modules(fiocruz Widgets)

add_custom_command(
      TARGET fiocruz
      POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/bin/Debug/fiocruz.dll" "${terralib_DIR}/debug"
      
      COMMENT "Copying plugin files..."
      VERBATIM
    )

install(TARGETS fiocruz
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

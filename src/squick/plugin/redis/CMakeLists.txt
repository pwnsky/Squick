set(ProjectName "core_redis")
file(GLOB soruces *.cc)
file(GLOB headers *.h)
add_library(${ProjectName} SHARED ${soruces} ${headers})

if (APPLE)
	
elseif(UNIX)
	target_link_libraries(${ProjectName} hiredis.a)
else()
	target_link_libraries(${ProjectName} hiredis.lib)
	target_link_libraries(${ProjectName} ws2_32)
	target_link_libraries(${ProjectName} event.lib)
endif()

LinkSquick(${ProjectName})
SquickOut(${ProjectName} "squick/plugin" ${SQUICK_BUILD_PLUGIN_CORE_PATH} "redis")
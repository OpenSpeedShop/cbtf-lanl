################################################################################
# Copyright (c) 2015 Krell Institute. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

add_library(memTool SHARED
	memTool.cpp mrnetConverters.cpp
)

target_include_directories(memTool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MRNet_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
    ${CBTF_INCLUDE_DIRS}
)

target_link_libraries(memTool
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${CBTF_LIBRARIES}
    ${XercesC_LIBRARIES}
    ${MRNet_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

set_target_properties(memTool PROPERTIES PREFIX "")


install(TARGETS memTool
    LIBRARY DESTINATION share/KrellInstitute/contrib/Components
)

set(contribxmldir ${CMAKE_INSTALL_PREFIX}/share/KrellInstitute/contrib/xml)
set(contriblib_location ${CMAKE_INSTALL_PREFIX}/share/KrellInstitute/contrib/Components)
configure_file(memtool.xml.in memtool.xml @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/memtool.xml
        DESTINATION share/KrellInstitute/contrib/xml)


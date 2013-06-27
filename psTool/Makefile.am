################################################################################
# Copyright (c) 2012 Krell Institute. All Rights Reserved.
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

contribxmldir = $(datadir)/KrellInstitute/contrib/xml
contribplugindir = $(datadir)/KrellInstitute/contrib/Components

BUILT_FILES = pstool.xml

nobase_dist_contribxml_DATA = \
        pstool.xml

contribplugin_LTLIBRARIES = psPlugin.la mrnetPlugin.la

psPlugin_la_SOURCES = psPlugin.cpp

psPlugin_la_CXXFLAGS = \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@

psPlugin_la_LDFLAGS = \
	-L$(top_srcdir) \
	-module -avoid-version -shared \
	@CBTF_LDFLAGS@

psPlugin_la_LIBADD = \
	-lcbtf-core \
	@CBTF_LIBS@

mrnetPlugin_la_SOURCES = mrnetPlugin.cpp

mrnetPlugin_la_CXXFLAGS = \
	@BOOST_CPPFLAGS@ \
	@CBTF_CPPFLAGS@ \
	@MRNET_CPPFLAGS@

mrnetPlugin_la_LDFLAGS = \
        -L$(top_srcdir) \
	-module -avoid-version -shared \
        @CBTF_LDFLAGS@ \
	@MRNET_LDFLAGS@

mrnetPlugin_la_LIBADD = \
	-lcbtf-core \
	-lcbtf-mrnet \
	@CBTF_LIBS@ \
	@MRNET_LIBS@
CLEANFILES = $(BUILT_FILES)

pstool.xml: pstool.xml.in

	@rm -f pstool.xml
	@sed \
                -e 's,@@contriblib_location@@,$(contribplugindir),' \
                $(srcdir)/pstool.xml.in > pstool.xml

EXTRA_DIST = pstool.xml.in

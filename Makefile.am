SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS=-std=c++11 

commoncppflags = -Ithird_party
GEN = 

#
#	Exclude generated sources from distribution
#
nodist_human_detector_SOURCES = $(GEN)
BUILT_SOURCES = $(GEN)
CLEANFILES = $(GEN)

#
# Binaries
#
bin_PROGRAMS = human-detector sigur-last-card

nobase_dist_include_HEADERS = \
	platform.h utiltty.h \
	strptime.h utildate.h \
	wt-sigur.h \
	third_party/argtable3/argtable3.h \
	errlist.h daemonize.h

commonlibs = -L/usr/local/lib 

human_detector_SOURCES = \
	human-detector.cpp utiltty.cpp errlist.cpp \
	daemonize.cpp \
	strptime.cpp utildate.cpp \
	third_party/argtable3/argtable3.c

human_detector_LDADD = $(commonlibs)
human_detector_CPPFLAGS = $(commoncppflags)

sigur_last_card_SOURCES = \
	sigur-last-card.cpp wt-sigur.cpp errlist.cpp \
	daemonize.cpp \
	strptime.cpp utildate.cpp \
	third_party/argtable3/argtable3.c

sigur_last_card_LDADD = $(commonlibs) -lmysqlclient
sigur_last_card_CPPFLAGS = $(commoncppflags)

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
   autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
   third_party/argtable3/README

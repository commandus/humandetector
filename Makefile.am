SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS=-std=c++11 -O0

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
bin_PROGRAMS = human-detector sigur-last-card usb-rfid-card \
	put-temperature-db put-temperature-json say-number

nobase_dist_include_HEADERS = \
	platform.h utiltty.h utildate.h util-cmd.h utilstring.h \
	config-filename.h strptime.h wt-sigur.h \
	third_party/argtable3/argtable3.h \
	errlist.h daemonize.h wavemap.h number2words.h

commonlibs = -L/usr/local/lib 

human_detector_SOURCES = \
	human-detector.cpp utiltty.cpp errlist.cpp \
	daemonize.cpp \
	strptime.cpp utildate.cpp util-cmd.cpp \
	third_party/argtable3/argtable3.c

human_detector_LDADD = $(commonlibs)
human_detector_CPPFLAGS = $(commoncppflags)

sigur_last_card_SOURCES = \
	sigur-last-card.cpp wt-sigur.cpp errlist.cpp \
	daemonize.cpp \
	strptime.cpp utildate.cpp util-cmd.cpp \
	third_party/argtable3/argtable3.c

sigur_last_card_LDADD = $(commonlibs) -lmysqlclient
sigur_last_card_CPPFLAGS = $(commoncppflags)

usb_rfid_card_SOURCES = \
	usb-rfid-card.cpp errlist.cpp \
	daemonize.cpp \
	util-cmd.cpp \
	third_party/argtable3/argtable3.c

usb_rfid_card_LDADD = $(commonlibs)
usb_rfid_card_CPPFLAGS = $(commoncppflags)

put_temperature_db_SOURCES = \
	put-temperature-db.cpp errlist.cpp \
	config-filename.cpp daemonize.cpp \
	util-cmd.cpp utilstring.cpp \
	third_party/argtable3/argtable3.c

put_temperature_db_LDADD = $(commonlibs) -lpq
put_temperature_db_CPPFLAGS = $(commoncppflags)

put_temperature_json_SOURCES = \
	put-temperature-json.cpp errlist.cpp \
	config-filename.cpp daemonize.cpp \
	util-cmd.cpp utilstring.cpp \
	third_party/argtable3/argtable3.c
put_temperature_json_LDADD = $(commonlibs) -lcurl
put_temperature_json_CPPFLAGS = $(commoncppflags)

say_number_SOURCES = \
	say-number.cpp wavemap.cpp errlist.cpp \
	config-filename.cpp daemonize.cpp \
	util-cmd.cpp utilstring.cpp \
	third_party/argtable3/argtable3.c \
	number2words.cpp 
say_number_LDADD = $(commonlibs) -lpulse
say_number_CPPFLAGS = $(commoncppflags) -D_REENTRANT

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
   autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
   third_party/argtable3/README

module_LTLIBRARIES				+= modules/confgen/libconfgen.la
modules_confgen_libconfgen_la_SOURCES		=	\
	modules/confgen/confgen.h			\
	modules/confgen/confgen.c			\
	modules/confgen/confgen-parser.h		\
	modules/confgen/confgen-parser.c		\
	modules/confgen/confgen-plugin.c		\
	modules/confgen/confgen-grammar.y

modules_confgen_libconfgen_la_CPPFLAGS		=	\
	$(AM_CPPFLAGS) 					\
	-I$(top_srcdir)/modules/confgen			\
	-I$(top_builddir)/modules/confgen
modules_confgen_libconfgen_la_LIBADD		=	\
	$(MODULE_DEPS_LIBS)
modules_confgen_libconfgen_la_LDFLAGS		=	\
	$(MODULE_LDFLAGS)
modules_confgen_libconfgen_la_DEPENDENCIES	=	\
	$(MODULE_DEPS_LIBS)

BUILT_SOURCES                                   += \
        modules/confgen/confgen-grammar.y            \
        modules/confgen/confgen-grammar.c            \
        modules/confgen/confgen-grammar.h

EXTRA_DIST                                      += \
        modules/confgen/confgen-grammar.ym


EXTRA_DIST += modules/confgen/CMakeLists.txt

modules/confgen modules/confgen/ mod-confgen: modules/confgen/libconfgen.la
.PHONY: modules/confgen/ mod-confgen

include modules/confgen/tests/Makefile.am

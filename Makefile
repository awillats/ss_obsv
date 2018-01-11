PLUGIN_NAME = ss_obsv

HEADERS = ss_obsv.h

SOURCES = ss_obsv.cpp\
          moc_ss_obsv.cpp\
		  ../../../module_help/StAC_rtxi/dataFuns.cpp\

LIBS = 

### Do not edit below this line ###

include $(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile

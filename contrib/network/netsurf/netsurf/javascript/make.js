CFLAGS += -O2
NETSURF_FB_FRONTEND := sdl
NETSURF_FB_FONTLIB := internal

NETSURF_FRAMEBUFFER_BIN := $(PREFIX)/bin/

  # Default resource install path
  NETSURF_FRAMEBUFFER_RESOURCES := $(PREFIX)/share/netsurf/

  # Default framebuffer search path
  NETSURF_FB_RESPATH := $${HOME}/.netsurf/:$${NETSURFRES}:$(NETSURF_FRAMEBUFFER_RESOURCES):./framebuffer/res

  # freetype compiled in font serch path
  NETSURF_FB_FONTPATH := /usr/share/fonts/truetype/ttf-dejavu:/usr/share/fonts/truetype/msttcorefonts
  
OBJS :=  none.o


OUTFILE = TEST.o
CFLAGS += -I ../include/ -I ../ -I../../ -I./ -I/home/sourcerer/kos_src/newenginek/kolibri/include
include $(MENUETDEV)/makefiles/Makefile_for_o_lib

@erase lang.inc
@echo lang fix sp >lang.inc
@fasm desktop.asm desktop
@kpack desktop
@erase lang.inc
@pause
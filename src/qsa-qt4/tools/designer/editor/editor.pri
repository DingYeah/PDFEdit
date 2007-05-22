isEmpty(EDITOR_BRANCH):EDITOR_BRANCH=.

HEADERS += \
	$$EDITOR_BRANCH/editor.h  \
	$$EDITOR_BRANCH/parenmatcher.h  \
	$$EDITOR_BRANCH/completion.h \
	$$EDITOR_BRANCH/viewmanager.h \
	$$EDITOR_BRANCH/markerwidget.h\	
	$$EDITOR_BRANCH/browser.h \
	$$EDITOR_BRANCH/arghintwidget.h \
	$$EDITOR_BRANCH/cindent.h 
        

SOURCES += \
	$$EDITOR_BRANCH/editor.cpp \
	$$EDITOR_BRANCH/parenmatcher.cpp  \
	$$EDITOR_BRANCH/completion.cpp \
	$$EDITOR_BRANCH/viewmanager.cpp \
	$$EDITOR_BRANCH/markerwidget.cpp \	
	$$EDITOR_BRANCH/browser.cpp \
	$$EDITOR_BRANCH/arghintwidget.cpp \
	$$EDITOR_BRANCH/cindent.cpp \
	$$EDITOR_BRANCH/yyindent.cpp
	

noide {
FORMS += \
	$$EDITOR_BRANCH/../../../src/ide/preferences.ui
HEADERS += $$EDITOR_BRANCH/../../../src/ide/preferences.h 
SOURCES += $$EDITOR_BRANCH/../../../src/ide/preferences.cpp 
}

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_UI(WidgetUISrcs FileSelectionWidget.ui)
QT4_WRAP_CPP(WidgetMOCSrcs FileSelectionWidget.h)

add_library(libWidgets FileSelectionWidget.cpp ${WidgetUISrcs} ${WidgetMOCSrcs})

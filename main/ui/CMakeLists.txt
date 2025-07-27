SET(SOURCES screens/ui_Menu.c
    screens/ui_LedColor.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    fonts/ui_font_H1.c
    fonts/ui_font_Number_big.c
    fonts/ui_font_Number_extra.c
    fonts/ui_font_Subtitle.c
    fonts/ui_font_Title.c)

add_library(ui ${SOURCES})

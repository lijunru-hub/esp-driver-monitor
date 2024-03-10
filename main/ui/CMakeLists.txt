SET(SOURCES screens/ui_Screen1.c
    ui.c
    ui_helpers.c
    images/ui_img_2146430232.c
    images/ui_img_459267587.c
    images/ui_img_284980415.c
    images/ui_img_o2_2_png.c
    images/ui_img_heart_png.c
    images/ui_img_o2_png.c
    fonts/ui_font_PHFont16.c
    fonts/ui_font_PHFont18.c
    fonts/ui_font_PHFont20.c
    fonts/ui_font_PHFont22.c
    fonts/ui_font_PHFont24.c
    fonts/ui_font_PHFont26.c
    fonts/ui_font_PHFont30.c)

add_library(ui ${SOURCES})

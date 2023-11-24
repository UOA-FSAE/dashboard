//
// Created by tazukiswift on 24/11/23.
//

#include "styles.h"
#include "popups.h"
lv_obj_t *left_popup;
lv_obj_t *right_popup;
lv_obj_t *label_popup;

static lv_style_t popup_style;

//

void init_popups() {
    lv_style_init(&popup_style);
    lv_style_set_text_font(&popup_style,&lv_font_montserrat_28);
    lv_style_set_text_align(&popup_style,LV_ALIGN_CENTER);
    lv_style_set_radius(&popup_style, 5);
    lv_style_set_bg_opa(&popup_style, LV_OPA_COVER);
    lv_style_set_bg_color(&popup_style, lv_palette_darken(LV_PALETTE_YELLOW,2));

    lv_style_set_text_color(&popup_style, lv_color_black());

    lv_style_set_border_opa(&popup_style, LV_OPA_COVER);
    lv_style_set_border_color(&popup_style, lv_color_black());
    lv_style_set_pad_all(&popup_style, 10);

    left_popup = lv_label_create(lv_layer_top());    // Left (test) popup
    lv_obj_add_style(left_popup,&popup_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(left_popup,&bitstream_vera_sans_200,LV_PART_MAIN);
    lv_obj_align(left_popup, LV_ALIGN_LEFT_MID, 50, 25);
    lv_label_set_text_fmt(left_popup,"%d",5);

    right_popup = lv_label_create(lv_layer_top());    // Left (test) popup
    lv_obj_add_style(right_popup,&popup_style,LV_PART_MAIN);

    lv_obj_set_style_text_font(right_popup,&bitstream_vera_sans_200,LV_PART_MAIN);
    lv_obj_align(right_popup, LV_ALIGN_RIGHT_MID, -50, 25);
    lv_label_set_text_fmt(right_popup,"%d",5);

    label_popup = lv_label_create(lv_layer_top());    // Left (test) popup
    lv_obj_add_style(label_popup,&popup_style,LV_PART_MAIN);
    lv_obj_set_width(label_popup,380);
    lv_obj_set_style_text_align(label_popup,LV_TEXT_ALIGN_CENTER,LV_PART_MAIN);
    lv_obj_align(label_popup, LV_ALIGN_TOP_LEFT, 50, 10);
    lv_label_set_text(label_popup,"Updated Dials");

    disable_popup();
}

void set_popup(int L,int R) {
    lv_label_set_text_fmt(right_popup,"%d",R);
    lv_label_set_text_fmt(left_popup,"%d",L);
    lv_obj_clear_flag(label_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(right_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(left_popup, LV_OBJ_FLAG_HIDDEN);
}

void disable_popup() {
    lv_obj_add_flag(label_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(right_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(left_popup, LV_OBJ_FLAG_HIDDEN);
}

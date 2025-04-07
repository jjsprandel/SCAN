#include "ui_styles.h"

// Static style variables for different components and states
static lv_style_t background_styles[5]; // Normal, Success, Warning, Error, Admin
static lv_style_t button_styles[5];
static lv_style_t text_styles[5];
static lv_style_t card_styles[5];
static lv_style_t icon_styles[5];
static lv_style_t progress_styles[5];

// Initialize all UI styles
void ui_styles_init(void)
{
    // Initialize background styles
    for (int i = 0; i < 5; i++)
    {
        lv_style_init(&background_styles[i]);
        lv_style_set_radius(&background_styles[i], 0);
        lv_style_set_border_width(&background_styles[i], 0);
    }

    // Set specific background colors based on state
    lv_style_set_bg_color(&background_styles[UI_STATE_NORMAL], UI_COLOR_PRIMARY);
    lv_style_set_bg_color(&background_styles[UI_STATE_SUCCESS], UI_COLOR_SUCCESS);
    lv_style_set_bg_color(&background_styles[UI_STATE_WARNING], UI_COLOR_WARNING);
    lv_style_set_bg_color(&background_styles[UI_STATE_ERROR], UI_COLOR_ERROR);
    lv_style_set_bg_color(&background_styles[UI_STATE_ADMIN], UI_COLOR_BACKGROUND);

    // Initialize text styles
    for (int i = 0; i < 5; i++)
    {
        lv_style_init(&text_styles[i]);
        lv_style_set_text_color(&text_styles[i], UI_COLOR_TEXT_LIGHT);
        lv_style_set_text_font(&text_styles[i], UI_FONT_MEDIUM);
        lv_style_set_text_align(&text_styles[i], LV_TEXT_ALIGN_CENTER);
    }

    // Initialize button styles
    for (int i = 0; i < 5; i++)
    {
        lv_style_init(&button_styles[i]);
        lv_style_set_radius(&button_styles[i], 10);
        lv_style_set_border_width(&button_styles[i], 0);
        lv_style_set_pad_all(&button_styles[i], 10);
        lv_style_set_shadow_width(&button_styles[i], 10);
        lv_style_set_shadow_opa(&button_styles[i], LV_OPA_50);
        lv_style_set_text_color(&button_styles[i], UI_COLOR_TEXT_LIGHT);
        lv_style_set_text_font(&button_styles[i], UI_FONT_SMALL);
    }

    // Set specific button colors based on state
    lv_style_set_bg_color(&button_styles[UI_STATE_NORMAL], UI_COLOR_PRIMARY);
    lv_style_set_bg_color(&button_styles[UI_STATE_SUCCESS], UI_COLOR_SUCCESS);
    lv_style_set_bg_color(&button_styles[UI_STATE_WARNING], UI_COLOR_WARNING);
    lv_style_set_bg_color(&button_styles[UI_STATE_ERROR], UI_COLOR_ERROR);
    lv_style_set_bg_color(&button_styles[UI_STATE_ADMIN], UI_COLOR_BACKGROUND);

    // Initialize card styles
    for (int i = 0; i < 5; i++)
    {
        lv_style_init(&card_styles[i]);
        lv_style_set_radius(&card_styles[i], 8);
        lv_style_set_bg_opa(&card_styles[i], LV_OPA_80);
        lv_style_set_border_width(&card_styles[i], 0);
        lv_style_set_pad_all(&card_styles[i], 8);
        lv_style_set_shadow_width(&card_styles[i], 8);
        lv_style_set_shadow_opa(&card_styles[i], LV_OPA_10); // Changed from LV_OPA_15 to LV_OPA_10
    }

    // Set specific card colors based on state
    lv_style_set_bg_color(&card_styles[UI_STATE_NORMAL], UI_COLOR_NEUTRAL);
    lv_style_set_bg_color(&card_styles[UI_STATE_SUCCESS], lv_color_hex(0xd5f5e3)); // Light green
    lv_style_set_bg_color(&card_styles[UI_STATE_WARNING], lv_color_hex(0xfdf2e9)); // Lighter orange (was 0xfdebd0)
    lv_style_set_bg_color(&card_styles[UI_STATE_ERROR], lv_color_hex(0xfadbd8));   // Light red
    lv_style_set_bg_color(&card_styles[UI_STATE_ADMIN], lv_color_hex(0xeaecee));   // Light gray

    // Initialize icon styles
    for (int i = 0; i < 5; i++)
    {
        lv_style_init(&icon_styles[i]);
        lv_style_set_img_recolor_opa(&icon_styles[i], LV_OPA_100);
        lv_style_set_img_opa(&icon_styles[i], LV_OPA_100);
    }

    // Set specific icon colors based on state
    lv_style_set_img_recolor(&icon_styles[UI_STATE_NORMAL], UI_COLOR_PRIMARY);
    lv_style_set_img_recolor(&icon_styles[UI_STATE_SUCCESS], UI_COLOR_SUCCESS);
    lv_style_set_img_recolor(&icon_styles[UI_STATE_WARNING], UI_COLOR_WARNING);
    lv_style_set_img_recolor(&icon_styles[UI_STATE_ERROR], UI_COLOR_ERROR);
    lv_style_set_img_recolor(&icon_styles[UI_STATE_ADMIN], UI_COLOR_BACKGROUND);

    // Initialize progress styles
    for (int i = 0; i < 5; i++)
    {
        lv_style_init(&progress_styles[i]);
        lv_style_set_radius(&progress_styles[i], 10);
        lv_style_set_border_width(&progress_styles[i], 0);
    }

    // Set specific progress colors based on state
    lv_style_set_bg_color(&progress_styles[UI_STATE_NORMAL], UI_COLOR_PRIMARY);
    lv_style_set_bg_color(&progress_styles[UI_STATE_SUCCESS], UI_COLOR_SUCCESS);
    lv_style_set_bg_color(&progress_styles[UI_STATE_WARNING], UI_COLOR_WARNING);
    lv_style_set_bg_color(&progress_styles[UI_STATE_ERROR], UI_COLOR_ERROR);
    lv_style_set_bg_color(&progress_styles[UI_STATE_ADMIN], UI_COLOR_BACKGROUND);
}

// Get a specific style based on type and state
lv_style_t *ui_get_style(ui_style_type_t type, ui_state_t state)
{
    switch (type)
    {
    case UI_STYLE_BACKGROUND:
        return &background_styles[state];
    case UI_STYLE_BUTTON:
        return &button_styles[state];
    case UI_STYLE_TEXT:
        return &text_styles[state];
    case UI_STYLE_CARD:
        return &card_styles[state];
    case UI_STYLE_ICON:
        return &icon_styles[state];
    case UI_STYLE_PROGRESS:
        return &progress_styles[state];
    default:
        return NULL;
    }
}

// Create a screen with appropriate background based on state
lv_obj_t *ui_create_screen(ui_state_t state)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_add_style(screen, ui_get_style(UI_STYLE_BACKGROUND, state), 0);
    lv_obj_set_size(screen, UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT);
    return screen;
}

// Create a centered container for content
lv_obj_t *ui_create_content_container(lv_obj_t *parent)
{
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, LV_PCT(80), LV_PCT(80)); // Further reduced width for better fit on circular displays
    lv_obj_set_style_pad_all(container, 6, 0);          // Increased padding to keep content away from edges
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_0, 0);
    lv_obj_center(container);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(container, 4, 0); // Further reduced row padding for more compact layout
    return container;
}

// Create a standard title label
lv_obj_t *ui_create_title(lv_obj_t *parent, const char *text, ui_state_t state)
{
    lv_obj_t *title = lv_label_create(parent);
    lv_obj_add_style(title, ui_get_style(UI_STYLE_TEXT, state), 0);
    lv_obj_set_style_text_font(title, UI_FONT_LARGE, 0);
    lv_label_set_text(title, text);
    return title;
}

// Set up screen transition animation
void ui_set_screen_transition(lv_obj_t *new_screen)
{
    // Check if the requested screen is already loaded
    lv_obj_t *current_screen = lv_scr_act();

    // Skip transition if the same screen is already displayed
    if (current_screen == new_screen)
    {
        return;
    }

    // Load the new screen instantly without animation
    lv_scr_load(new_screen);
}
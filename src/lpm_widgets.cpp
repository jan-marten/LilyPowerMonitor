#include "lvgl/lvgl.h"
#include <stdio.h>

// these functions are available in main.cpp;
extern void setBrightness(uint8_t level);
extern void relayTurnOff(void);
extern void relayTurnOn(void);
extern void turnOnUSB(void);
extern void turnOffUSB(void);

// STATIC PROTOTYPES
static void CreateTabStatus(lv_obj_t *parent);
static void CreateTabChart(lv_obj_t *parent);
static void CreateTabPV(lv_obj_t *parent);
static void CreateTabSettings(lv_obj_t *parent);

static lv_obj_t *tabView;

// for each tab button:
static lv_obj_t *tabStatus;
static lv_obj_t *tabChart;
static lv_obj_t *tabPV;
static lv_obj_t *tabSettings;

static lv_style_t style_box;

static char buff[16]; // used for formatting floats into strings

void lpm_widgets(void)
{
    lv_style_init(&style_box);
    lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, - LV_DPX(10));
    lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(30));

    tabView = lv_tabview_create(lv_scr_act(), NULL);

    tabStatus = lv_tabview_add_tab(tabView, "Status");
    CreateTabStatus(tabStatus);

    tabChart = lv_tabview_add_tab(tabView, "Charts");
    CreateTabChart(tabChart);

    tabPV = lv_tabview_add_tab(tabView, "PV");
    CreateTabPV(tabPV);

    tabSettings = lv_tabview_add_tab(tabView, "Settings");
    CreateTabSettings(tabSettings);
}

static void CreateTabStatus(lv_obj_t *parent)
{
    // Here comes current status;
    
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);

    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_coord_t grid_w_meter;
    if (disp_size <= LV_DISP_SIZE_SMALL) grid_w_meter = lv_page_get_width_grid(parent, 1, 1);
    else if (disp_size <= LV_DISP_SIZE_MEDIUM) grid_w_meter = lv_page_get_width_grid(parent, 2, 1);
    else grid_w_meter = lv_page_get_width_grid(parent, 3, 1);

    lv_coord_t meter_h = lv_page_get_height_fit(parent);
    lv_coord_t meter_size = LV_MATH_MIN(grid_w_meter, meter_h);
    meter_size -= 15; // add some margin because the gauges are placed inside containers

    lv_obj_t *container;
    lv_obj_t *gauge;
    lv_obj_t *label;
    lv_obj_t *table;

    // Container: Usage + Current electricity usage + Total today [ Power / Gas / Euros ]
    container = lv_cont_create(parent, NULL);
    lv_cont_set_layout(container, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(container, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_drag_parent(container, true);

    lv_cont_set_fit2(container, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(container, grid_w);
    lv_obj_set_style_local_value_str(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Usage");

    gauge = lv_gauge_create(container, NULL);
    lv_obj_set_drag_parent(gauge, true);
    lv_obj_set_size(gauge, meter_size, meter_size);
    lv_obj_add_style(gauge, LV_GAUGE_PART_MAIN, &style_box);
    lv_obj_set_style_local_value_str(gauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, "Usage");

    label = lv_label_create(gauge, label);
    lv_obj_align(label, gauge, LV_ALIGN_CENTER, 0, grid_w / 3);
    lv_label_set_text_fmt(label, "%d W", 0);

    // Datatable;
    table = lv_table_create(container, NULL);
    lv_obj_set_drag_parent(table, true);
    
    // Column definitions
    lv_table_set_col_cnt(table, 2); // 2-columns only
    lv_table_set_col_width(table, 0, grid_w  / 2);
    lv_table_set_col_width(table, 1, grid_w / 2);

    uint16_t row = 0;
    lv_table_set_cell_value(table, row, 0, "Today");
    
    lv_table_set_cell_value(table, ++row, 0, "Energy");
    snprintf(buff, sizeof(buff), "%.2f kW", 1.234);
    lv_table_set_cell_value(table, row, 1, buff);

    lv_table_set_cell_value(table, ++row, 0, "Gas");
    snprintf(buff, sizeof(buff), "%.2f m3", 1.234);
    lv_table_set_cell_value(table, row, 1, buff);

    lv_table_set_cell_value(table, ++row, 0, "Cost ");
    snprintf(buff, sizeof(buff), "%.2f Euro", 1.234);
    lv_table_set_cell_value(table, row, 1, buff);

    // Container: Delivered + Current electricity returned +  Total today [ Power / Euros ]
    container = lv_cont_create(parent, container);
    lv_cont_set_layout(container, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(container, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_drag_parent(container, true);

    lv_cont_set_fit2(container, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_width(container, grid_w);
    lv_obj_set_style_local_value_str(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Delivered");

    gauge = lv_gauge_create(container, NULL);
    lv_obj_set_drag_parent(gauge, true);
    lv_obj_set_size(gauge, meter_size, meter_size);
    lv_obj_add_style(gauge, LV_GAUGE_PART_MAIN, &style_box);
    lv_obj_set_style_local_value_str(gauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, "Delivered");

    label = lv_label_create(gauge, label);
    lv_obj_align(label, gauge, LV_ALIGN_CENTER, 0, grid_w / 3);
    lv_label_set_text_fmt(label, "%d W", 0);

    // Datatable;
    table = lv_table_create(container, NULL);
    lv_obj_set_drag_parent(table, true);
    
    // Column definitions
    lv_table_set_col_cnt(table, 2); // 2-columns only
    lv_table_set_col_width(table, 0, grid_w  / 2);
    lv_table_set_col_width(table, 1, grid_w / 2);

    row = 0;
    lv_table_set_cell_value(table, row, 0, "Today");
    
    lv_table_set_cell_value(table, ++row, 0, "Energy");
    snprintf(buff, sizeof(buff), "%.2f kW", 1.234);
    lv_table_set_cell_value(table, row, 1, buff);

    lv_table_set_cell_value(table, ++row, 0, "Profits");
    snprintf(buff, sizeof(buff), "%.2f Euro", 1.234);
    lv_table_set_cell_value(table, row, 1, buff);
}

static void CreateTabChart(lv_obj_t *parent)
{
    // Here comes chart data for power and gas consumption

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
}

static void CreateTabPV(lv_obj_t *parent)
{
    // Here comes chart data retrieved from SolarEdge

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);
}

static void CreateTabSettings(lv_obj_t *parent)
{
    // Here comes settings for various things;
    // - WiFi (SSID + password)
    // - P1Mon (URL + Token + update frequency)
    // - Theme (light / dark), brightness
    // - Date/Time (?)

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
}

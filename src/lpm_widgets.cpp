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

lv_obj_t *chartPower;
lv_chart_series_t *chartSeriesPower;

lv_obj_t *chartPV;
lv_chart_series_t *chartSeriesPV;

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

    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);

    lv_coord_t grid_h_chart = lv_page_get_height_grid(parent, 1, 1);
    lv_coord_t grid_w_chart = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_LARGE ? 1 : 2, 1);

    chartPower = lv_chart_create(parent, NULL);
    lv_obj_add_style(chartPower, LV_CHART_PART_BG, &style_box);
    if (disp_size <= LV_DISP_SIZE_SMALL) {
        lv_obj_set_style_local_text_font(chartPower, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, lv_theme_get_font_small());
    }
    lv_obj_set_drag_parent(chartPower, true);
    lv_obj_set_style_local_value_str(chartPower, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Power consumption");
    lv_obj_set_width_margin(chartPower, grid_w_chart);
    lv_obj_set_height_margin(chartPower, grid_h_chart);
    lv_chart_set_div_line_count(chartPower, 3, 0);
    lv_chart_set_point_count(chartPower, 12); // 12 hours of data is displayed
    lv_chart_set_range(chartPower, 0, 4000);
    lv_chart_set_type(chartPower, LV_CHART_TYPE_LINE);
    if (disp_size > LV_DISP_SIZE_SMALL) {
        lv_obj_set_style_local_pad_left(chartPower,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 4 * (LV_DPI / 10));
        lv_obj_set_style_local_pad_bottom(chartPower,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 3 * (LV_DPI / 10));
        lv_obj_set_style_local_pad_right(chartPower,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 2 * (LV_DPI / 10));
        lv_obj_set_style_local_pad_top(chartPower,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 2 * (LV_DPI / 10));
        lv_chart_set_y_tick_length(chartPower, 0, 0);
        lv_chart_set_x_tick_length(chartPower, 0, 0);
        lv_chart_set_y_tick_texts(chartPower, "4.0\n3.5\n3.0\n2.5\n2.0\n1.5\n1.0\n0.5\n0", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
        lv_chart_set_x_tick_texts(chartPower, "7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
    }

    chartSeriesPower = lv_chart_add_series(chartPower, LV_THEME_DEFAULT_COLOR_PRIMARY);
    lv_chart_set_next(chartPower, chartSeriesPower, 10);
    lv_chart_set_next(chartPower, chartSeriesPower, 200);
    lv_chart_set_next(chartPower, chartSeriesPower, 3000);
    lv_chart_set_next(chartPower, chartSeriesPower, 400);
    lv_chart_set_next(chartPower, chartSeriesPower, 50);
    lv_chart_set_next(chartPower, chartSeriesPower, 600);
    lv_chart_set_next(chartPower, chartSeriesPower, 750);
    lv_chart_set_next(chartPower, chartSeriesPower, 840);
    lv_chart_set_next(chartPower, chartSeriesPower, 900);
    lv_chart_set_next(chartPower, chartSeriesPower, 1000);
    lv_chart_set_next(chartPower, chartSeriesPower, 500);
    lv_chart_set_next(chartPower, chartSeriesPower, 50);

}

static void CreateTabPV(lv_obj_t *parent)
{
    // Here comes chart data retrieved from SolarEdge

   lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);

    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);

    lv_coord_t grid_h_chart = lv_page_get_height_grid(parent, 1, 1);
    lv_coord_t grid_w_chart = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_LARGE ? 1 : 2, 1);

    chartPV = lv_chart_create(parent, NULL);
    lv_obj_add_style(chartPV, LV_CHART_PART_BG, &style_box);
    if (disp_size <= LV_DISP_SIZE_SMALL) {
        lv_obj_set_style_local_text_font(chartPV, LV_CHART_PART_SERIES_BG, LV_STATE_DEFAULT, lv_theme_get_font_small());
    }
    lv_obj_set_drag_parent(chartPV, true);
    lv_obj_set_style_local_value_str(chartPV, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Solar power generated");
    lv_obj_set_width_margin(chartPV, grid_w_chart);
    lv_obj_set_height_margin(chartPV, grid_h_chart);
    lv_chart_set_div_line_count(chartPV, 3, 0);
    lv_chart_set_point_count(chartPV, 12); // 12 hours of data is displayed
    lv_chart_set_range(chartPV, 0, 4000);
    lv_chart_set_type(chartPV, LV_CHART_TYPE_LINE);
    if (disp_size > LV_DISP_SIZE_SMALL) {
        lv_obj_set_style_local_pad_left(chartPV,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 4 * (LV_DPI / 10));
        lv_obj_set_style_local_pad_bottom(chartPV,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 3 * (LV_DPI / 10));
        lv_obj_set_style_local_pad_right(chartPV,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 2 * (LV_DPI / 10));
        lv_obj_set_style_local_pad_top(chartPV,  LV_CHART_PART_BG, LV_STATE_DEFAULT, 2 * (LV_DPI / 10));
        lv_chart_set_y_tick_length(chartPV, 0, 0);
        lv_chart_set_x_tick_length(chartPV, 0, 0);
        lv_chart_set_y_tick_texts(chartPV, "4.0\n3.5\n3.0\n2.5\n2.0\n1.5\n1.0\n0.5\n0", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
        lv_chart_set_x_tick_texts(chartPV, "7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18", 0, LV_CHART_AXIS_DRAW_LAST_TICK);
    }

    chartSeriesPV = lv_chart_add_series(chartPV, LV_THEME_DEFAULT_COLOR_PRIMARY);
    lv_chart_set_next(chartPV, chartSeriesPV, 0);
    lv_chart_set_next(chartPV, chartSeriesPV, 500);
    lv_chart_set_next(chartPV, chartSeriesPV, 1200);
    lv_chart_set_next(chartPV, chartSeriesPV, 1800);
    lv_chart_set_next(chartPV, chartSeriesPV, 2700);
    lv_chart_set_next(chartPV, chartSeriesPV, 3000);
    lv_chart_set_next(chartPV, chartSeriesPV, 3500);
    lv_chart_set_next(chartPV, chartSeriesPV, 2500);
    lv_chart_set_next(chartPV, chartSeriesPV, 1500);
    lv_chart_set_next(chartPV, chartSeriesPV, 1000);
    lv_chart_set_next(chartPV, chartSeriesPV, 500);
    lv_chart_set_next(chartPV, chartSeriesPV, 0);
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

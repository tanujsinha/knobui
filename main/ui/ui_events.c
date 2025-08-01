// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.11
// Project name: Smartwatch

#include "ui.h"
#include "i2c_equipment.h"

static const char *TAG = "counter";

void OpenLedScreen(lv_event_t * e)
{
	lv_obj_t *roller = lv_event_get_target(e);

	// Get the selected index
	uint16_t selected_index = lv_roller_get_selected(roller);
	switch(selected_index)
	{
		case 0:
			break;
		case 1:
			lv_disp_load_scr( ui_LedColor);
			break;
		case 2:
		case 3:
		default:
			break;
	}
	
	// Your code here
}

void ReturnToMenu(lv_event_t * e)
{
	lv_disp_load_scr( ui_Menu);
}
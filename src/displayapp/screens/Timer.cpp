#include "displayapp/screens/Timer.h"

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  Timer* screen = static_cast<Timer*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

void Timer::createButtons() {
  rollerMinutes = lv_roller_create(lv_scr_act(), nullptr);
  lv_roller_set_options(rollerMinutes,
    "59\n58\n57\n56\n55\n54\n53\n52\n51\n50\n"
    "49\n48\n47\n46\n45\n44\n43\n42\n41\n40\n"
    "39\n38\n37\n36\n35\n34\n33\n32\n31\n30\n"
    "29\n28\n27\n26\n25\n24\n23\n22\n21\n20\n"
    "19\n18\n17\n16\n15\n14\n13\n12\n11\n10\n"
    "09\n08\n07\n06\n05\n04\n03\n02\n01\n00",
    LV_ROLLER_MODE_INFINITE
  );
  lv_roller_set_selected(rollerMinutes, 59 - minutesToSet, LV_ANIM_OFF);
  lv_roller_set_visible_row_count(rollerMinutes, 3);
  lv_obj_set_style_local_text_font(rollerMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(rollerMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_height(rollerMinutes, 228);
  lv_obj_align(rollerMinutes, lv_scr_act(), LV_ALIGN_CENTER, -69, 0);

  rollerSeconds = lv_roller_create(lv_scr_act(), nullptr);
  lv_roller_set_options(rollerSeconds,
    "59\n58\n57\n56\n55\n54\n53\n52\n51\n50\n"
    "49\n48\n47\n46\n45\n44\n43\n42\n41\n40\n"
    "39\n38\n37\n36\n35\n34\n33\n32\n31\n30\n"
    "29\n28\n27\n26\n25\n24\n23\n22\n21\n20\n"
    "19\n18\n17\n16\n15\n14\n13\n12\n11\n10\n"
    "09\n08\n07\n06\n05\n04\n03\n02\n01\n00",
    LV_ROLLER_MODE_INFINITE
  );
  lv_roller_set_selected(rollerSeconds, 59 - secondsToSet, LV_ANIM_OFF);
  lv_roller_set_visible_row_count(rollerSeconds, 3);
  lv_obj_set_style_local_text_font(rollerSeconds, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(rollerSeconds, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_height(rollerSeconds, 228);
  lv_obj_align(rollerSeconds, lv_scr_act(), LV_ALIGN_CENTER, 69, 0);
}

Timer::Timer(DisplayApp* app, Controllers::TimerController& timerController)
  : Screen(app), running {true}, timerController {timerController} {
  time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  if (timerController.IsRunning()) {
    uint32_t seconds = timerController.GetTimeRemaining() / 1000;
    lv_label_set_text_fmt(time, "%02lu:%02lu", seconds / 60, seconds % 60);
  } else {
    lv_label_set_text(time, ":");
  }
  lv_obj_align(time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_height(btnPlayPause, 40);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  if (timerController.IsRunning()) {
    lv_label_set_text(txtPlayPause, Symbols::pause);
  } else {
    lv_label_set_text(txtPlayPause, Symbols::play);
    createButtons();
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Timer::~Timer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Timer::Refresh() {
  if (timerController.IsRunning()) {
    uint32_t seconds = timerController.GetTimeRemaining() / 1000;
    lv_label_set_text_fmt(time, "%02lu:%02lu", seconds / 60, seconds % 60);
  }
}

void Timer::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED && obj == btnPlayPause) {
    if (timerController.IsRunning()) {
      lv_label_set_text(txtPlayPause, Symbols::play);
      uint32_t seconds = timerController.GetTimeRemaining() / 1000;
      minutesToSet = seconds / 60;
      secondsToSet = seconds % 60;
      timerController.StopTimer();
      createButtons();
      lv_label_set_text(time, ":");
      lv_obj_realign(time);
    } else {
      minutesToSet = 59 - lv_roller_get_selected(rollerMinutes);
      secondsToSet = 59 - lv_roller_get_selected(rollerSeconds);
      if (secondsToSet + minutesToSet > 0) {
        lv_label_set_text(txtPlayPause, Symbols::pause);
        timerController.StartTimer((secondsToSet + minutesToSet * 60) * 1000);

        lv_label_set_text_fmt(time, "%02d:%02d", minutesToSet, secondsToSet);
        lv_obj_realign(time);

        lv_obj_del(rollerMinutes);
        rollerMinutes = nullptr;
        lv_obj_del(rollerSeconds);
        rollerSeconds = nullptr;
      }
    }
  }
}

void Timer::setDone() {
  lv_label_set_text(time, ":");
  lv_obj_realign(time);
  lv_label_set_text(txtPlayPause, Symbols::play);
  secondsToSet = 0;
  minutesToSet = 0;
  createButtons();
}

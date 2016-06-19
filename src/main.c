#include <pebble.h>

#define KEY_CHARACTER 0
#define KEY_KANA 1
#define KEY_MEANING 2
#define KEY_TYPE 3
#define KEY_REV 4
#define CHARACTER_KEY 5
#define KANA_KEY 6
#define MEANING_KEY 7
#define TYPE_KEY 8

// backup api key on watch and phone. if not on watch, check phone.
// fixes update problem of deleting saved data


static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_character_layer;
static TextLayer *s_kana_layer;
static TextLayer *s_meaning_layer;
static TextLayer *s_revdue_layer;
static TextLayer *s_levtitle_layer;
static GFont s_time_font;
static GFont s_weather_font;

static char character_buffer[32]; // Buffers used in inbox_recieved_callback() & init()
static char kana_buffer[32]; 
static char meaning_buffer[32];
static char type_buffer[2];
static char rev_buffer[10];


// Recieved messages from Javascript
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    
  
  // Read tuples for data
  Tuple *char_tuple = dict_find(iterator, KEY_CHARACTER); 
  Tuple *kan_tuple = dict_find(iterator, KEY_KANA);
  Tuple *mean_tuple = dict_find(iterator, KEY_MEANING);
  Tuple *type_tuple = dict_find(iterator, KEY_TYPE);
  Tuple *rev_tuple = dict_find(iterator, KEY_REV);

  
  // If all data is available, use it
  if(char_tuple && mean_tuple) {
    
    
    snprintf(character_buffer, sizeof(character_buffer), "%s", char_tuple->value->cstring);
    snprintf(kana_buffer, sizeof(kana_buffer), "%s", kan_tuple->value->cstring);
    snprintf(meaning_buffer, sizeof(meaning_buffer), "%s", mean_tuple->value->cstring);
    snprintf(type_buffer, sizeof(type_buffer), "%s", type_tuple->value->cstring);

    
    // Assemble full string and display
    text_layer_set_text(s_character_layer, character_buffer);
    text_layer_set_text(s_kana_layer, kana_buffer);
    text_layer_set_text(s_meaning_layer, meaning_buffer);
    
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "type_buffer: %d", *type_buffer);
    // Set Background Color based on Type |  8 1 3 r k v
    if(*type_buffer == 8) {
      window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorCadetBlue, GColorBlack) );
      
      
      
      text_layer_set_text(s_levtitle_layer, "Radical");} //radical 8
    else if(*type_buffer == 1) {
      window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorFolly, GColorBlack) );
      text_layer_set_text(s_levtitle_layer, "Kanji");} // kanji 1
    else if(*type_buffer == 3) {
      window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorImperialPurple, GColorBlack) );
      text_layer_set_text(s_levtitle_layer, "Vocabulary");}// vocab 3
    
    
    
  }
  if(rev_tuple) {
    
    snprintf(rev_buffer, sizeof(rev_buffer), "「%d」", (int)rev_tuple->value->int32);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "rev_buffer: %d", *rev_buffer);
    
    // SET REVIEW TEXT HERE
    text_layer_set_text(s_revdue_layer, rev_buffer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "set rev_buffer text");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "rev_buffer: %d", *rev_buffer);
    
  }
  
}

// Log Errors and such
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// Time & Ticks
static void update_time() {
  time_t temp = time(NULL); // Get a tm structure
  struct tm *tick_time = localtime(&temp);
  static char s_buffer[8];// Write the current hours and minutes into a buffer
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer);// Display this time on the TextLayer
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;// Begin dictionary
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);// Add a key-value pair
    app_message_outbox_send();// Send the message!
  }
}






static void main_window_load(Window *window) {
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);  
  GRect bounds = layer_get_bounds(window_layer);
  
  
  
  // ++++++++++ CLOCK Text ++++++++++++++
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, 38, bounds.size.w, 50));  
  text_layer_set_background_color(s_time_layer, GColorClear);  // Improve the layout to be more like a watchface
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  //s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_14)); // Create GFont
  //text_layer_set_font(s_time_layer, s_time_font);// Apply to TextLayer
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));  // Add it as a child layer to the Window's root layer
  
  // ++++++++++ MEANING Text ++++++++++++++
  // Create temperature Layer
  s_meaning_layer = text_layer_create(
      GRect(0, 135, bounds.size.w, 50));
  text_layer_set_background_color(s_meaning_layer, GColorClear);  // Style the text
  text_layer_set_text_color(s_meaning_layer, GColorWhite);
  text_layer_set_text_alignment(s_meaning_layer, GTextAlignmentCenter);
  text_layer_set_text(s_meaning_layer, "Crabigator");// あン一営嘆噌\n器四"
  text_layer_set_font(s_meaning_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));  // Set Font, apply it and add to Window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_meaning_layer));
  
  // ++++++++++ KANA Text ++++++++++++++
  // Create the TextLayer with specific bounds
  s_kana_layer = text_layer_create(
      GRect(0, 105, bounds.size.w, 32));
  text_layer_set_background_color(s_kana_layer, GColorClear);  // Improve the layout to be more like a watchface
  text_layer_set_text_color(s_kana_layer, GColorWhite);
  text_layer_set_text(s_kana_layer, "WaniKani");
  text_layer_set_font(s_kana_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_kana_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_kana_layer));  // Add it as a child layer to the Window's root layer
  
  // ++++++++++ CHARACTER Text ++++++++++++++
  // Create the TextLayer with specific bounds
  s_character_layer = text_layer_create(
      GRect(0, 80, bounds.size.w, 32));
  text_layer_set_background_color(s_character_layer, GColorClear);  // Improve the layout to be more like a watchface
  text_layer_set_text_color(s_character_layer, GColorWhite);
  text_layer_set_text(s_character_layer, "KANJI");
  text_layer_set_font(s_character_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_character_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_character_layer));  // Add it as a child layer to the Window's root layer

  // +++++++++ REVIEW DUE Text ++++++++++++++
  // Create the TextLayer with specific bounds
  s_revdue_layer = text_layer_create(
      GRect(0, 10, bounds.size.w, 28));
  text_layer_set_background_color(s_revdue_layer, GColorClear);  // Improve the layout to be more like a watchface
  text_layer_set_text_color(s_revdue_layer, GColorWhite);
  text_layer_set_text(s_revdue_layer, "");
  text_layer_set_font(s_revdue_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_revdue_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_revdue_layer));  // Add it as a child layer to the Window's root layer

  // +++++++++ LEVEL & TITLE Text ++++++++++++
  // Create the TextLayer with specific bounds
  s_levtitle_layer = text_layer_create(
      GRect(0, 28, bounds.size.w, 28));
  text_layer_set_background_color(s_levtitle_layer, GColorClear);  // Improve the layout to be more like a watchface
  text_layer_set_text_color(s_levtitle_layer, GColorWhite);
  text_layer_set_text(s_levtitle_layer, "Kanji"); // Compatible symbols ≠≈∆º
  text_layer_set_font(s_levtitle_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_levtitle_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_levtitle_layer));  // Add it as a child layer to the Window's root layer

}





static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);  // Destroy TextLayer
  fonts_unload_custom_font(s_time_font);  // Unload GFont
  text_layer_destroy(s_meaning_layer);  // Destroy weather elements
  fonts_unload_custom_font(s_weather_font);
}
static void init() {
  s_main_window = window_create();  // Create main Window element and assign to pointer
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });// Set handlers to manage the elements inside the Window
  window_stack_push(s_main_window, true);  // Show the Window on the watch, with animated=true
  update_time();// Make sure the time is displayed from the start
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);// Register with TickTimerService

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());// Open AppMessage
  
  //================== Load last card ======================
  
  
  // Check for keys. If not exist, set them with blank data to look nice
  if (persist_exists(CHARACTER_KEY) == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No Character Key! Adding");
    persist_write_string(CHARACTER_KEY, "鰐蟹");
    
    // Remove below before release
    //persist_read_string(CHARACTER_KEY, character_buffer, sizeof(character_buffer));
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "CHARACTER KEY: %s", character_buffer);
  }
  if (persist_exists(KANA_KEY) == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No Kana Key! Adding");
    persist_write_string(KANA_KEY, "ワニカニ");
    
    // Remove below before release
    //persist_read_string(KANA_KEY, kana_buffer, sizeof(kana_buffer));
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "KANA KEY: %s", kana_buffer);
  }
  if (persist_exists(MEANING_KEY) == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No Meaning Key! Adding");
    persist_write_string(MEANING_KEY, "WaniKani.com");
    
    // Remove below before release
    //persist_read_string(MEANING_KEY, meaning_buffer, sizeof(meaning_buffer));
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "MEANING KEY: %s", meaning_buffer);
  }
  if (persist_exists(TYPE_KEY) == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No Type Key! Adding");
    persist_write_string(TYPE_KEY, "1");
    
    // Remove below before release
    //persist_read_string(TYPE_KEY, type_buffer, sizeof(type_buffer));
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "TYPE KEY: %s", meaning_buffer);
  }
  
  // Load persistant memory into buffer and set texts!
  persist_read_string(CHARACTER_KEY, character_buffer, sizeof(character_buffer));
  persist_read_string(KANA_KEY, kana_buffer, sizeof(kana_buffer));
  persist_read_string(MEANING_KEY, meaning_buffer, sizeof(meaning_buffer));
  persist_read_string(TYPE_KEY, type_buffer, sizeof(type_buffer));
  
  text_layer_set_text(s_character_layer, character_buffer);
  text_layer_set_text(s_kana_layer, kana_buffer);
  text_layer_set_text(s_meaning_layer, meaning_buffer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded keys: Ram -> Buffer -> Screen");
  
  // Set background color equal to the card it is
  // This is a similar duplicate of inside "inbox_received_callback()"
  if(*type_buffer == 8) {
      window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorCadetBlue, GColorBlack) );
      text_layer_set_text(s_levtitle_layer, "Radical");} //radical 8
    else if(*type_buffer == 1) {
      window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorFolly, GColorBlack) );
      text_layer_set_text(s_levtitle_layer, "Kanji");} // kanji 1
    else if(*type_buffer == 3) {
      window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorImperialPurple, GColorBlack) );
      text_layer_set_text(s_levtitle_layer, "Vocabulary");// vocab 3
    } else {
      text_layer_set_text(s_levtitle_layer, "Kanji");
      window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorCadetBlue, GColorBlack) );
    }// Error colored
  
  
  
  
  
  
  
  
  
  
}
static void deinit() {
  window_destroy(s_main_window);// Destroy Window
  
  // Ask on forums if this is proper
  // Write to persist memory on close
  persist_write_string(CHARACTER_KEY, character_buffer);
  persist_write_string(KANA_KEY, kana_buffer);
  persist_write_string(MEANING_KEY, meaning_buffer);
  persist_write_string(TYPE_KEY, type_buffer);
  
}
int main(void) {
  init();
  app_event_loop();
  deinit();
}
//******************************************************************************
// File:        menus.c
// Author:      Everett Brostedt
// Date:        April 2, 2026
// Course:      ECE 306 - Introduction to Embedded Systems
// Assignment:  Homework 9 - Scrolling Menus
//
// Description:
//   This file implements three scrolling menus for the MSP430-based embedded
//   system. The menus are navigated via the ADC thumb wheel. Button 1 (SW1)
//   selects the currently highlighted menu item; Button 2 (SW2) returns to
//   the main menu.
//
//   Menu 1 (Main): Resistors | Shapes | Song
//   Menu 2 (Resistors): Scrolls through 10 resistor color codes (Table 1)
//   Menu 3 (Shapes): Scrolls through 10 shapes (Table 2) on big-mid display
//   Menu 4 (Song): Scrolls the "Red and White" lyrics character-by-character
//                  on the big-mid display, advancing only on CCW thumb wheel
//                  movement.
//
// Globals Modified:
//   display_line[][]: Updated with content for the LCD display
//   display_mode:     Controls which menu/display is currently active
//   number_of_presses, number_of_presses_2: Read to detect button events
//
// Notes:
//   - lcd_BIG_mid() must be called before shape/song display
//   - lcd_4line()   must be called when returning to 4-line mode
//   - ADC thumb wheel value is in ADC_Thumb (0-4095, 12-bit)
//******************************************************************************

#include <string.h>
#include "msp430.h"
#include "functions.h"
#include "ports.h"
#include "macros.h"
#include "globals.h"

//==============================================================================
// Menu State #defines
//==============================================================================
#define MENU_SPLASH         (0x00)   // Initial splash / homework screen
#define MENU_MAIN           (0x01)   // Top-level menu: Resistors/Shapes/Song
#define MENU_RESISTORS      (0x02)   // Resistor color code sub-menu
#define MENU_SHAPES         (0x03)   // Shapes sub-menu (big-mid display)
#define MENU_SONG           (0x04)   // Song scroll (big-mid display)

//==============================================================================
// ADC / Scroll #defines
//==============================================================================
#define ADC_MAX             (4095)   // Maximum 12-bit ADC value
#define MAIN_ITEMS          (3)      // Number of items in the main menu
#define MAIN_DIVISOR        (1366)   // ADC range per main item (4096 / 3)

#define RESISTOR_ITEMS      (10)     // 10 resistor color codes
#define RESISTOR_DIVISOR    (410)    // ADC range per resistor item (4096 / 10)

#define SHAPES_ITEMS        (10)     // 10 shapes
#define SHAPES_DIVISOR      (410)    // ADC range per shape item (4096 / 10)

// Song scrolling
// The song is scrolled in "chunks" using the thumb wheel direction.
// The ADC range is divided into SONG_ZONES bands. Each full sweep of the
// wheel (low -> high CCW) advances one character.  CW motion has no effect.
#define SONG_ZONES          (4)      // Number of ADC "comfort" zones per advance
#define SONG_ZONE_SIZE      (1024)   // 4096 / 4 zones

// Main-menu item indices
#define MAIN_RESISTORS      (0)
#define MAIN_SHAPES         (1)
#define MAIN_SONG           (2)

// LCD line constants (1-indexed matches lcd_out usage convention)
#define LCD_LINE1           (1)
#define LCD_LINE2           (2)
#define LCD_LINE3           (3)
#define LCD_LINE4           (4)

// LCD display width
#define LCD_WIDTH           (10)

// Button press detection
#define SW1_PRESSED         (1)
#define SW2_PRESSED         (2)
#define NO_PRESS            (0)

//==============================================================================
// Module-level (file-scope) state variables
//==============================================================================

// Current active menu state
static unsigned char menu_state = MENU_SPLASH;

// Tracks previously displayed indices to avoid redundant LCD writes
static int prev_main_idx      = -1;
static int prev_resistor_idx  = -1;
static int prev_shape_idx     = -1;

// Song scrolling state
static int  song_char_index   = 0;   // Current character position displayed
static int  song_prev_zone    = -1;  // Previous ADC zone (detect CCW advance)
static char song_scroll_buf[LCD_WIDTH + 1]; // 10-char wide scrolling window

//==============================================================================
// Constant Data Tables
//==============================================================================

// Table 1: Resistor color codes (value 0-9)
static const char * const resistor_color[RESISTOR_ITEMS] = {
    "Black",    // 0
    "Brown",    // 1
    "Red",      // 2
    "Orange",   // 3
    "Yellow",   // 4
    "Green",    // 5
    "Blue",     // 6
    "Violet",   // 7
    "Gray",     // 8
    "White"     // 9
};

static const char resistor_value[RESISTOR_ITEMS] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

// Table 2: Shape names
static const char * const shape_name[SHAPES_ITEMS] = {
    "Circle",
    "Square",
    "Triangle",
    "Octagon",
    "Pentagon",
    "Hexagon",
    "Cube",
    "Oval",
    "Sphere",
    "Cylinder"
};

// Table 3: Red and White song lyrics (continuous string for character scroll)
static const char song_lyrics[] =
    "We're the Red and White from State "
    "And we know we are the best. "
    "A hand behind our back, "
    "We can take on all the rest. "
    "Come over the hill, Carolina. "
    "Devils and Deacs stand in line. "
    "The Red and White from N.C. State. "
    "Go State!";

#define SONG_LENGTH  (sizeof(song_lyrics) - 1)   // length excluding '\0'

//==============================================================================
// Static helper prototypes (internal to this file)
//==============================================================================
static int  get_adc_index(unsigned int adc_val, int num_items, int divisor);
static void safe_lcd_string(char *dest, const char *src, int max_len);
static void display_splash(void);
static int  check_button(void);

//==============================================================================
// Function: display_splash
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Displays the initial "splash" screen: First name on Line 1,
//           Last name on Line 3, and "Homework 9" until a button is pressed.
//           Uses the standard 4-line display mode.
// Passed:   Nothing
// Returns:  Nothing
// Globals:  display_line[][] - written with splash content
//           display_mode_i   - triggers Display_Process to refresh LCD
//==============================================================================
static void display_splash(void) {
    lcd_4line();
    lcd_out("Everett   ", LCD_LINE1, 0);
    lcd_out("          ", LCD_LINE2, 0);
    lcd_out("Brostedt  ", LCD_LINE3, 0);
    lcd_out("HW9       ", LCD_LINE4, 0);
    enable_display_update();
}

//==============================================================================
// Function: check_button
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Checks whether SW1 or SW2 has been pressed since last call.
//           Consumes the press count so the same press is not processed twice.
// Passed:   Nothing
// Returns:  SW1_PRESSED (1), SW2_PRESSED (2), or NO_PRESS (0)
// Globals:  number_of_presses   - read and reset
//           number_of_presses_2 - read and reset
//==============================================================================
static int check_button(void) {
    if (number_of_presses > 0) {
        number_of_presses = 0;
        return SW1_PRESSED;
    }
    if (number_of_presses_2 > 0) {
        number_of_presses_2 = 0;
        return SW2_PRESSED;
    }
    return NO_PRESS;
}

//==============================================================================
// Function: get_adc_index
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Maps a 12-bit ADC value to a menu item index [0, num_items-1]
//           using even division of the ADC range.
// Passed:   adc_val  - current ADC thumb wheel reading (0-4095)
//           num_items - total number of menu items
//           divisor   - ADC range per item (ADC_MAX+1 / num_items)
// Returns:  Integer index in [0, num_items-1]
// Globals:  None
//==============================================================================
static int get_adc_index(unsigned int adc_val, int num_items, int divisor) {
    int idx = (int)(adc_val / divisor);
    if (idx >= num_items) {
        idx = num_items - 1;
    }
    return idx;
}

//==============================================================================
// Function: safe_lcd_string
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Copies src into dest, padding with spaces up to max_len, and
//           null-terminating. Prevents LCD garbage from short strings.
// Passed:   dest    - destination char buffer (must be >= max_len+1 bytes)
//           src     - source string
//           max_len - number of display characters (typically LCD_WIDTH = 10)
// Returns:  Nothing
// Globals:  None
//==============================================================================
static void safe_lcd_string(char *dest, const char *src, int max_len) {
    int i;
    int src_len = (int)strlen(src);
    for (i = 0; i < max_len; i++) {
        if (i < src_len) {
            dest[i] = src[i];
        } else {
            dest[i] = ' ';
        }
    }
    dest[max_len] = '\0';
}

//==============================================================================
// Function: Menu_Splash_Process
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Handles the initial splash screen state. Displays the student's
//           name and "Homework 9" until either button is pressed, at which
//           point the system transitions to the main menu.
// Passed:   Nothing
// Returns:  Nothing
// Globals:  menu_state       - updated to MENU_MAIN on button press
//           number_of_presses, number_of_presses_2 - consumed on press
//==============================================================================
void Menu_Splash_Process(void) {
    // Draw splash once (called every loop; safe because lcd_out is idempotent
    // when content does not change, but we track state to call only when needed)
    display_splash();

    if (check_button() != NO_PRESS) {
        // Any button press exits splash and enters main menu
        number_of_presses   = 0;   // clear both in case both fired
        number_of_presses_2 = 0;
        menu_state = MENU_MAIN;
        prev_main_idx = -1;        // force redraw on entry
        lcd_4line();
    }
}

//==============================================================================
// Function: Menu_Main_Process
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Handles the top-level scrolling menu with three items: Resistors,
//           Shapes, and Song. The ADC thumb wheel selects the highlighted item.
//           Button 1 (SW1) selects the current item and enters its sub-menu.
//           Button 2 (SW2) has no effect at the main menu level.
//           Display uses standard 4-line mode.
//             Line 1: "Main Menu"
//             Line 2: current selection (highlighted item)
//             Line 3: blank
//             Line 4: "SW1=Select"
// Passed:   Nothing
// Returns:  Nothing
// Globals:  menu_state       - updated to sub-menu on SW1 press
//           ADC_Thumb        - read for scroll position
//           display_line[][] - updated with current selection
//==============================================================================
void Menu_Main_Process(void) {
    int idx;
    int btn;
    char line_buf[LCD_WIDTH + 1];

    static const char * const main_labels[MAIN_ITEMS] = {
        "Resistors ",
        "Shapes    ",
        "Song      "
    };

    lcd_4line();

    idx = get_adc_index(ADC_Thumb, MAIN_ITEMS, MAIN_DIVISOR);

    btn = check_button();

    if (btn == SW1_PRESSED) {
        // Enter the selected sub-menu
        prev_resistor_idx = -1;
        prev_shape_idx    = -1;
        song_char_index   = 0;
        song_prev_zone    = -1;

        if (idx == MAIN_RESISTORS) {
            menu_state = MENU_RESISTORS;
        } else if (idx == MAIN_SHAPES) {
            menu_state = MENU_SHAPES;
            lcd_BIG_mid();
        } else if (idx == MAIN_SONG) {
            menu_state = MENU_SONG;
            lcd_BIG_mid();
        }
        return;
    }

    // Only redraw when the selection changes
    if (idx != prev_main_idx) {
        prev_main_idx = idx;

        safe_lcd_string(line_buf, "Main Menu ", LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE1, 0);

        safe_lcd_string(line_buf, main_labels[idx], LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE2, 0);

        safe_lcd_string(line_buf, "          ", LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE3, 0);

        safe_lcd_string(line_buf, "SW1=Select", LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE4, 0);

        enable_display_update();
    }
}

//==============================================================================
// Function: Menu_Resistors_Process
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Implements the Resistor Color Code scrolling sub-menu. The thumb
//           wheel selects one of 10 resistor colors (Table 1). The display
//           uses the 4-line mode:
//             Line 1: "Color"      (header)
//             Line 2: color name   (current selection)
//             Line 3: "Value"      (header)
//             Line 4: digit value  (0-9 corresponding to the color)
//           Button 1 (SW1): no effect (already in sub-menu)
//           Button 2 (SW2): returns to the main menu
//           The menu stops scrolling at the last color (index 9).
// Passed:   Nothing
// Returns:  Nothing
// Globals:  menu_state       - set to MENU_MAIN on SW2 press
//           ADC_Thumb        - read for scroll position
//           display_line[][] - updated with resistor data
//==============================================================================
void Menu_Resistors_Process(void) {
    int idx;
    int btn;
    char line_buf[LCD_WIDTH + 1];
    char val_str[LCD_WIDTH + 1];

    lcd_4line();

    idx = get_adc_index(ADC_Thumb, RESISTOR_ITEMS, RESISTOR_DIVISOR);

    btn = check_button();
    if (btn == SW2_PRESSED) {
        menu_state    = MENU_MAIN;
        prev_main_idx = -1;   // force main menu redraw
        lcd_4line();
        return;
    }

    if (idx != prev_resistor_idx) {
        prev_resistor_idx = idx;

        // Line 1: "Color" header
        safe_lcd_string(line_buf, "Color     ", LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE1, 0);

        // Line 2: Color name
        safe_lcd_string(line_buf, resistor_color[idx], LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE2, 0);

        // Line 3: "Value" header
        safe_lcd_string(line_buf, "Value     ", LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE3, 0);

        // Line 4: single digit value
        val_str[0] = resistor_value[idx];
        val_str[1] = '\0';
        safe_lcd_string(line_buf, val_str, LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE4, 0);

        enable_display_update();
    }
}

//==============================================================================
// Function: Menu_Shapes_Process
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Implements the Shapes scrolling sub-menu using the BIG middle line
//           display mode (lcd_BIG_mid). The thumb wheel selects one of 10
//           shapes (Table 2). The large (middle) line shows the current shape.
//           Line 1 shows the previous shape; Line 3 shows the next shape.
//           Blank lines guard the top and bottom ends so the first and last
//           shapes are always centered in the big line.
//           Button 2 (SW2): returns to the main menu (restores 4-line mode).
// Passed:   Nothing
// Returns:  Nothing
// Globals:  menu_state       - set to MENU_MAIN on SW2 press
//           ADC_Thumb        - read for scroll position
//           display_line[][] - updated with shape names
//==============================================================================
void Menu_Shapes_Process(void) {
    int idx;
    int btn;
    char line_buf[LCD_WIDTH + 1];

    btn = check_button();
    if (btn == SW2_PRESSED) {
        menu_state    = MENU_MAIN;
        prev_main_idx = -1;
        lcd_4line();
        return;
    }

    idx = get_adc_index(ADC_Thumb, SHAPES_ITEMS, SHAPES_DIVISOR);

    if (idx != prev_shape_idx) {
        prev_shape_idx = idx;

        // Line 1: previous shape (blank if at first item)
        if (idx > 0) {
            safe_lcd_string(line_buf, shape_name[idx - 1], LCD_WIDTH);
        } else {
            safe_lcd_string(line_buf, "          ", LCD_WIDTH);
        }
        lcd_out(line_buf, LCD_LINE1, 0);

        // Big middle line: current shape
        safe_lcd_string(line_buf, shape_name[idx], LCD_WIDTH);
        lcd_out(line_buf, LCD_LINE2, 0);

        // Line 3: next shape (blank if at last item)
        if (idx < SHAPES_ITEMS - 1) {
            safe_lcd_string(line_buf, shape_name[idx + 1], LCD_WIDTH);
        } else {
            safe_lcd_string(line_buf, "          ", LCD_WIDTH);
        }
        lcd_out(line_buf, LCD_LINE3, 0);

        enable_display_update();
    }
}

//==============================================================================
// Function: Menu_Song_Process
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Scrolls the "Red and White" song lyrics character by character on
//           the big middle line display (lcd_BIG_mid). Characters scroll in
//           from the right and off the left (marquee style).
//
//           The ADC range is divided into SONG_ZONES (4) comfort zones.
//           The song character advances ONLY when the thumb wheel moves
//           counter-clockwise (CCW), detected as the ADC zone index decreasing
//           from a higher zone back to zone 0. Moving CW (zone increasing from
//           0 upward) has no effect on the song position.
//
//           Line 1 alternates "Red&White " / "White&Red " and Line 3
//           alternates "White&Red " / "Red&White " as the song progresses.
//
//           Button 2 (SW2): returns to the main menu (restores 4-line mode).
// Passed:   Nothing
// Returns:  Nothing
// Globals:  menu_state       - set to MENU_MAIN on SW2 press
//           ADC_Thumb        - read for zone detection
//           song_char_index  - tracks current character position in lyrics
//           song_prev_zone   - tracks last ADC zone to detect CCW motion
//           display_line[][] - updated with song content
//==============================================================================
void Menu_Song_Process(void) {
    int  btn;
    int  current_zone;
    int  i;
    char line_buf[LCD_WIDTH + 1];
    int  buf_pos;
    int  char_pos;

    // Alternating header strings for Line 1 / Line 3
    static const char * const alt_a[2] = { "Red&White ", "White&Red " };
    static const char * const alt_b[2] = { "White&Red ", "Red&White " };
    int alt_idx;

    btn = check_button();
    if (btn == SW2_PRESSED) {
        menu_state    = MENU_MAIN;
        prev_main_idx = -1;
        song_char_index = 0;
        song_prev_zone  = -1;
        lcd_4line();
        return;
    }

    // Determine current ADC zone (0 = low/CCW end, SONG_ZONES-1 = high/CW end)
    current_zone = (int)(ADC_Thumb / SONG_ZONE_SIZE);
    if (current_zone >= SONG_ZONES) {
        current_zone = SONG_ZONES - 1;
    }

    // Advance song character only when returning to zone 0 from a higher zone
    // (i.e., the user swept CCW through the full range and is back at the bottom)
    if (song_prev_zone > 0 && current_zone == 0) {
        if (song_char_index < (int)SONG_LENGTH) {
            song_char_index++;
        }
    }
    song_prev_zone = current_zone;

    // Build the 10-character scrolling window.
    // Characters enter from the right: show the LCD_WIDTH chars ending at
    // song_char_index (right-justified into the window, padded with spaces).
    for (i = 0; i < LCD_WIDTH; i++) {
        buf_pos  = i;
        char_pos = song_char_index - (LCD_WIDTH - 1) + i;

        if (char_pos < 0 || char_pos >= (int)SONG_LENGTH) {
            song_scroll_buf[buf_pos] = ' ';
        } else {
            song_scroll_buf[buf_pos] = song_lyrics[char_pos];
        }
    }
    song_scroll_buf[LCD_WIDTH] = '\0';

    // Alternating header/footer swap every character advance
    alt_idx = song_char_index & 0x01;   // 0 or 1

    // Line 1: alternating "Red&White" / "White&Red"
    safe_lcd_string(line_buf, alt_a[alt_idx], LCD_WIDTH);
    lcd_out(line_buf, LCD_LINE1, 0);

    // Big middle line: current song window
    lcd_out(song_scroll_buf, LCD_LINE2, 0);

    // Line 3: alternating complement of Line 1
    safe_lcd_string(line_buf, alt_b[alt_idx], LCD_WIDTH);
    lcd_out(line_buf, LCD_LINE3, 0);

    enable_display_update();
}

//==============================================================================
// Function: Menu_Process
// Author:   Everett Brostedt
// Date:     April 2, 2026
// Purpose:  Top-level menu dispatcher. Called from the main while loop every
//           iteration. Routes execution to the correct sub-menu handler based
//           on the current menu_state. This is the single entry point for all
//           menu logic and satisfies the Menu_Process() prototype in functions.h.
// Passed:   Nothing
// Returns:  Nothing
// Globals:  menu_state - read to dispatch; may be modified by sub-functions
//==============================================================================
void Menu_Process(void) {
    switch (menu_state) {
        case MENU_SPLASH:
            Menu_Splash_Process();
            break;

        case MENU_MAIN:
            Menu_Main_Process();
            break;

        case MENU_RESISTORS:
            Menu_Resistors_Process();
            break;

        case MENU_SHAPES:
            Menu_Shapes_Process();
            break;

        case MENU_SONG:
            Menu_Song_Process();
            break;

        default:
            menu_state = MENU_SPLASH;
            break;
    }
}

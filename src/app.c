#include "app.h"
#include <string.h>

#define BUTTON_COUNT 100
#define CLOCK_INDEX 10
#define AND_INDEX 91
#define OR_INDEX 92
#define XOR_INDEX 93
#define NOT_INDEX 94
#define NAND_INDEX 95
#define NOR_INDEX 96

enum CELL_TYPE
{
  EMPTY = 0,
  INPUT = 1,
  GATE = 2,
};

enum GATE_TYPE
{
  AND = 0,
  OR = 1,
  XOR = 2,
  NOT = 3,
  NAND = 4,
  NOR = 5,
};

struct cell
{
  enum CELL_TYPE type;
  u8 input_index;
  enum GATE_TYPE gate_type;
  u8 gate_input_index_a;
  u8 gate_input_index_b;
  u8 output;
};

struct cell display_state[BUTTON_COUNT] = {0};
struct cell temp_state[BUTTON_COUNT] = {0};
u8 held_button_index = 0;

//______________________________________________________________________________

// This function is kindof insane.
// It just checks if the supplied index is one of the allowed side buttons
// For the user input, clocks, gates, and output routing.
// This could be compacted but it works fine and I don't feel like changing it. c:
u8 is_input_button(u8 index)
{
  return (index > 90 && index < 97) || (index > 0 && index < 11) || index == 19 || index == 29 || index == 39 || index == 49 || index == 59 || index == 69 || index == 79 || index == 89;
}

// user input buttons range from 1 to 8
// These are the bottom row of buttons
u8 is_user_input_button(u8 index)
{
  return index > 0 && index < 9;
}

void run_simulation()
{
  for (u8 y = 1; y < 9; y++)
  {
    u8 last_row_value = 0;
    for (u8 x = 1; x < 9; x++)
    {
      u8 index = x + y * 10;
      if (display_state[index].type == INPUT)
      {
        temp_state[index].output = display_state[display_state[index].input_index].output;
        last_row_value = temp_state[index].output;
      }
      else if (display_state[index].type == GATE)
      {
        u8 a = display_state[display_state[index].gate_input_index_a].output;
        u8 b = display_state[display_state[index].gate_input_index_b].output;
        switch (display_state[index].gate_type)
        {
        case AND:
          temp_state[index].output = a && b;
          break;
        case OR:
          temp_state[index].output = a || b;
          break;
        case XOR:
          temp_state[index].output = a ^ b;
          break;
        case NOT:
          temp_state[index].output = !a;
          break;
        case NAND:
          temp_state[index].output = !(a && b);
          break;
        case NOR:
          temp_state[index].output = !(a || b);
          break;
        }
        last_row_value = temp_state[index].output;
        continue;
      }
      else if (display_state[index].type == EMPTY)
      {
        temp_state[index].output = 0;
      }
    }
    temp_state[y * 10 + 9].output = last_row_value;
  }
}

void display_current_state()
{
  for (u8 y = 1; y < 9; y++)
  {
    for (u8 x = 1; x < 9; x++)
    {
      u8 index = x + y * 10;

      if (display_state[index].type == GATE)
      {
        switch (display_state[index].gate_type)
        {
        case AND:
          hal_plot_led(TYPEPAD, index, MAXLED, 0, MAXLED);
          break;
        case OR:
          hal_plot_led(TYPEPAD, index, 0, MAXLED, 0);
          break;
        case XOR:
          hal_plot_led(TYPEPAD, index, MAXLED, 0, 0);
          break;
        case NOT:
          hal_plot_led(TYPEPAD, index, 0, MAXLED, MAXLED);
          break;
        case NAND:
          hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, 0);
          break;
        case NOR:
          hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, MAXLED);
          break;
        }
        continue;
      }
      hal_plot_led(TYPEPAD, index, 0, 0, display_state[index].output * MAXLED);
    }
    hal_plot_led(TYPEPAD, y * 10 + 9, 0, 0, display_state[y * 10 + 9].output * MAXLED);
  }
  hal_plot_led(TYPEPAD, 10, 0, 0, display_state[10].output * MAXLED);
}

void copy_temp_to_display()
{
  memcpy(display_state, temp_state, sizeof(display_state));
}

void full_update()
{
  run_simulation();
  copy_temp_to_display();
  display_current_state();
}

void app_surface_event(u8 type, u8 index, u8 value)
{
  switch (type)
  {
  case TYPEPAD:
  {
    if (is_input_button(index))
    {
      if (value)
      {
        held_button_index = index;
      }
      else
      {
        held_button_index = 0;
      }
      if (is_user_input_button(index))
      {
        // display_state[index].type = INPUT;
        // display_state[index].input_index = index;
        temp_state[index].output = value ? 1 : 0;
      }
      copy_temp_to_display();

      return;
    }
    // anding with value ensures this only triggers on a down press
    if (held_button_index && value)
    {
      if (display_state[index].input_index == held_button_index)
      {
        temp_state[index].input_index = 0;
        temp_state[index].type = EMPTY;
        hal_plot_led(TYPEPAD, index, MAXLED, 0, 0);
      }
      else
      {
        switch (held_button_index)
        {

        case AND_INDEX:
        {
          temp_state[index].input_index = held_button_index;
          temp_state[index].gate_type = AND;
          temp_state[index].type = GATE;
          temp_state[index].gate_input_index_a = index - 1;
          temp_state[index].gate_input_index_b = index - 2;
          hal_plot_led(TYPEPAD, index, 0, 0, MAXLED);
          break;
        }
        case OR_INDEX:
        {
          temp_state[index].input_index = held_button_index;
          temp_state[index].gate_type = OR;
          temp_state[index].type = GATE;
          temp_state[index].gate_input_index_a = index - 1;
          temp_state[index].gate_input_index_b = index - 2;
          hal_plot_led(TYPEPAD, index, 0, MAXLED, 0);
          break;
        }
        case XOR_INDEX:
        {
          temp_state[index].input_index = held_button_index;
          temp_state[index].gate_type = XOR;
          temp_state[index].type = GATE;
          temp_state[index].gate_input_index_a = index - 1;
          temp_state[index].gate_input_index_b = index - 2;
          hal_plot_led(TYPEPAD, index, MAXLED, 0, 0);
          break;
        }
        case NOT_INDEX:
        {
          temp_state[index].input_index = held_button_index;
          temp_state[index].gate_type = NOT;
          temp_state[index].type = GATE;
          temp_state[index].gate_input_index_a = index - 1;
          hal_plot_led(TYPEPAD, index, 0, 0, MAXLED);
          break;
        }
        case NAND_INDEX:
        {
          temp_state[index].input_index = held_button_index;
          temp_state[index].gate_type = NAND;
          temp_state[index].type = GATE;
          temp_state[index].gate_input_index_a = index - 1;
          temp_state[index].gate_input_index_b = index - 2;
          hal_plot_led(TYPEPAD, index, MAXLED / 2, 0, MAXLED / 2);
          break;
        }
        case NOR_INDEX:
        {
          temp_state[index].input_index = held_button_index;
          temp_state[index].gate_type = NOR;
          temp_state[index].type = GATE;
          temp_state[index].gate_input_index_a = index - 1;
          temp_state[index].gate_input_index_b = index - 2;
          hal_plot_led(TYPEPAD, index, 0, MAXLED / 2, 0);
          break;
        }
        default:
        {
          temp_state[index].input_index = held_button_index;
          temp_state[index].type = INPUT;
          hal_plot_led(TYPEPAD, index, 0, MAXLED, 0);
          break;
        }
        }
      }
    }
    break;
  }
  }
}

//______________________________________________________________________________

void app_midi_event(u8 port, u8 status, u8 d1, u8 d2) {}

//______________________________________________________________________________

void app_sysex_event(u8 port, u8 *data, u16 count) {}

//______________________________________________________________________________

void app_aftertouch_event(u8 index, u8 value) {}

//______________________________________________________________________________

void app_cable_event(u8 type, u8 value) {}

//______________________________________________________________________________

void app_timer_event()
{
#define TICK_MS 500

  static u16 ms = TICK_MS;
  if (++ms >= TICK_MS)
  {
    ms = 0;
    display_state[10].output = 1;
  }
  full_update();
  display_state[10].output = 0;
  // for (u8 y = 1; y < 9; y++)
  // {
  //   u8 last_row_value = 0;
  //   for (u8 x = 1; x < 9; x++)
  //   {
  //     u8 index = x + y * 10;
  //     if (display_state[index].type == INPUT)
  //     {
  //       temp_state[index].output = display_state[display_state[index].input_index].output;
  //       last_row_value = temp_state[index].output;
  //     }
  //     else if (display_state[index].type == GATE)
  //     {
  //       u8 a = display_state[display_state[index].gate_input_index_a].output;
  //       u8 b = display_state[display_state[index].gate_input_index_b].output;
  //       switch (display_state[index].gate_type)
  //       {
  //       case AND:
  //         temp_state[index].output = a && b;
  //         hal_plot_led(TYPEPAD, index, MAXLED, 0, MAXLED);
  //         break;
  //       case OR:
  //         temp_state[index].output = a || b;
  //         hal_plot_led(TYPEPAD, index, 0, MAXLED, 0);
  //         break;
  //       case XOR:
  //         temp_state[index].output = a ^ b;
  //         hal_plot_led(TYPEPAD, index, MAXLED, 0, 0);
  //         break;
  //       case NOT:
  //         temp_state[index].output = !a;
  //         hal_plot_led(TYPEPAD, index, 0, MAXLED, MAXLED);
  //         break;
  //       case NAND:
  //         temp_state[index].output = !(a && b);
  //         hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, 0);
  //         break;
  //       case NOR:
  //         temp_state[index].output = !(a || b);
  //         hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, MAXLED);
  //         break;
  //       }
  //       last_row_value = temp_state[index].output;
  //       continue;
  //     }
  //     else if (display_state[index].type == EMPTY)
  //     {
  //       temp_state[index].output = 0;
  //     }
  //     hal_plot_led(TYPEPAD, index, 0, 0, display_state[index].output * MAXLED);
  //   }
  //   temp_state[y * 10 + 9].output = last_row_value;
  //   hal_plot_led(TYPEPAD, y * 10 + 9, 0, 0, display_state[y * 10 + 9].output * MAXLED);
  // }
  // hal_plot_led(TYPEPAD, 10, 0, 0, display_state[10].output * MAXLED);
}

//______________________________________________________________________________

void app_init(const u16 *adc_raw)
{
  // example - load button statess from flash
  // hal_read_flash(0, g_Buttons, BsUTTON_COUNT);

  hal_plot_led(TYPEPAD, 91, MAXLED, 0, MAXLED);
  hal_plot_led(TYPEPAD, 92, 0, MAXLED, 0);
  hal_plot_led(TYPEPAD, 93, MAXLED, 0, 0);
  hal_plot_led(TYPEPAD, 94, 0, MAXLED, MAXLED);
  hal_plot_led(TYPEPAD, 95, MAXLED, MAXLED, 0);
  hal_plot_led(TYPEPAD, 96, MAXLED, MAXLED, MAXLED);

  // example - light the LEDs to say hello !

  // for (int i = 1; i < 9; ++i)
  // {
  //   for (int j = 1; j < 9; ++j)
  //   {
  //     // u8 b = g_Buttons[j * 10 + i];

  //     temp_state[j * 10 + i] = does_cell_live(i, j);
  //     hal_plot_led(TYPEPAD, j * 10 + i, 0, 0, temp_state[j * 10 + i] * MAXLED);
  //   }
  // }

  // memcpy(display_state, temp_state, sizeof(display_state));

  // store off the raw ADC frame pointer for later use
  // g_ADC = adc_raw;
}

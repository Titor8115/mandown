#include "cmd.h"

#include "st_curses.h"

mdn_command less_cmd_scheme(int key)
{
  switch (key) {
    case 'q': {
      return CMD_EXIT;
    }
    case TAB: {
      return CMD_SELECT_HREF;
    }
    case ENTER: {
      return CMD_SELECT_COMFIRM;
    }
    case 'e':
    case 'j':
    case KEY_DOWN: {
      return CMD_GOTO_DOWN;
    }
    case 'y':
    case 'k':
    case KEY_UP: {
      return CMD_GOTO_UP;
    }
    case 'f':
    case ' ': 
    case KEY_NPAGE: {
      return CMD_GOTO_NPAGE;
    }
    case 'b':
    case KEY_PPAGE: {
      return CMD_GOTO_PPAGE;
    }
    case 'g':
    case KEY_HOME: {
      return CMD_GOTO_TOF;
    }
    case 'G':
    case KEY_END: {
      return CMD_GOTO_EOF;
    }
    case KEY_RESIZE: {
      return CMD_RESIZE_EVENT;
    }
    case KEY_MOUSE: {
      return CMD_MOUSE_EVENT;
    }
    case ERR:
    default: {
      return CMD_ERR;
    }
  }
}

mdn_command mdn_cmd_scheme(int key)
{
  switch (key) {
    case 'q': {
      return CMD_EXIT;
    }
    case TAB: {
      return CMD_SELECT_HREF;
    }
    case ENTER: {
      return CMD_SELECT_COMFIRM;
    }
    case 's':
    case KEY_DOWN: {
      return CMD_GOTO_DOWN;
    }
    case 'w':
    case KEY_UP: {
      return CMD_GOTO_UP;
    }
    case 'S':
    case KEY_NPAGE: {
      return CMD_GOTO_NPAGE;
    }
    case 'W':
    case KEY_PPAGE: {
      return CMD_GOTO_PPAGE;
    }
    case 'g':
    case KEY_HOME: {
      return CMD_GOTO_TOF;
    }
    case 'G':
    case KEY_END: {
      return CMD_GOTO_EOF;
    }
    case KEY_RESIZE: {
      return CMD_RESIZE_EVENT;
    }
    case KEY_MOUSE: {
      return CMD_MOUSE_EVENT;
    }
    case ERR:
    default: {
      return CMD_ERR;
    }
  }
}

mdn_command vim_cmd_scheme(int key)
{
  switch (key) {
    case 'q': {
      return CMD_EXIT;
    }
    case TAB: {
      return CMD_SELECT_HREF;
    }
    case ENTER: {
      return CMD_SELECT_COMFIRM;
    }
    default: {
      return CMD_ERR;
    }
  }
}

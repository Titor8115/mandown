#ifndef MDN_CMD_H
#define MDN_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CMD_ERR = -1,
  CMD_OK,
  CMD_EXIT,
  CMD_RESIZE_EVENT,
  CMD_MOUSE_EVENT,

  CMD_SELECT_COMFIRM = 100,
  CMD_SELECT_HREF,
  CMD_GOTO_DOWN,
  CMD_GOTO_UP,
  CMD_GOTO_NPAGE,
  CMD_GOTO_PPAGE,
  CMD_GOTO_TOF,
  CMD_GOTO_EOF,

} mdn_command;

mdn_command less_cmd_scheme(int);
mdn_command mdn_cmd_scheme(int);
mdn_command vim_cmd_scheme(int);

#ifdef __cplusplus
}
#endif

#endif /* MDN_CMD_H */

#ifndef GUI_H_
#define GUI_H_

#include "gesm.h"
/* graphical user interface */
void main_gui(int argc, char *argv[], enum device dev, char *dir);

typedef void (* handle_cmd)(void);

#endif /*GUI_H_*/

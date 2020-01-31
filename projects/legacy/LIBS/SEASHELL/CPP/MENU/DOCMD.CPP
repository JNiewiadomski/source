#include "menu\_menu.h"

#include <seashell\seashell.rh>
#include <seashell\error.h>

void MenuDoCommand(short id, short item) {
	if (id == _MenuErrorMENU)
		MenuErrorCommand(item);
	else if (menuCallBack)
		(*menuCallBack)(id, item);
	else
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_NoCallBack), id, item);
	}

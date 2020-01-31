#include <stdio.h>

#include <common\common.h>
#include <common\old\coord.h>

#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\form.h>

static ResForm	*r_form;
static short	formCount=0, formId;
static char	*formStr;


char *FormGetFirst(short id) {
	if ((r_form = (ResForm *) ResLoad(R_Form, id)) == NULL)
		return (NULL);
	formId = id;
	formCount = r_form->num;
	formStr = r_form->form;
	return (FormGetNext());
	}


char *FormGetNext(void) {
	char 	*s;

	if (formCount == 0)
		return (NULL);
	s = formStr;
	while (*formStr++)
		;
	formCount--;
	if (formCount == 0)
		ResUnLoad(R_Form, formId);
	return (s);
	}

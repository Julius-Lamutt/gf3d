#ifndef __ELEMENT_BUTTON_H__
#define __ELEMENT_BUTTON_H__

#include "element_label.h"
#include "element_actor.h"

typedef enum
{
	BT_HIDDEN,
	BT_LABEL,
	BT_ACTOR,
	BT_BOTH
} ButtonType;

typedef struct
{
	ButtonType      type;			/*<type of button*/
	LabelElement	*label;			/*<button label*/
	ActorElement	*actor;			/*<button actor*/
	GFC_Color		highColor;		/*<color when highlighted*/
	GFC_Color		pressColor;		/*<color when pressed*/
} ButtonElement;

/*
* @brief load button element
* @param windel: the window element to load the button element from
* @return NULL on error, a pointer to an button element otherwise
*/
ButtonElement *element_button_load(SJson *windel);

/*
* @brief draws a button element
* @param button: the button element to draw
* @param element: a pointer to basic element data
*/
void element_button_draw(ButtonElement *button, Element *element);

/*
* @brief free a button element from memory
* @param label: the button element to be freed
*/
void element_button_free(ButtonElement *button);

#endif


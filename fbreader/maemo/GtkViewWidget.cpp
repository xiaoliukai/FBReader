/*
 * FBReader -- electronic book reader
 * Copyright (C) 2005 Nikolay Pultsin <geometer@mawhrin.net>
 * Copyright (C) 2005 Mikhail Sobolev <mss@mawhrin.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

// #define ROTATION_TIMING

#ifdef ROTATION_TIMING
#include <iostream>

#include <abstract/ZLTime.h>
#endif

#include "GtkViewWidget.h"
#include "GtkFBReader.h"
#include "GtkPaintContext.h"

#include "gdk-pixbuf-hack.h"

static void mousePressed(GtkWidget*, GdkEventButton *event, gpointer data) {
	GtkViewWidget *viewWidget = (GtkViewWidget*)data;
	View *view = viewWidget->view();
	if (viewWidget->isRotated()) {
		view->onStylusPress(
			viewWidget->height() - (int)event->y - view->context().rightMargin().value(),
			(int)event->x - view->context().topMargin().value());
	} else {
		view->onStylusPress(
			(int)event->x - view->context().leftMargin().value(),
			(int)event->y - view->context().topMargin().value());
	}
}

int GtkViewWidget::height() const {
	return (myArea != 0) ? myArea->allocation.height : 0;
}

GtkViewWidget::GtkViewWidget(GtkFBReader *reader) {
	myReader = reader;
	myArea = gtk_drawing_area_new();
	myOriginalPixbuf = 0;
	gtk_widget_set_double_buffered(myArea, false);
	gtk_widget_set_events(myArea, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(myArea), "button_press_event", GTK_SIGNAL_FUNC(mousePressed), this);
}

GtkViewWidget::~GtkViewWidget() {
	if (myOriginalPixbuf != 0) {
		gdk_pixbuf_unref(myOriginalPixbuf);
		gdk_pixbuf_unref(myRotatedPixbuf);
	}
}

void GtkViewWidget::repaintView()	{
	GtkPaintContext &gtkContext = (GtkPaintContext&)view()->context();
	int w = isRotated() ? myArea->allocation.height : myArea->allocation.width;
	int h = isRotated() ? myArea->allocation.width : myArea->allocation.height;
	gtkContext.updatePixmap(myArea, w, h);
	view()->paint();
	if (isRotated()) {
		if ((myOriginalPixbuf != 0) && ((gdk_pixbuf_get_width(myOriginalPixbuf) != w) || (gdk_pixbuf_get_height(myOriginalPixbuf) != h))) {
			gdk_pixbuf_unref(myOriginalPixbuf);
			gdk_pixbuf_unref(myRotatedPixbuf);
			myOriginalPixbuf = 0;
		}
		if (myOriginalPixbuf == 0) {
			myOriginalPixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, w, h);
			myRotatedPixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, h, w);
		}
#ifdef ROTATION_TIMING
    ZLTime c1;
#endif
		gdk_pixbuf_get_from_drawable(myOriginalPixbuf, gtkContext.pixmap(), gdk_drawable_get_colormap(gtkContext.pixmap()), 0, 0, 0, 0, w, h);
#ifdef ROTATION_TIMING
    ZLTime c2;
#endif
		::rotate(myRotatedPixbuf, myOriginalPixbuf);
#ifdef ROTATION_TIMING
    ZLTime c3;
#endif
		gdk_draw_pixbuf(myArea->window, myArea->style->white_gc, myRotatedPixbuf, 0, 0, 0, 0, h, w, GDK_RGB_DITHER_NONE, 0, 0);
#ifdef ROTATION_TIMING
    ZLTime c4;

    std::cout << "Rotate: " << c2.millisecondsFrom(c1) << ", " << c3.millisecondsFrom(c2) << ", " << c4.millisecondsFrom(c3) << ", " << c2.millisecondsFrom(c1)+c3.millisecondsFrom(c2)+c4.millisecondsFrom(c3) << std::endl;
#endif
	} else {
		if (myOriginalPixbuf != 0) {
			gdk_pixbuf_unref(myOriginalPixbuf);
			gdk_pixbuf_unref(myRotatedPixbuf);
			myOriginalPixbuf = 0;
		}
		gdk_draw_pixmap(myArea->window, myArea->style->white_gc, gtkContext.pixmap(), 0, 0, 0, 0, myArea->allocation.width, myArea->allocation.height);
	}
	
	myReader->enableMenuButtons();
}

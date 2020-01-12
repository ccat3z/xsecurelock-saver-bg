#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

Display *display;
int screen;

Pixmap get_root_pixmap(Display* display, Window *root) {
    Pixmap currentRootPixmap;
    Atom act_type;
    int act_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;
    Atom _XROOTPMAP_ID;

    _XROOTPMAP_ID = XInternAtom(display, "_XROOTPMAP_ID", False);

    if (XGetWindowProperty(display, *root, _XROOTPMAP_ID, 0, 1, False,
                XA_PIXMAP, &act_type, &act_format, &nitems, &bytes_after,
                &data) == Success) {

        if (data) {
            currentRootPixmap = *((Pixmap *) data);
            XFree(data);
            return currentRootPixmap;
        }
    }
    return -1;
}

Window get_saver_window() {
    static Window saver = -1;
    if (saver != -1) return saver;

    char *id_s = getenv("XSCREENSAVER_WINDOW");
    if (id_s == NULL) {
        int screen = DefaultScreen(display);
        saver = XCreateSimpleWindow(
            display, RootWindow(display, screen),
            10, 10, 100, 100, 1,
            BlackPixel(display, screen), WhitePixel(display, screen)
        );
        XMapWindow(display, saver);
    } else {
        saver = atoi(id_s);
    }
    return saver;
}

void fresh_saver_window() {
    Window root = RootWindow(display, screen);
    Window saver = get_saver_window();

    XWindowAttributes saver_attrs;
    XGetWindowAttributes(display, saver, &saver_attrs);

    int x = saver_attrs.x, y = saver_attrs.y;
    int w = saver_attrs.width, h = saver_attrs.height;

    Pixmap root_bg = get_root_pixmap(display, &root);
    if (root_bg == -1) {
        fprintf(stderr, "cannot get root bg");
        return;
    }
    Pixmap saver_bg = XCreatePixmap(display, saver, w, h, 24);

    XCopyArea(
        display,
        root_bg, saver_bg,
        XCreateGC(display, saver, 0, NULL),
        x, y,
        w, h,
        0, 0
    );
    XSetWindowBackgroundPixmap(display, saver, saver_bg);
    XFreePixmap(display, saver_bg);

    XClearWindow(display, saver);
    XFlush(display);
    XSync(display, 1);
}

void sig_handler(int sig) {
    if (sig == SIGUSR1) {
        fresh_saver_window();
    }
}

int main(int argc, const char *argv[])
{
    display = XOpenDisplay(NULL);
    screen = DefaultScreen(display);

    if (display == NULL) {
        fprintf(stderr, "cannot connect to X server.\n");
        return 1;
    }

    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        fprintf(stderr, "cannot catch SIGUSR1.\n");
        return 1;
    }

    if (signal(SIGTERM, sig_handler) == SIG_ERR) {
        fprintf(stderr, "cannot catch SIGTERM.\n");
        return 1;
    }

    fresh_saver_window();

    for (;;) {
        pause();
    }

    // never
    return 1;
}

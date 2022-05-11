// From: https://bbs.archlinux.org/viewtopic.php?id=226665


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

//gcc -g enigo3.c -lXtst -lX11

int main(int argc, char *argv[])
{
    Display *dpy;
    dpy = XOpenDisplay(NULL);

    KeySym *keysyms = NULL;
    int keysyms_per_keycode = 0;
    int scratch_keycode = 0; // Scratch space for temporary keycode bindings
    int keycode_low, keycode_high;
    //get the range of keycodes usually from 8 - 255
    XDisplayKeycodes(dpy, &keycode_low, &keycode_high);
    //get all the mapped keysyms available
    keysyms = XGetKeyboardMapping(
        dpy,
        keycode_low,
        keycode_high - keycode_low,
        &keysyms_per_keycode);

    //find unused keycode for unmapped keysyms so we can
    //hook up our own keycode and map every keysym on it
    //so we just need to 'click' our once unmapped keycode
    int i;
    for (i = keycode_low; i <= keycode_high; i++)
    {
        int j = 0;
        int key_is_empty = 1;
        for (j = 0; j < keysyms_per_keycode; j++)
        {
            int symindex = (i - keycode_low) * keysyms_per_keycode + j;
            // test for debugging to looking at those value
            // KeySym sym_at_index = keysyms[symindex];
            // char *symname;
            // symname = XKeysymToString(keysyms[symindex]);

            if (keysyms[symindex] != 0)
            {
                key_is_empty = 0;
            }
            else
            {
                break;
            }
        }
        if (key_is_empty)
        {
            scratch_keycode = i;
            fprintf( stdout, "Free keycode: %d\n", i );
            break;
        }
    }
    XFree(keysyms);
    XFlush(dpy);

    usleep(200 * 1000);
    
    //find the keysym for the given unicode char
    //map that keysym to our previous unmapped keycode
    //click that keycode/'button' with our keysym on it

    //feed various glyphs into it 
    //"U007A" z "U002a" * "U0021" ! "U0026" & "U00e4" ä "U00c4" Ä "U1f4a3" Bombglyph etc.
    
    //KeySym sym = XStringToKeysym("U005A"); //uppercase Z
    //KeySym sym = XStringToKeysym("U10450");
    KeySym sym = XStringToKeysym("U1047f");


    KeySym keysym_list[] = {sym};
    XChangeKeyboardMapping(dpy, scratch_keycode, 1, keysym_list, 1);
    KeyCode code = scratch_keycode;

    usleep(90 * 1000);
    XTestFakeKeyEvent(dpy, code, True, 0);
    XFlush(dpy);

    usleep(90 * 1000);
    XTestFakeKeyEvent(dpy, code, False, 0);
    XFlush(dpy);

    //revert scratch keycode
    {
        KeySym keysym_list[] = {0};
        XChangeKeyboardMapping(dpy, scratch_keycode, 1, keysym_list, 1);
    }

    usleep(100 * 1000);

    XCloseDisplay(dpy);

    return 0;
}

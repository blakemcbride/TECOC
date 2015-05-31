
TECOC - TECO Editor
===================

Teco is a text editor written by Dan Murphy in 1962.  It differs from
other editors in several ways as follows:

### Character oriented

Vi and Emacs are screen oriented editors.  You view the text on the whole
window or screen, and you edit the text right where you see it.  Changes take
affect immediately.

ed, ex, edlin are line oriented editors.  Edited text is viewed,
referenced, and edited by line.

Teco is character oriented.  This means the unit of reference is a
single character.  On one hand, this may make teco cumbersome to use,
but on the other hand, it gives teco very fine control over what is
happening.

Teco does have a video mode, however text is still edited with
commands on a character basis.  The screen is simply updated once a
command is completed to give the user a visual representation of where
they are.  Text is not edited in the screen view area.

### Macros

In general, vi, ed, and edlin are not programmable.  This means you
have the functions the editor provides and that is all you can do.
However, teco is highly programmable.  Programs that you write are
called macros.  In fact, the original version of Emacs was written as
teco macros.  Also, "emacs" actually means "Editing Macros".

### TECOC

The original versions of Teco were written in the machine languages of
the machines they operated on.  As newer machines and editors arose, teco
fell into increasing disuse and obscurity.

In the early '80's Pete Siemsen, a long-time teco user, created a
portable version of Teco in the C language called "tecoc".  Rather
than create a new editor, Pete's goal was to duplicate the original teco
as closely as possible.

As time wore on, Tom Almy, Blake McBride, and others increasingly
assisted in tecoc's maintenance (porting, bug fixes, etc.).
Eventually, tecoc found a new home and principal support at Tom
Almy's home page where Tom kept up with ports to Windows, Mac, and
Linux.

Most recently, Blake McBride:

1. Merged Tom Almy's various ports along with work done by Blake
2. renewed the Windows, Mac, and Linux ports
3. re-enabled video support (on Mac & Linux)
4. bug fixes and speed improvements
5. tested on 64 bit machines

This latest work is available on github at:

https://github.com/blakemcbride/TECOC

Other documents in the "doc" directory provide complete documentation.
See the following files in the "doc" directory:

File | Description
:------- | :----------------
BUILD.txt | Build and setup instructions
INTRO.txt | Introduction to using tecoc
teco-manual.txt | Complete manual

echo GNOME Library must be installed for this software package to work. This includes:
echo gtk+2.0
echo glib
echo libglade
echo gmodule
echo
echo You can get the packages from:
echo 	http://www.gtk.org/download/index.php
echo 	http://developer.gnome.org/
echo
cc main.c -o gkr $(pkg-config --cflags=0 --libs gtk+-2.0 gmodule-2.0)
./gkr

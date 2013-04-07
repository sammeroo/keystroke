echo Extracting glib...
tar -Jxf glib-2.30.3.tar.xz
echo Success.
echo Extracting atk...
tar -Jxf atk-2.1.92.tar.xz
echo Success.
echo Extracting glade...
tar -Jxf glade-3.10.2.tar.xz
echo Success.
echo Extracting gtk+...
tar -Jxf gtk+-3.2.4.tar.xz
echo Success.
echo Extracting gdkpixbuf...
tar -Jxf gdk-pixbuf-2.25.0.tar.xz
echo Success.
tar -zvvf libglade-html-2.6.4.tar.gz
tar -zxvf pango-1.28.4.tar.gz
cd glib-2.30.3
./configure
make
sudo rm -rf /install-prefix/include/glib.h /install-prefix/include/gmodule.h
sudo make install
cd ..
cd gdk-pixbuf-2.25.0
./configure
make
sudo make install
cd ..
cd atk-2.1.92
./configure
make
sudo make install
cd ..
cd gtk+-3.2.4
./configure
make
sudo make install
cd ..
cd glade-3.10.2
./configure
make
sudo make install
cd ..
cd pango-1.28.4
./configure
make
sudo make install

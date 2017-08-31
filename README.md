# watchman

A configurable inode-watcher based on `inotify`


    apt install libnotify-dev libglib2.0-dev libgdk-pixbuf2.0-dev libyaml-dev

Because of an error when compiling with `libglib2` relating to the `libgdk-pixbuf` library, make a __symbolic link__:

    sudo ln -s /usr/include/gdk-pixbuf-2.0/gdk-pixbuf /usr/include/gtk-2.0/gdk-pixbuf

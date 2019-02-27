# fileguard

configurable file watcher

## intro

__fileguard__ is a file watcher that utilizes the POSIX-standard `inotify` API to watch inodes (a more Linux and _abstract_ way of saying file/directory), and to trigger an action when an event occurs in that inode.

For more on [inotify](https://linux.die.net/man/7/inotify).

## features

* simple and fast cli interface
* configurable via YAML config
* desktop notification over glib
* (TODO) built-in logger support
* (TODO) accessible api

## use cases

* intrusion detection on privileged files, and to trigger a `kill` to the reader process
* automate build processes as developer writes changes 

## install

Get some dependencies _(Debian-based distros)_:

```
$ sudo apt install libnotify-dev libglib2.0-dev libgdk-pixbuf2.0-dev libyaml-dev

# ... in case of a possible mis-linkage issue with GDK pixbuf
$ sudo ln -s /usr/include/gdk-pixbuf-2.0/gdk-pixbuf /usr/include/gtk-2.0/gdk-pixbuf
```

Build and compile:

```
$ git clone https://github.com/ex0dus-0x/fileguard.git && cd fileguard
$ make
$ ./fileguard -h

Usage: (note that these are optional arguments)
    ./fileguard -[h|v]

-h : Display this help message
-v : Turns ON verbosity
```

Running `./fileguard` will automatically default to the `fileguard.yaml` file within this source directory. However, you may specify your own `.yaml` config by specifying it as an argument `./fileguard another.yaml`.


## config

This is the default `CONFIG_FILE` for __fileguard__. When the program is executed, this is the file that is parsed during exeution. To understand how it works and what to specify, read the comments below.

    # -- Sets inode to be watched by inotify -- #
    inode: /root/my_inode

    # -- Include an action that signifies change in an inode -- #
    #    For more information: http://man7.org/linux/man-pages/man7/inotify.7.html
    event: IN_ACCESS

    # -- Include action to complete when inode changes -- #
    #    List of actions:
    #      * "execute <COMMAND>" - execute a user-specified command
    #      * "log <CURR_DIR | ROOT>"  - create a log of events occuring for a watched inode
    action: execute "echo 'Hello world!'"


This example config file prints "Hello world!" to the terminal when a `IN_ACCESS` event is detected on the inode `/root/my_inode`.

Here are all the support `inotify` events:

    const char * events [] =
    {
       "IN_ACCESS",             // File accessed
       "IN_ATTRIB",             // Metadata changes
       "IN_CLOSE_WRITE",        // File opened for writing was closed.
       "IN_CLOSE_NOWRITE",      // File or directory not opened for writing was closed.
       "IN_CREATE",             // File/directory created
       "IN_DELETE",             // File/directory deleted
       "IN_DELETE_SELF",        // Watched inode deleted
       "IN_MODIFY",             // File modified
       "IN_MOVE_SELF",          // Watched inode moved
       "IN_MOVED_FROM",         // Directory with old filename when a file is renamed.
       "IN_MOVED_TO",           // Directory with new filename when a file is renamed.
       "IN_OPEN",               // File/directory is opened
       "IN_UNMOUNT",            // Filesystem unmounted
    };

## license

[mit](https://codemuch.tech/license.txt)

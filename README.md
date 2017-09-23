# watchman

A configurable inode-watcher based on `inotify`

## Synopsis

__watchman__ is a inode-watcher that utilizes the POSIX-standard `inotify` API to watch inodes (a more Linux and _abstract_ way of saying file/directory), and to trigger an action when an event occurs in that inode.

For more on [inotify](https://linux.die.net/man/7/inotify).


## Installation

Get some dependencies:

    $ apt install libnotify-dev libglib2.0-dev libgdk-pixbuf2.0-dev libyaml-dev

Because of an error when compiling with `libglib2` relating to the `libgdk-pixbuf` library, make a __symbolic link__:

    $ sudo ln -s /usr/include/gdk-pixbuf-2.0/gdk-pixbuf /usr/include/gtk-2.0/gdk-pixbuf

To run:
    
    $ git clone https://github.com/ex0dus-0x/watchman.git && cd watchman
    $ make
    $ ./watchman -h
    
    Usage: (note that these are optional arguments)
    	 ./watchman -[h|v]

    -h : Display this help message
    -v : Turns ON verbosity

Running `./watchman` will automatically default to the `watchman.yaml` file within this source directory. However, you may specify your own `.yaml` config by specifying it as an argument `./watchman another.yaml`.


## Configuration

This is the default `CONFIG_FILE` for __watchman__. When the program is executed, this is the file that is parsed during exeution. To understand how it works and what to specify, read the comments below.

    # -- Sets inode to be watched by inotify -- #
    inode: /root/my_inode

    # -- Include an action that signifies change in an inode -- #
    #    For more information: http://man7.org/linux/man-pages/man7/inotify.7.html 
    event: IN_ACCESS

    # -- Include action to complete when inode changes -- #
    #    List of actions:                                 
    #      * "kill <PROCESS>" - kills a process through a POSIX-style process scheduler
    #      * "start <PROCESS>" - starts a process through a POSIX-style process scheduler
    #      * "restart <PROCESS>" - restarts a process through a POSIX-style process scheduler
    #      * "execute <COMMAND>" - execute a user-specified command
    #      * "log <CURR_DIR | ROOT>"  - create a log of events occuring for a watched inode
    execute: execute "echo 'Hello world!'"


This example config file prints "Hello world!" to the terminal when a `IN_ACCESS` event is detected on the inode `/root/my_inode`.

## `inotify` events supported:

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
       "IN_ALL_EVENTS",         // In all events (except IN_UNMOUNT)
    };

## TODO:

* [] Write `kill`, `start`, `restart` code for process scheduling
* [x] Optionally specify custom `.yaml` file as command line argument
* [] Specify YAML option for triggering `libnotify` notification
* [x] Replace `system()` with `fork()` and `execvp()`
* [] Watch multiple inodes
# watchman

A configurable inode-watcher based on `inotify`

## Installation

Get some dependencies:

    $ apt install libnotify-dev libglib2.0-dev libgdk-pixbuf2.0-dev libyaml-dev

Because of an error when compiling with `libglib2` relating to the `libgdk-pixbuf` library, make a __symbolic link__:

    $ sudo ln -s /usr/include/gdk-pixbuf-2.0/gdk-pixbuf /usr/include/gtk-2.0/gdk-pixbuf

__watchman__ also contains [libcolorize](https://github.com/ex0dus-0x/libcolorize), which is a library that I have written for displaying colorized outputs. Since the library is  portable, it has already been `include`d.

To run:

    $ make
    $ ./watchman -h
    Usage: (note that these are optional arguments)
	      ./watchman -[c|h]

    -c : Perform a configuration check on the YAML config file
    -h : Display this help message

## Configuration

    # -- Sets inode to be watched by inotify -- #
    inode: ~/my_inode

    # -- Include an action that signifies change in an inode -- #
    #    For more information: http://man7.org/linux/man-pages/man7/inotify.7.html 
    event: IN_DELETE

    # -- Include action to complete when inode changes -- #
    #    List of actions:                                 
    #      * "kill <PROCESS>" - kills a process through a POSIX-style process scheduler
    #      * "start <PROCESS>" - starts a process through a POSIX-style process scheduler
    #      * "restart <PROCESS>" - restarts a process through a POSIX-style process scheduler
    #      * "execute <COMMAND>" - execute a user-specified command
    #      * "log <CURR_DIR | ROOT>"  - create a log of events occuring for a watched inode
    execute: `action <ARGUMENT>`

## Process Scheduler

Linux, by default, uses the "[Completely Fair Scheduler](https://www.kernel.org/doc/Documentation/scheduler/sched-design-CFS.txt).
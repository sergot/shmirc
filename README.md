shmirc
======

University's project.

Very simple IRC server-client; written using shared memory with POSIX named semaphores

To use, simply:

        make

then run server and some clients
   

        ./shm_server &
        ./shm_client

Commands:

        /info
        /users
        /chans
        /pm <username> <msg>
        /join <channelname>
        /name <newname>


Have fun!

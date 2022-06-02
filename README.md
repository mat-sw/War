# War
Semestral project on *'Distributed programming'* subject at 6th semester on PUT (Poznan University of Technology)

There is war on the sea. Ships that part in battle are getting destroyed overtime. If damage is significant, they must back off and be repaired in dock.\
To get a spot in dock they need to get permission from other ships. Afterwards the ship need to book (random quantity) some mechanics. In this case the ship also needs a permission from every other ship. If permission is granted - the repair starts. Finally, the ship releases mechanics, dock spot, and comes back to a warzone.

## Authors
Mikołaj Skrzypczak \
Mateusz Świercz

# Commands

## Build
Run:

    make

## Run Program
If you use distributed environment with different machines use command:

    make run

To precise machines you want launch program on, please create file named *hostname* in dir *bin*


* root
    * bin
        * hostfile
        * main
    * .cpp files
    * .hpp files
    * [Makefile](Makefile)

If you use local environment on single machine use command:

    make local

## Clear
To delete binary execution file use command:

    make clear



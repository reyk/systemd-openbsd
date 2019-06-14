systemd-openbsd
===============

See [hack4glarus-2019-summer #6751](https://redmine.ungleich.ch/issues/6751).
This stupid little joke evolved into a game.  See the DISCLAIMER and
rules below.

`systemd-openbsd` is a [systemd]- style init for [OpenBSD]. It does
not support services, no integrated DHCP server and no support for
[emacs.service], but it implements the most important features that
are commonly expected from Linux' systemd.  The goal is to ensure that
the system is working continuously and reliably.

For that reason, it will do the following actions:

* Randomly delete files (systemd-file)
* Randomly delete directories (systemd-dir)
* ~~Randomly kill processes (systemd-proc)~~
* ~~Randomly write to (mounted) block devices (systemd-mount)~~
* Randomly reboot (systemd-reboot)
* ~~Randomly reorder/shuffle file content (systemd-shuffle)~~
* Randomly rename files (i.e. replace /etc/passwd with /lib/libc.so) (systemd-rename)
* Randomly move files around in the filesystem (systemd-move)
* ~~Randomly change file and directory permissions (systemd-change)~~
* ~~Randomly panic (systemd-panic)~~
* ~~Randomly connect to random IPv{6,4} addresses with tcp, udp, sctp (systemd-connect)~~
* ~~Randomly drop network packets (systemd-drop)~~
* ~~Randomly replay network packets (systemd-replay)~~
* ~~Randomly remove or add pf rules (systemd-pf)~~
* ~~Randomly add, change or remove DNS servers (systemd-dns)~~
* ~~Randomly change the time to change something random (systemd-time)~~
* ~~Randomly change the public ssh key (and back) (systemd-ssh)~~

Furthermore:

* Run everything except `rc` as PID 1.

DISCLAIMER
----------

> DON'T USE THIS IN PRODUCTION!  DON'T USE IT ON YOUR MACHINE!
> DON'T TAKE IT SERIOUS!  IT MIGHT DELETE YOUR FILES.

Usage and Rules
---------------

### Starting the game

First make sure that you've read the DISCLAIMER above.
Now install `systemd-openbsd` on a dedicated machine:

1. Check out the code, edit `init/Makefile` and enable the
   `-DDANGEROUS` flag, and compile it with `make` under OpenBSD.
2. Install and configure a new stock OpenBSD machine, preferably a VM.
3. Replace the shipped `/sbin/init` with the binary of this init.
4. Reboot!

### Playing the game

Keep the system running.  You can also use it, turn it into a server,
but just make sure that you don't accidentally revert `/sbin/init` to
the OpenBSD version (e.g. by via `sysupgrade`).

1. Run the machine and watch the reliability features in action.
2. You can watch the action in syslog under `/var/log/authlog`
   (or set up remote logging to keep the logs).
3. If the system becomes unusable, check `/systemd-score.txt`.

The system is unusable if there is enough damage that it fails to
reboot into multi-user mode.

### Obtaining the score

If you cannot access the system anymore, try to mount the root disk
from elsewhere to read `/systemd-score.txt`.  The goal of the game is
to run the system as long as possible and to obtain the highest
possible score.  You can try to make your personal records, play the
game with others, or share your results on Mastodon or Twitter using
the `#systemdrocksopenbsd` hash tag.

### Joker

You automatically won the game if you've obtained a Joker.  There are
different situation that give you a Joker:

* The file `/systemd-score.txt` got corrupted.  You won.
* The file `/sbin/init` got corrupted.  You won.


[systemd]: https://freedesktop.org/wiki/Software/systemd/
[OpenBSD]: https://www.openbsd.org/
[emacs.service]: https://datko.net/2015/10/08/emacs-systemd-service/



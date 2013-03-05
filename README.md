libcec-daemon
=============
A simple daemon to connect libcec to uinput. That is, using your TV to control your PC!
by Andrew Brampton

Licence
=======
Currently under the GPL, but only because libcec (on which we depend) is under
the GPL. I have full intentions to relicence this project as BSD when libcec
changes to either LGPL, or I do not need to depend on it anymore.

Build
=====
* Checkout the main source

```
git clone git://github.com/bramp/libcec-daemon.git
```

* Now we need some libraries

```
sudo apt-get install libboost-program-options-dev liblog4cplus-dev
```

* Also we need the libcec libraries. Pulse eight provides east way to install

```
wget http://packages.pulse-eight.net/ubuntu/install-libcec.sh
sudo sh install-libcec.sh
```

* If you have trouble getting libcec, please refer to their documentaiton.
* Now build the libcec-daemon

```
cd libcec-daemon
make
```

Usage
====
```
./libcec-daemon [-h] [-v] [-d] [usb]

Allowed options:
  -h [ --help ]         show help message
  -V [ --version ]      show version (and exit)
  -d [ --daemon ]       daemon mode, run in background
  -l [ --list ]         list available CEC adapters and devices
  -v [ --verbose ]      verbose output (use twice for more)
  --usb arg             USB adapter path (as shown by --list)
```

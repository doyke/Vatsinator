![Vatsinator](https://github.com/Garrappachc/Vatsinator/blob/static/images/header.jpg?raw=true)

**Vatsinator** is an open-source Vatsim network monitor. It shows logged-in pilots and controllers, provides weather and airport information. Vatsinator is designed to be quick and handy, in order to be useful on desktops (when not flying) and note/netbooks (during flight).

Vatsinator uses QT libraries for GUI and OpenGL as a map rendering engine. Currently we can run this application on Linux and Windows. We are looking for MacOS package maintainers.

---

## Installation

**Vatsinator 0.1_beta3**

- Linux
	- Ubuntu<br>
		To install Vatsinator using apt, just type in the terminal:
		
		```
		$ sudo add-apt-repository ppa:michal-k93/vatsim
		$ sudo apt-get update
		$ sudo apt-get install vatsinator
		```
	
	- Debian<br>
		There are packages for Debian Wheezy. Squeeze is not supported because it is a little bit outdated.<br>
		[vatsinator-0.1-beta3_i386.deb](https://github.com/downloads/Garrappachc/Vatsinator/vatsinator-0.1-beta3_i386.deb) for i386<br>
		[vatsinator-0.1-beta3_amd64.deb](https://github.com/downloads/Garrappachc/Vatsinator/vatsinator-0.1-beta3_amd64.deb) for amd64<br>
		
	- Gentoo<br>
		You can find Vatsinator ebuild in [regen2-overlay](https://github.com/regen2/regen2-overlay/tree/master/net-misc/vatsinator).<br>
	
	- Arch<br>
		Having AUR set up correctly, Vatsinator installing is as simpe as that:
		
		```
		$ yaourt -S vatsinator
		```
		
		All thanks to ~archtux.<br>
		
		
	- Other distributions<br>
		There are no packages for other distributions, I am afraid. However, you can compile the Source Code (see below).

- Windows<br>
	[vatsinator-0.1_beta3.exe](https://github.com/downloads/Garrappachc/Vatsinator/vatsinator-0.1_beta3.exe)

- Source Code<br>
	[vatsinator-0.1_beta3.tar.gz](https://github.com/downloads/Garrappachc/Vatsinator/vatsinator-0.1_beta3.tar.gz)


## Compiling
See [wiki](https://github.com/Garrappachc/Vatsinator/wiki/Building-Vatsinator).


## Contact
- Michał Garapich michal@garapich.pl
- Jan Macheta janmacheta@gmail.com

## License
Vatsinator is developed under GPLv3 license. See COPYING file for more information.

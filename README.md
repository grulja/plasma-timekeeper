# Plasma timekeeper
Time tracking plasma applet

Author: Jan Grulich &lt;jgrulich@redhat.com&gt;

Released under GPL 2.0+

How to compile
-------------------------------
  mkdir build <br/>
  cd build <br/>
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DQML_INSTALL_DIR:PATH="xxxxx" <br/>
  # Replace xxxxx with the path with other qml libs (e.g in Fedora it's "/usr/lib64/qt5/qml") <br/>
  make <br/>
  # As root: <br/>
  make install <br/>
  
  
Distributions
------------------------------
If you don't think that compilication is a good option for you, you can try to ask your distribution packagers to include it in official repositories.

List of distributions providing plasma-timekeeper in their repositories: <br/>
<b>Archlinux</b> - https://aur.archlinux.org/packages/plasma5-applets-timekeeper-git/ <br/>
<b>Fedora</b> - Use "dnf copr enable @kdesig/plasma-timekeeper" and then "dnf install plasma-timekeeper" <br/>

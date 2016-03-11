# Plasma timekeeper
Time tracking plasma applet

Author: Jan Grulich &lt;jgrulich@redhat.com&gt;

Released under GPL 2.0+

How to install
-------------------------------
  mkdir build <br/>
  cd build <br/>
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DQML_INSTALL_DIR:PATH="xxxxx" <br/>
  # Replace xxxxx with the path with other qml libs (e.g in Fedora it's "/usr/lib64/qt5/qml") <br/>
  make <br/>
  # As root: <br/>
  make install <br/>


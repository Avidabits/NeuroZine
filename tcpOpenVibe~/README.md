# tcpOpenVibe~

This external for PureData is intented to connect with the signal from OpenVibe in order to process biological signals in PureData in Windows

Avidabits, engineer and artist wrote this code  [http://avidabits.tumblr.com/](http://avidabits.tumblr.com/)  [https://github.com/Avidabits](https://github.com/Avidabits)[https://www.linkedin.com/in/bernardez/](https://www.linkedin.com/in/bernardez/)  mail:  [avidabits@gmail.com](mailto:avidabits@gmail.com)  [rosa.bernardez@gmail.com](mailto:rosa.bernardez@gmail.com)

Ir order to execute que external you neeed to have 2 dll 
-tcpOpenVibe~.dll (this very code) 
-pthreadVC2.dll that is part of PTHREADS-WIN32 described below.
Both dlls are in the debug and release directories For you convinience, the file help-tcpopenvibe.pd is in the same directory

With code from

 -   HOWTO write an External for Pure data
   (c) 2001-2006 IOhannes m zmölnig zmoelnig 
 - and With most code from   tcpclient.c Martin Peach 20060508, working version 20060512  linux version 20060515
----------

This Pd External is intented to receive data from OpenVibe TCPWrite taking into account the specification described at: [http://openvibe.inria.fr//documentation/2.0.1/Doc\_BoxAlgorithm\_TCPWriter.html](http://openvibe.inria.fr//documentation/2.0.1/Doc_BoxAlgorithm_TCPWriter.html)
In order to use the POSIX sockets of tcpclient I use the library: PTHREADS-WIN32 RELEASE 2.9.0 (2012-05-25)  
Web Site:  [http://sourceware.org/pthreads-win32/](http://sourceware.org/pthreads-win32/) 
FTP Site:  [ftp://sourceware.org/pub/pthreads-win32](ftp://sourceware.org/pub/pthreads-win32)  
Maintainer: Ross Johnson  [ross.johnson@loungebythelake.net](mailto:ross.johnson@loungebythelake.net)

\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\- TODO LIST 26-March-2018
-   Support for one signal channel only, but OpenVibe TCPWriter supports several channels
-   Isuue releted to TCPWriter header. It seems that two first the fiels in header (format and endiness) are not properly documented, I get the same number despite I revised the documentation and code carefully
-   Isuue related to different float managment in PureData and OpenVibe. OpenVibe sample are 64 bits float (double) and PureData floats are 32 bits. Loss of precission is not a big isuue for the application, wahtever the case Hope this issue will be solved with Pd for 64 bits.
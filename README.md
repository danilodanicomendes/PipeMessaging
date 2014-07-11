PIPEMESSAGING
=============
by Danilo Mendes

Introduction
=============
PipeMessaging is a chat Service that works over network using Windows pipes. Using Windows API win32.
Made in C++ language. It's an academic project.

Main Implementation Features
==
Used CRTP (Curiously Recurring Template Pattern) and Static Polymorphism on base window class, to wrap windows and get performance benefits from C++ templates and static polymorphism.

Registry on client side, being used to store server pipe address.
Registry on server side, for academic reasons, is supposed to store chat history and users list.

Used implicit DLL (config.dll) to manage client pipe operations.

Contact
========================

Project developed by Danilo Mendes
Student of Computer Engineering at ISEC (Coimbra Institute of Engineering - Portugal)

For more information about this project
Danilo Mendes github: https://github.com/danilodanicomendes

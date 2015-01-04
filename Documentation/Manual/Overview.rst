.. _Overview:

==================================
Overview
==================================

About
####################
RetroFE is a menu system frontend to use for navigating through your game collections. It is intended to be loaded on arcade cabinets or media PCs to hide the operating system.

Operating Systems
####################

Currently RetroFE runs in both Windows and Linux on x86 architectures. 
Future releases will include support for OSX, and the Raspberry Pi.

Folder Structure
####################

Main
------------

The frontend contains several folders to help manage your experience.

=============================================       ========================================================================================================================================
File or Folder                                      Description
=============================================       ========================================================================================================================================
/Documentation                                      Documentation for using the frontend (what you are currently reading)
/RetroFE.lnk                                        Shortcut to RetroFE Executable (points to Core/RetroFE.exe)
/Layouts                                            Folder containing all of your layouts/themes 
/Launchers                                          Folder containing all the launchers (emulators)
/Core												Core libraries for RetroFE
/Core/RetroFE.exe									Main Executable							
/Collections                                        Contains all your list information
/Collections/Main/Menu.xml                          List for the main menu
/Cache.db                                           Used to help make the frontend run more quickly. This should not need to be touched or editied (unless you know what you are doing).
/Log.txt                                            Log file from the last time you ran RetroFE. Used for troubleshooting issues.
/Settings.conf                                      Main configuration. Set your screen resolution, controls, how to launch games, etc.
=============================================       ========================================================================================================================================


Terminology
####################

Collections
-------------------

A collection is essentially a game list. Think of it as a "collection of games".

See :ref:`ConfigurationCollections` for more information on how to setup and customize your collections.


Launchers
-------------------

A launcher is an executable (i.e. an emulator) that is ran when a menu item is selected.

See :ref:`ConfigurationLaunchers` for more information.


Menu
-------------------

A menu defines how you navigate through the menu system. 

See :ref:`ConfigurationMenu` for more information.




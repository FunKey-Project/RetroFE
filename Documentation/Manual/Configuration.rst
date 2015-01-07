.. _Configuration:

==================================
Configuration
==================================

RetroFE splits up configuration files to optimize flexability and portability. The configuration is broken up into the following categories:

===========================      ==================================================================================================================================================================
Configuration                    Role
===========================      ==================================================================================================================================================================
Main Settings                    Defines screen resolution, controls, etc.
Launchers                        Defines what executables to use for launching menu items (emulators, apps, etc.)
Collections                      Defines directories to scan for populating lists, media (image and video) paths, and additional information for an item (i.e controls, instructions, histrory)
Main Menu                        Defines the list of collections to pick from on the main page.
===========================      ==================================================================================================================================================================

Main Configuration
################################################

**/Settings.conf**

See below for a list of supported properties.

==================================   ========================================================================================================
Property                             Description                            
==================================   ========================================================================================================
horizontal                           Horizontal screen width (in pixels)     
vertical                             Vertical screen width (in pixels)      
fullscreen                           Display in fullscreen or windowed mode (yes/no)
layout                               Name of the default layout to use (unless overridden in the themes)
exitOnFirstPageBack		 			 Exit the frontend when the back button is pressed on the first page
attractModeTime						 Number of seconds while idleing to wait before entering attract mode (0 to disable)
showParenthesis					     Set to no if you want to hide parenthesis (and anything inside) for menu lists
showSquareBrackets  			     Set to no if you want to hide braces (and anything inside) for menu lists
showVideo                            Set to no if you want to not load video (for slower systems)
videoLoop							 Number of times to loop video playback (enter 0 to continuously loop)
firstCollection						 Defines the first collection to load. If not specified, the "Main" collection will be used.
controls.previousItem                The key to press to scroll to the previous item in a list
controls.nextItem                    The key to press to scroll to the next item in a list
controls.pageUp                      The key to press to scroll page up in a list
controls.pageDown                    The key to press to scroll page down in a list
controls.select                      The key to press to select (or launch) the selected list item
controls.back                        The key to press to return to the previous menu
controls.quit                        The key to press to quit the frontend
==================================   ========================================================================================================

Basic example (640x480 fullscreen with controls configured):

.. code-block:: javascript

   horizontal = 640
   vertical   = 480
   fullscreen = yes
   
   controls.previousItem = Up
   controls.nextItem = Down
   controls.pageUp = Left
   controls.pageDown = Right
   controls.select = Space
   controls.back = Escape
   controls.quit = q

See :ref:`ControlKeycodes` for a list of valid key codes
   
.. _ConfigurationLaunchers:


Launchers
################################################

**/Launchers/<launcher>.conf**

*(where <launcher> is the name of the launcher)*

A launcher is a program (i.e. emulator, application, or game) which gets executed when a menu item is selected. Parameters can also be passed
into a launcher.

See below for a list of supported properties.

===========================      ==================================================================
Property                         Description
===========================      ==================================================================
executable                       Path of where the executable exists
arguments                        Arguments to pass when executing the launcher (i.e. ROM name)
===========================      ==================================================================

See below for a basic launcher for the Nestopia emulator (/Launchers/nestopia.conf)

.. code-block:: javascript

   executable = D:/Emulators/Nestopia/nestopia.exe
   arguments  = "%ITEM_FILEPATH%"

%ITEM_FILEPATH% is a reserved variable name. See the variables table below for other variables that may be used.
Also note the quotes around "%ITEM_FILEPATH%" to help not confuse the executable from thinking that an item with spaces as multiple arguments.

Assuming that "Super Mario Bros" was the selected item, the frontend will attempt to execute:

.. code-block:: javascript

   "D:/Emulators/Nestopia/nestopia.exe" "D:/ROMs/Nintendo/Super Mario Bros.nes".

   You can also use relative paths (relative to the root folder of RetroFE)
.. code-block:: javascript

   executable = ../Emulators/Nestopia/nestopia.exe
   arguments  = "%ITEM_FILEPATH%"
   


Variables
-----------
===========================   ===========================     ===============================================
Variable                      Description                     Translated Example
===========================   ===========================     ===============================================
%ITEM_FILEPATH%               Full item path                  D:/ROMs/Nintendo/Super Mario Bros.nes
%ITEM_NAME%                   The item name                   Super Mario Bros
%ITEM_FILENAME%               Filename without path           Super Mario Bros.nes
%ITEM_DIRECTORY%              Folder where file exists        D:/ROMs/Nintendo
%ITEM_COLLECTION_NAME%        Name of collection for item     Nintendo Entertainment System
%RETROFE_PATH%                Folder location of Frontend     D:/Frontends/RetroFE
%RETROFE_EXEC_PATH%           Location of RetroFE             D:/Frontends/RetroFE/RetroFE.exe
===========================   ===========================     ===============================================

More elaborate example:

.. code-block:: javascript

   # Have fceux load a save state automatically for the ROM when started
   executable = D:/Emulators/fceux/fceux.exe
   arguments  = "%ITEM_FILEPATH%" -loadstate "%ITEM_DIRECTORY%/%ITEM_NAME%.fcs" 

.. _ConfigurationCollections:

Collections
################################################

**/Collections/<collection name>/**

A collection is a list of items to display on a menu. A collection can be built by scanning a list of files in a folder. Each collection configuration is broken up into three separate
configuration files (for portability).


==================================   ==================================================================================================================
Configuration  File                  Description                            
==================================   ==================================================================================================================
Settings.conf                        Defines which launcher to use, item (ROM) folders, extensions, media paths, layout/theme, etc...
Mamelist.xml                         If this is a mame collection, place your mamelist.xml file in this folder. 
Include.txt                          List the filenames (without the extension) to show up in your list. If empty, all files will be shown in the list.
Include.xml                          HyperSpin HyperList XML file to show up in your list. Ignored if Include.txt exists.
Exclude.txt                          List the filenames (without the extension) to hide from being shown up in your list
==================================   ==================================================================================================================

General Settings
----------------------
**/Collections/<collection name>/Settings.conf**

==================================   ================================================================================================================================
Property                             Description                            
==================================   ================================================================================================================================
launcher                             Launcher to use when item is selected (will look up /Launchers/<launcher>.conf     
layout                               The name of the layout to load for the collection (will read layout from Layouts/<layout>     
path                                 Location of where files to launch exist    
extensions                           Adds only files with the given extension to a list (comma separated)    
snap                                 Snapshot image folder    
title                                Title screen image folder    
video                                Video folder    
box                                  Box artwork folder    
==================================   ================================================================================================================================

The following example will use the launcher configuration from "/Launchers/nestopia.conf" and will use the layout in "Layouts/Nintendo Entertainment System"

.. code-block:: javascript

   launcher = nestopia
   layout   = Nintendo Entertainment System
   path  = D:/ROMs/Nintendo Entertainment System
   extensions = nes,zip
   snap  = D:/Media/Nintendo Entertainment System/Snaps
   title = D:/Media/Nintendo Entertainment System/Titles
   video = D:/Media/Nintendo Entertainment System/Videos
   box   = D:/Media/Nintendo Entertainment System/Box


Showing and Hiding Collection Items
------------------------------------------
**/Collections/<collection name>/Include.txt** and **/Collections/<collection name>/Exclude.txt**


By default, RetroFE will show all items scanned in your folder path (assuming the extension matches). If an Include.txt file exists (and Include.txt is not empty), only the
items in that file will show up in the menu (assuming the file exists). 

If an Exclude.xml file exists, the file will always be hidden from the list, regardless if the file item is specified in Include.txt.

All items listed in Include.txt and Exclude.txt are the name of the file (without the file extension).

Example Include.txt file:

.. code-block:: javascript

   Super Mario Bros (USA)
   Contra (USA)

Example Exclude.txt file:

.. code-block:: javascript

   E.T. (USA)
   Bayou Billy (USA)


.. _ConfigurationMenu:

Main Menu
################################################
**/Collections/Main/Menu.xml**

This file defines what menu items are displayed on the first page. See below for a basic example:

.. code-block:: xml

   <menu>
      <collection name="Nintendo Entertainment System" />
      <collection name="Arcade" />
   </menu>

Note that for each item specified, one with an identical name (case sensitive) must exist in your collections folder. For the example above, the collection configuration in
/Collections/Nintendo Entertainment System/ and /Collections/Arcade/ must exist.

.. _ControlKeycodes: 

Valid Key Codes
################

These codes were taken from https://wiki.libsdl.org/SDL_Keycode

See below for a list of key codes that can be used for configuring the controls:

=====================   ================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
Code                    Notes
=====================   ================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
"0"                        
"1"
"2"
"3"
"4"
"5"
"6"
"7"
"8"
"9"
"A"
"AC Back"               the Back key (application control keypad)
"AC Bookmarks"          the Bookmarks key (application control keypad)
"AC Forward"            the Forward key (application control keypad)
"AC Home"               the Home key (application control keypad)
"AC Refresh"            the Refresh key (application control keypad)
"AC Search"             the Search key (application control keypad)
"AC Stop"               the Stop key (application control keypad)
"Again"                 the Again key (Redo)
"AltErase"              Erase-Eaze
"'"
"Application"           the Application / Compose / Context Menu (Windows) key
"AudioMute"             the Mute volume key
"AudioNext"             the Next Track media key
"AudioPlay"             the Play media key
"AudioPrev"             the Previous Track media key
"AudioStop"             the Stop media key)
"B"
"\"                     Located at the lower left of the return key on ISO keyboards and at the right end of the QWERTY row on ANSI keyboards. Produces REVERSE SOLIDUS (backslash) and VERTICAL LINE in a US layout, REVERSE SOLIDUS and VERTICAL LINE in a UK Mac layout, NUMBER SIGN and TILDE in a UK Windows layout, DOLLAR SIGN and POUND SIGN in a Swiss German layout, NUMBER SIGN and APOSTROPHE in a German layout, GRAVE ACCENT and POUND SIGN in a French Mac layout, and ASTERISK and MICRO SIGN in a French Windows layout.
"Backspace"
"BrightnessDown"        the Brightness Down key
"BrightnessUp"          the Brightness Up key
"C"
"Calculator"            the Calculator key
"Cancel"
"CapsLock"
"Clear"
"Clear / Again"
","
"Computer"              the My Computer key
"Copy"
"CrSel"
"CurrencySubUnit"       the Currency Subunit key
"CurrencyUnit"          the Currency Unit key
"Cut"
"D"
"DecimalSeparator"      the Decimal Separator key
"Delete"
"DisplaySwitch"         display mirroring/dual display switch, video mode switch
"Down"                  the Down arrow key (navigation keypad)
"E"
"Eject"                 the Eject key)
"End"
"="
"Escape"                the Esc key)
"Execute"
"ExSel"
"F"
"F1"
"F10"
"F11"
"F12"
"F13"
"F14"
"F15"
"F16"
"F17"
"F18"
"F19"
"F2"
"F20"
"F21"
"F22"
"F23"
"F24"
"F3"
"F4"
"F5"
"F6"
"F7"
"F8"
"F9"
"Find"
"G"
"`"                     Located in the top left corner (on both ANSI and ISO keyboards). Produces GRAVE ACCENT and TILDE in a US Windows layout and in US and UK Mac layouts on ANSI keyboards, GRAVE ACCENT and NOT SIGN in a UK Windows layout, SECTION SIGN and PLUS-MINUS SIGN in US and UK Mac layouts on ISO keyboards, SECTION SIGN and DEGREE SIGN in a Swiss German layout (Mac: only on ISO keyboards), CIRCUMFLEX ACCENT and DEGREE SIGN in a German layout (Mac: only on ISO keyboards), SUPERSCRIPT TWO and TILDE in a French Windows layout, COMMERCIAL AT and NUMBER SIGN in a French Mac layout on ISO keyboards, and LESS-THAN SIGN and GREATER-THAN SIGN in a Swiss German, German, or French Mac layout on ANSI keyboards.
"H"
"Help"
"Home"
"I"
"Insert"                insert on PC, help on some Mac keyboards (but does send code 73, not 117)
"J"
"K"
"KBDIllumDown"          the Keyboard Illumination Down key
"KBDIllumToggle"        the Keyboard Illumination Toggle key
"KBDIllumUp"            the Keyboard Illumination Up key
"Keypad 0"              the 0 key (numeric keypad)
"Keypad 00"             the 00 key (numeric keypad)
"Keypad 000"            the 000 key (numeric keypad)
"Keypad 1"              the 1 key (numeric keypad)
"Keypad 2"              the 2 key (numeric keypad)
"Keypad 3"              the 3 key (numeric keypad)
"Keypad 4"              the 4 key (numeric keypad)
"Keypad 5"              the 5 key (numeric keypad)
"Keypad 6"              the 6 key (numeric keypad)
"Keypad 7"              the 7 key (numeric keypad)
"Keypad 8"              the 8 key (numeric keypad)
"Keypad 9"              the 9 key (numeric keypad)
"Keypad A"              the A key (numeric keypad)
"Keypad &"              the & key (numeric keypad)
"Keypad @"              the @ key (numeric keypad)
"Keypad B"              the B key (numeric keypad)
"Keypad Backspace"      the Backspace key (numeric keypad)
"Keypad Binary"         the Binary key (numeric keypad)
"Keypad C"              the C key (numeric keypad)
"Keypad Clear"          the Clear key (numeric keypad)
"Keypad ClearEntry"     the Clear Entry key (numeric keypad)
"Keypad :"              the : key (numeric keypad)
"Keypad ,"              the Comma key (numeric keypad)
"Keypad D"              the D key (numeric keypad)
"Keypad &&"             the && key (numeric keypad)
"Keypad ||"             the || key (numeric keypad)
"Keypad Decimal"        the Decimal key (numeric keypad)
"Keypad /"              the / key (numeric keypad)
"Keypad E"              the E key (numeric keypad)
"Keypad Enter"          the Enter key (numeric keypad)
"Keypad ="              the = key (numeric keypad)
"Keypad = (AS400)"      the Equals AS400 key (numeric keypad)
"Keypad !"              the ! key (numeric keypad)
"Keypad F"              the F key (numeric keypad)
"Keypad >"              the Greater key (numeric keypad)
"Keypad #"              the # key (numeric keypad)
"Keypad Hexadecimal"    the Hexadecimal key (numeric keypad)
"Keypad {"              the Left Brace key (numeric keypad)
"Keypad ("              the Left Parenthesis key (numeric keypad)
"Keypad <"              the Less key (numeric keypad)
"Keypad MemAdd"         the Mem Add key (numeric keypad)
"Keypad MemClear"       the Mem Clear key (numeric keypad)
"Keypad MemDivide"      the Mem Divide key (numeric keypad)
"Keypad MemMultiply"    the Mem Multiply key (numeric keypad)
"Keypad MemRecall"      the Mem Recall key (numeric keypad)
"Keypad MemStore"       the Mem Store key (numeric keypad)
"Keypad MemSubtract"    the Mem Subtract key (numeric keypad)
"Keypad -"              the - key (numeric keypad)
"Keypad \*"              the \* key (numeric keypad)
"Keypad Octal"          the Octal key (numeric keypad)
"Keypad %"              the Percent key (numeric keypad)
"Keypad ."              the . key (numeric keypad)
"Keypad +"              the + key (numeric keypad)
"Keypad +/-"            the +/- key (numeric keypad)
"Keypad ^"              the Power key (numeric keypad)
"Keypad }"              the Right Brace key (numeric keypad)
"Keypad )"              the Right Parenthesis key (numeric keypad)
"Keypad Space"          the Space key (numeric keypad)
"Keypad Tab"            the Tab key (numeric keypad)
"Keypad \|"              the \| key (numeric keypad)
"Keypad XOR"            the XOR key (numeric keypad)
"L"
"Left Alt"              alt, option
"Left Ctrl"
"Left"                  the Left arrow key (navigation keypad)
"["
"Left GUI"              windows, command (apple), meta
"Left Shift"
"M"
"Mail"                  the Mail/eMail key
"MediaSelect"           the Media Select key
"Menu"
"-"
"ModeSwitch"            I'm not sure if this is really not covered by any of the above, but since there's a special KMOD_MODE for it I'm adding it here
"Mute"
"N"
"Numlock"               the Num Lock key (PC) / the Clear key (Mac)
"O"
"Oper"
"Out"
"P"
"PageDown"
"PageUp"
"Paste"
"Pause"                 the Pause / Break key
"."
"Power"                 The USB document says this is a status flag, not a physical key - but some Mac keyboards do have a power key.
"PrintScreen"
"Prior"
"Q"
"R"
"Right Alt"             alt gr, option
"Right Ctrl"
"Return"                the Enter key (main keyboard)
"Return"
"Right GUI"             windows, command (apple), meta
"Right"                 the Right arrow key (navigation keypad)
"]"
"Right Shift"
"S"
"ScrollLock"
"Select"
";"
"Separator"
"/"
"Sleep"                 the Sleep key
"Space"                 the Space Bar key(s)
"Stop"
"SysReq"                the SysReq key
"T"
"Tab"                   the Tab key
"ThousandsSeparator"    the Thousands Separator key
"U"
"Undo"
"Up"                    the Up arrow key (navigation keypad)
"V"
"VolumeDown"
"VolumeUp"
"W"
"WWW"                   the WWW/World Wide Web key
"X"
"Y"
"Z"
"#"                     ISO USB keyboards actually use this code instead of 49 for the same key, but all OSes I've seen treat the two codes identically. So, as an implementor, unless your keyboard generates both of those codes and your OS treats them differently, you should generate SDL_SCANCODE_BACKSLASH instead of this code. As a user, you should not rely on this code because SDL will never generate it with most (all?) keyboards.
"&"
"*"
"@"
"^"
":"
"$"
"!"
">"
"#"
"("
"<"
"%"
"+"
"?"
")"
"_"
=====================   ================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

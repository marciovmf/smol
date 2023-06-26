
# About the editor UI texture

## Description
This folder holds files used to produce assets for the editor.

 - smol_gui_icons.xcf:
    - It's a [GIMP](https://www.gimp.org/) file with the main font and icons.
 - export_iocons.py:
    - It's a python plugin for GIMP that will export all icons to a header file so the engine code can reference them.

### Using the export_icons plugin

For gimp to be able to see your plugin, you must add it to one of it's scripts
folders or add this folder as a script folder.
To do that go to **Edtit** > **Preferences** > **Folders** > **Scripts** and add this flolder path to the list.
After restarting GIMP there should be a new menu option in **File** > **Exporte layers to header file**.

CLicking in that option will display the exporter window where you can select the file name and location where to save the data.
The editor expects it to be in **/src/smoL_editor_icons.h** .

With this pluggin it's easy to add, move, replace or edit icons as needed, since their positions and names will be exported to a header file.

> ⚠️ Keep in mind that renaming icons will cause existing code to break if they are being referenced, so keep that in mind.

> ⚠️ Do NOT move the font image. It MUST be at the top left corner of the image sincethe font information requires it.


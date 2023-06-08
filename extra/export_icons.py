#!/usr/bin/env python
from gimpfu import *
from datetime import datetime
import os

def export_layer_positions(image, drawable, filepath):
    # Get the image name without extension
    image_name = os.path.splitext(os.path.basename(image.filename))[0]
    image_name = image_name.replace(" ","_").replace(".","_").replace("-","_")
    # Create the header file name
    header_filename = "{}.h".format(os.path.splitext(filepath)[0])
    # Initialize the header file contents
    header_contents = "// This file was generated automatially. Do not edit it!\n"
    header_contents += datetime.now().strftime("// Last generated: %m/%d/%Y, %H:%M:%S\n")
    header_contents += "#ifndef {}\n".format(image_name.upper() + "_H")
    header_contents += "#define {}\n\n".format(image_name.upper() + "_H")
    header_contents += "#include <smol/smol_rect.h>\n"
    header_contents += "\nnamespace smol\n{\n"
    # Iterate through all layers
    for layer in image.layers:
        # Check if the layer is a group
        if pdb.gimp_item_is_group(layer):
            # Iterate through child layers
            for child in layer.children:
                if not pdb.gimp_item_is_group(child):
                    layername = child.name.replace(" ", "_").replace(".", "_")
                    x = (child.offsets[0] - image.layers[0].offsets[0])
                    y = (child.offsets[1] - image.layers[0].offsets[1])
                    width = child.width
                    height = child.height
                    header_contents += "\t// x = {}, y = {}, w = {}, h = {}\n".format(x, y, width, height)
                    header_contents += "\tinline const Rectf Icon{}(){{ return Rectf({}f, {}f, {}f, {}f);}}\n".format(
                            layername.upper(),
                            x / float(image.width),
                            1.0 - (y / float(image.height)),
                            width / float(image.width),
                            height / float(image.height))
    # Add the closing statements to the header file contents
    header_contents += "}\n#endif\n"

    # Write the header file
    with open(header_filename, "w") as file:
        file.write(header_contents)
    # Display a message to confirm the export
    gimp.message("Layer positions exported to {}".format(header_filename))

# Register the export_layer_positions function as a GIMP plug-in
register(
    "python-fu-export-layer-positions",
    "Export layer positions to a header file",
    "Export layer positions to a header file",
    "Your Name",
    "Your Name",
    "2023",
    "<Image>/File/Export layers to header file",
    "*",
    [
        (PF_FILE, "filepath", "Save header file to:", "")
    ],
    [],
    export_layer_positions)

# Run the GIMP main loop
main()

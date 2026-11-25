# BodyParts3D import (human vessel model)

Converts BodyParts3D (BP3D) vessel meshes into the BloodVoyagerS/MEHLISSA vessel CSV format.

Output columns: ID, Kind, x_start, y_start, z_start, x_end, y_end, z_end (cm).
Kind: 0 = artery, 1 = vein (assumption, adjust KIND in the script).
Origin: center of the heart. Coordinates in cm.

## Usage

1. Download from https://dbarchive.biosciencedbc.jp/en/bodyparts3d/download.html:
   isa_BP3D_4.0_obj_99.zip, isa_parts_list_e.txt, isa_element_parts.txt
2. pip install -r requirements.txt
3. python bp3d_to_bvs.py --obj-dir isa_BP3D_4.0_obj_99 --parts-list isa_parts_list_e.txt --element-parts isa_element_parts.txt --out vessels.csv

Generated files are in model/. Raw BP3D data is not part of this repository.

## Known limitations

- Not a closed circulation yet (arteries do not connect to veins).
- Some cerebral veins are missing in the current output.
- Single male reference body (BP3D).

## Attribution

BodyParts3D, (c) The Database Center for Life Science. Licensed under CC Attribution 4.0
International (older releases: CC BY-SA 2.1 Japan). Check the current terms at
https://dbarchive.biosciencedbc.jp/en/bodyparts3d/lic.html

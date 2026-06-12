# formula solver

The formula solver allows the use of formula expressions to flexibly process LAS data.
It supports both filtering and transforming operations based on header variables or point variables.
All expressions are evaluated using the external muparser library, which provides fast and reliable 
mathematical expression parsing.
For a complete list of **muparser specific supported operators, functions, and constants**, refer to the muparser documentation: 
'https://beltoforion.de/en/muparser/features.php#idDef2'.

> **Note:** The -formula and -fileformula features are available in all LAStools modules.   
> These advanced capabilities require a full licensed build of LAStools.  
> They are not available in the open-source components of the software.  

**Key capabilities include:**
- Filtering input files: Apply formula expressions to LAS header variables to filter input files. 
- Filtering individual points: Use header and point variables in formula expressions to include or exclude points during processing.
- Transforming point or LAS header variables: Modify selected point variables (e.g., coordinates, intensity, RGB) or writable LAS header 
variables (e.g., scale factors, offsets) using formula based expressions.  

Note: '-fileformula' may only use LAS header variables.  
Note: '-formula' uses point variables for transformations or filtering and may use LAS header variables for filtering in the expressions. 
LAS header variables cannot be used as transformation targets.  

### LAS specific variables and constants

The formula solver exposes a set of point variables and LAS header variables that can be used inside formula expressions.
Some variables are writable (transformable), while others are read-only and can only be used for filtering or evaluation.  
LAS header variables are writable only with '-fileformula', not with '-formula'.  

Below is the complete list of all bound variables, their meaning, and whether they can be modified.  

**Filter keywords:**  
  keep        : Keep the point/file if the condition evaluates to true  
  drop        : Drop the point/file if the condition evaluates to true  

**Point variables:**  
  x         : Scaled x coordinate (rw)  
  y         : Scaled y coordinate (rw)  
  z         : Scaled z coordinate (rw)  
  X         : Raw unscaled X (rw)  
  Y         : Raw unscaled Y (rw)  
  Z         : Raw unscaled Z (rw)  
  i         : Intensity (rw)  
  c         : Classification (rw)  
  t         : GPS time (rw)  
  R         : Red channel (rw)  
  G         : Green channel (rw)  
  B         : Blue channel (rw)  
  I         : NIR channel (rw)  
  a         : Scan angle (rw)  
  n         : Number of returns (rw)  
  r         : Return number (rw)  
  l         : Scanner channel (rw)  
  u         : User data (rw)  
  p         : Point source ID (rw)  

  h         : Withheld flag (ro)  
  k         : Keypoint flag (ro)  
  g         : Synthetic flag (ro)  
  o         : Overlap flag (ro)  
  e         : Edge of flight line flag (ro)  
  d         : Direction of scan flag (ro)  
  m         : Point index (0‑based) (ro)  
  M         : Point index (1‑based) (ro)  
  A0..A(n)  : Extra attribute values (ro)  

**LAS header variables:**  
  hdrXmin     : Minimum X bound (rw)  
  hdrYmin     : Minimum Y bound (rw)  
  hdrZmin     : Minimum Z bound (rw)  
  hdrXmax     : Maximum X bound (rw)  
  hdrYmax     : Maximum Y bound (rw)  
  hdrZmax     : Maximum Z bound (rw)  
  hdrScaleX   : X scale factor (rw)  
  hdrScaleY   : Y scale factor (rw)  
  hdrScaleZ   : Z scale factor (rw)  
  hdrOffsX    : X offset (rw)  
  hdrOffsY    : Y offset (rw)  
  hdrOffsZ    : Z offset (rw)  
  hdrDtDay    : File creation day (rw)  
  hdrDtYear   : File creation year (rw)  
  hdrGpsMin   : Minimum GPS time (rw)  
  hdrGpsMax   : Maximum GPS time (rw)  
  hdrPcnt     : Number of point records (rw)  

  hdrGeValue  : Global encoding (ro)  
  hdrGeWkt    : WKT flag (ro)  
  hdrGeGpsStd : GPS standard flag (ro)  
  hdrGeGpsOffs: GPS offset flag (ro)  
  hdrVer      : LAS version (ro)  
  hdrVlrCnt   : Number of VLRs (ro)  
  hdrPver     : Point data format (ro)  
  hdrGpsOffs  : Time offset (ro)  

(rw) = read/write  
       The variable can be modified using formula expressions.  
       These variables support transformations.  

(ro) = read-only  
       The variable cannot be modified.  
       It can be used for filtering or conditions, but not used as targets in transformations.  

**Constants:**  
  M2FT    : Meters to Feet conversion factor (3.28083989501312)  
  FT2M    : Feet to Meters conversion factor (0.3048)  
  SFT2M   : Survey Feet to Meters conversion factor (0.3048006096012)  
  M2SFT   : Meters to Survey Feet conversion factor (3.28083333333354)  
  FT2SFT  : Feet to Survey Feet conversion factor (0.999998)  

  DEG2RAD : Degrees to Radians (0.017453292519943295)  
  RAD2DEG : Radians to Degrees (57.29577951308232)  
  GRAD2DEG: Gradians to Degrees (0.9)  
  DEG2GRAD: Degrees to Gradians (1.11111111111111)  


  CLASS_CREATED        : LAS class 0  
  CLASS_UNCLASSIFIED   : LAS class 1  
  CLASS_GROUND         : LAS class 2  
  CLASS_LOW_VEG        : LAS class 3  
  CLASS_MED_VEG        : LAS class 4  
  CLASS_HIGH_VEG       : LAS class 5  
  CLASS_BUILDING       : LAS class 6  
  CLASS_LOW_POINT      : LAS class 7  
  CLASS_MODEL_KEY_POINT: LAS class 8  
  CLASS_WATER          : LAS class 9  
  CLASS_OVERLAP        : LAS class 12  

  NO_DATA     : Constant for missing values (-9999)  
  EPS         : Small epsilon value (1e-9)  

### Examples
- In all formula expressions, the variable on the left side is the target.
- For filtering:  
      keep = <condition>  
      drop = <condition>  

- For transformations:  
      x = <expression>  
      y = <expression>  
      i = <expression>  
      R = <expression>  
      …  

- Multiple expressions can be separated by semicolons:  
      "x = x + 50; y = y + 100"  

**How to use -formula for all LAStools:**

    LAStools64 -i input.laz -o out.laz -formula "x = x + 50"

shifts all x coordinates by +50 units and writes the transformed points
to out.laz.  

    LAStools64 -i in.laz -o out.laz -formula "x = x + 50; y = y + 100; z = z + 5"

applies multiple coordinate transformations in one expression: x, y, and z
are shifted independently.  

    LAStools64 -i in.laz -o out.laz -formula "i = i * 0.5"

reduces the intensity of all points by 50 percent.  

    LAStools64 -i in.laz -o out.laz -formula "R = R * 1.1; G = G * 1.1; B = B * 1.1"

brightens all RGB values by 10 percent but the formula can produce RGB values above 255.  

    LAStools64 -i in.laz -o out.laz -formula "R = min(max(R * 1.1, 0), 255); G = min(max(G * 1.1, 0), 255); B = min(max(B * 1.1, 0), 255);"

This is generally the better Approach to brighten all RGB values by 10 percent, as it ensures that the values remain within the 8-bit range (0-255) expected by most standard viewers.  

    LAStools64 -i in.laz -o out.laz -formula "keep = x > 280800.0"

keeps only points whose x coordinate is greater than 280800.0.  

    LAStools64 -i in.laz -o out.laz -formula "drop = x >= 280800 && x <= 280900 && y > 5656500 && y < 5656750"

drops all points inside the given bounding box and keeps all others.  

    LAStools64 -i *.laz -o out.laz -formula "z = z * M2FT"

converts all z coordinates from meters to feet using the predefined
constant M2FT for all processed files.  

    LAStools64 -i in.laz -o out.laz -formula "c = (c == CLASS_LOW_VEG) ? CLASS_MED_VEG : c"

reclassifies all points of LAS class 3 (low vegetation) to class 4
(medium vegetation). The same operation can also be written using the
numeric class values directly "c = (c == 3) ? 4 : c".  



**How to use -fileformula for all LAStools:**

    LAStools64 -i *.laz -o out.laz -fileformula "drop = hdrGeWkt == 0"

drops the entire file if the LAS header indicates that no WKT
coordinate system information is present.  

    LAStools64 -i *.laz -o out.laz -fileformula "keep = hdrPcnt > 1000000"

keeps only files that contain more than one million points.  

    LAStools64 -i *.laz -o out.laz -fileformula "drop = hdrVer < 14"

drops all files whose LAS version is older than 1.4.  

    LAStools64 -i *.laz -o out.laz -fileformula "keep = hdrXmin >= 500000.0 && hdrYmin > 5200000.0"

keeps only files whose bounding box lies entirely above the given coordinate thresholds.  

    LAStools64 -i *.laz -o out.laz -fileformula "hdrDtDay = 94; hdrDtYear = 2026"

sets the file creation day to 94 and the file creation year to 2026 (04.04.2026) for all processed files.  

    LAStools64 -i in.laz -o out.laz -fileformula "hdrScaleX = 0.01; hdrScaleY = 0.01; hdrScaleZ = 0.01"

updates the LAS header scale factors to a uniform value of 0.01.  

    LAStools64 -i in.laz -o out.laz -fileformula "hdrOffsX = 500000; hdrOffsY = 5200000"

sets new coordinate offsets in the LAS header.  


## Licensing

The formula features use the muparser library for mathematical expression parsing.
muparser is released under the MIT open source license.
For more information about the license, usage terms, and the muparser project itself, refer to the official website:
https://beltoforion.de/en/muparser/

## Support

1. We invite you to join our LAStools Google Group (http://groups.google.com/group/lastools/).
   If you are looking for information about a specific tool, enter the tool name in the search 
   function and you'll find all discussions related to the respective tool. 
2. Customer Support Page: https://rapidlasso.de/customer-support/.  
3. Download LAStools: https://rapidlasso.de/downloads/.  
4. Changelog: https://rapidlasso.de/changelog/.  


If you want to send us feedback or have questions that are not answered in the resources above, 
please email to info@rapidlasso.de.

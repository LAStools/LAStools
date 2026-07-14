# formula solver

The formula solver allows the use of formula expressions to flexibly process LAS data.
The solver supports filtering functions at the file and point levels. At the point level,
filtering and transformation operations can be performed based on header variables or point variables.

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


## examples
- In all formula expressions, the variable on the left side is the target.
- For filtering:  
      keep = <condition>  
      drop = <condition>  

- For transformations:  
      x = <expression>  
      y = <expression>  
      i = <expression>  
      R = <expression>  
      ...

- Multiple expressions can be separated by semicolons:  
      "x = x + 50; y = y + 100"  

**How to use -formula for all LAStools:**

    las2las64 -i input.laz -o out.laz -formula "x=x+50"

shifts all x coordinates by +50 units and writes the transformed points
to out.laz.  


    las2las64 -i in.laz -o out.laz -formula "x=x+50;y=y+100; z=z+5"

applies multiple coordinate transformations in one expression: x, y, and z
are shifted independently.  


    las2dem_new64 -i in.laz -o out.laz -formula "i=i*0.5"

reduces the intensity of all points by 50 percent.  


    lasclip64 -i in.laz -o out.laz -formula "R=R*1.1;G=G*1.1;B=B*1.1"

brightens all RGB values by 10 percent but the formula can produce RGB values above 255.  


    lasclip64 -i in.laz -o out.laz -formula "R=min(max(R*1.1,0),0xFFFF);G=min(max(G*1.1,0),0xFFFF);B=min(max(B*1.1,0),0xFFFF);"

this brighten all RGB values by 10 percent and limit the value to the maximum RGB color range.


    lasgrid64 -i in.laz -o out.laz -formula "keep=x>280800.0"

keeps only points whose x coordinate is greater than 280800.0.  


    lasgrid64 -i in.laz -o out.laz -formula "drop=x>=280800 && x<=280900 && y>5656500 && y<5656750"

drops all points inside the given bounding box and keeps all others.  


    las2dem_new64 -i *.laz -o out.laz -formula "z=z*M2FT"

converts all z coordinates from meters to feet using the predefined
constant M2FT for all processed files.  


    las2las64 -i in.laz -o out.laz -formula "c=(c==CLASS_LOW_VEG)?CLASS_MED_VEG:c"

reclassifies all points of LAS class 3 (low vegetation) to class 4
(medium vegetation). The same operation can also be written using the
numeric class values directly "c = (c == 3) ? 4 : c".  



**How to use -fileformula for all LAStools:**

    las2las64 -i *.laz -o out.laz -fileformula "drop = hdrGeWkt == 0"

drops the entire file if the LAS header indicates that no WKT
coordinate system information is present.  


    las2las64 -i *.laz -o out.laz -fileformula "keep = hdrPcnt > 1000000"

process only files that contain more than one million points.  


    las2dem_new64 -i *.laz -o out.laz -fileformula "drop = hdrVer < 14"

drops all files whose LAS version is older than 1.4.  


    lasclip64 -i *.laz -o out.laz -fileformula "keep=hdrXmin>=500000.0 && hdrYmin>5200000.0"

process only files whose bounding box lies entirely above the given coordinate thresholds.  


    lasclip64 -i *.laz -o out.laz -fileformula "hdrDtDay=94; hdrDtYear=2026"

sets the file creation day to 94 and the file creation year to 2026 (04.04.2026) for all processed files.  


    lasgrid64 -i in.laz -o out.laz -fileformula "hdrScaleX = 0.01; hdrScaleY = 0.01; hdrScaleZ = 0.01"

updates the LAS header scale factors to a uniform value of 0.01.  


    lasgrid64 -i in.laz -o out.laz -fileformula "hdrOffsX=500000;hdrOffsY=5200000"

sets new coordinate offsets in the LAS header.   


## reference

The formula solver exposes a set of point variables and LAS header variables that can be used inside formula expressions.
Some variables are writable (transformable), while others are read-only and can only be used for filtering or evaluation.  
LAS header variables are writable only with '-fileformula', not with '-formula'.  

Below is the complete list of all bound variables, their meaning, and whether they can be modified. In addition, all available 
operators, functions and constants are listed.  

### filter keywords
  keep: Keep the point/file if the condition evaluates to true  
  drop: Drop the point/file if the condition evaluates to true  

### numbers
  numbers are decimal integer or float (. as decimal limiter) by default  
  0x   : prefix for hexadecimal numbers  
  0b   : prefix for binary numbers  

### point variables
  x    : Scaled x coordinate (rw)  
  y    : Scaled y coordinate (rw)  
  z    : Scaled z coordinate (rw)  
  X    : Raw unscaled X (rw)  
  Y    : Raw unscaled Y (rw)  
  Z    : Raw unscaled Z (rw)  
  i    : Intensity (rw)  
  c    : Classification ("2" = Ground) (rw)  
  t    : GPS time (rw)  
  R    : Red channel (rw)  
  G    : Green channel (rw)  
  B    : Blue channel (rw)  
  I    : NIR channel (rw)  
  a    : Scan angle in degrees (rw)  
  n    : Number of returns (rw)  
  r    : Return number (rw)  
  l    : Scanner channel (rw)  
  u    : User data (rw)  
  p    : Point source ID (rw)
  
  h    : Withheld flag (ro/rw)  
  k    : Keypoint flag (ro/rw)  
  g    : Synthetic flag (ro/rw)  
  o    : Overlap flag (ro/rw)  
  e    : Edge of flight line flag (ro/rw)  
  d    : Direction of scan flag (ro/rw)  
  m    : Point index (0-based) (ro)  
  M    : Point index (1-based) (ro)  
  A0..A(n): Extra attribute values (ro)  

### header variables
  hdrScaleX   : X scale factor (rw)  
  hdrScaleY   : Y scale factor (rw)  
  hdrScaleZ   : Z scale factor (rw)  
  hdrOffsX    : X offset (rw)  
  hdrOffsY    : Y offset (rw)  
  hdrOffsZ    : Z offset (rw)  
  hdrDtDay    : File creation day ("45" = 14 Feb.) (rw)  
  hdrDtYear   : File creation year ("2025") (rw)
  hdrDt       : File creation date ("20250214" = 14 Feb. 2025) (ro)  
  
  hdrXmin     : Minimum X bound (ro)  
  hdrYmin     : Minimum Y bound (ro)  
  hdrZmin     : Minimum Z bound (ro)  
  hdrXmax     : Maximum X bound (ro)  
  hdrYmax     : Maximum Y bound (ro)  
  hdrZmax     : Maximum Z bound (ro)  
  hdrGpsMin   : Minimum GPS time (ro)  
  hdrGpsMax   : Maximum GPS time (ro)  
  hdrPcnt     : Number of point records (ro) 
  hdrGeValue  : Global encoding ("17" = [WKT, GpsStd]) (ro)  
  hdrGeWkt    : WKT flag (ro)  
  hdrGeGpsStd : GPS standard flag (ro)  
  hdrGeGpsOffs: GPS offset flag (ro)  
  hdrVer      : LAS version ("12" = version 1.2) (ro)  
  hdrVlrCnt   : Number of VLRs (ro)  
  hdrPver     : Point data format (ro)  
  hdrGpsOffs  : Time offset ("604800" = +1 week offset) (ro)  

(rw) = read/write  
       The variable can be modified using formula expressions.  
       These variables support transformations.  

(ro) = read-only  
       The variable cannot be modified.  
       It can be used for filtering or conditions, but not used as targets in transformations.  

(ro/rw) = currently read-only; will be writeable in next versions  

	   
### operators  
  The priority determines the order in which operators are processed.  

```
                             priority
  =   : assignment operator         0  
  ||  : logical OR                  1  
  &&  : logical AND                 2  
  |   : bitwise OR                  3  
  &   : bitwise AND                 4  
  <=  : less or equal               5  
  >=  : greater or equal            5  
  !=  : not equal                   5  
  ==  : equal                       5  
  >   : greater than                5  
  <   : less than                   5  
  +   : addition                    6  
  -   : subtraction                 6  
  *   : multiplication              7  
  /   : division                    7  
  ^   : raise x to the power of y   8  
  ?:  : if-then-else operator. sample: p==k?3:5  (if p is equal k the result is 3, else 5)
``` 

### functions
  The brackets (n) indicate the required number n of arguments; (var.) means a variable number of arguments.  

  rnd(0)   : Generate a random number between 0 and 1  
  sin(1)   : sine function  
  cos(1)   : cosine function  
  tan(1)   : tangens function  
  asin(1)  : arcus sine function  
  acos(1)  : arcus cosine function  
  atan(1)  : arcus tangens function  
  sinh(1)  : hyperbolic sine function  
  cosh(1)  : hyperbolic cosine function  
  tanh(1)  : hyperbolic tangens function  
  asinh(1) : hyperbolic arcus sine function  
  acosh(1) : hyperbolic arcus cosine function  
  atanh(1) : hyperbolic arcus tangens function  
  log2(1)  : logarithm to the base 2  
  log10(1) : logarithm to the base 10  
  log(1)   : logarithm to base e (2.71828…)  
  ln(1)    : logarithm to base e (2.71828…)  
  exp(1)   : e raised to the power of x  
  sqrt(1)  : square root of a value  
  sign(1)  : sign function (−1 if x < 0; 1 if x > 0)  
  rint(1)  : round to nearest integer  
  abs(1)   : absolute value  
  min(var.): minimum of all arguments  
  max(var.): maximum of all arguments  
  sum(var.): sum of all arguments  
  avg(var.): mean value of all arguments  
  
### constants
  M2FT    : Meters to Feet conversion factor (3.28083989501312)  
  FT2M    : Feet to Meters conversion factor (0.3048)  
  SFT2M   : Survey Feet to Meters conversion factor (0.3048006096012)  
  M2SFT   : Meters to Survey Feet conversion factor (3.28083333333354)  
  FT2SFT  : Feet to Survey Feet conversion factor (0.999998)  

  DEG2RAD : Degrees to Radians (0.017453292519943295)  
  RAD2DEG : Radians to Degrees (57.29577951308232)  
  GRAD2DEG: Gradians to Degrees (0.9)  
  DEG2GRAD: Degrees to Gradians (1.11111111111111)  

  CLASS_CREATED        : LAS classification 0  
  CLASS_UNCLASSIFIED   : LAS classification 1  
  CLASS_GROUND         : LAS classification 2  
  CLASS_LOW_VEG        : LAS classification 3  
  CLASS_MED_VEG        : LAS classification 4  
  CLASS_HIGH_VEG       : LAS classification 5  
  CLASS_BUILDING       : LAS classification 6  
  CLASS_LOW_POINT      : LAS classification 7  
  CLASS_MODEL_KEY_POINT: LAS classification 8  
  CLASS_WATER          : LAS classification 9  
  CLASS_OVERLAP        : LAS classification 12  

  NO_DATA: Constant for missing values (-9999)  
  EPS    : Small epsilon value (1e-9)  
  
  _pi: The one and only pi (3.141592653589793238462643)  
  _e : Euler's number (2.718281828459045235360287)


## licensing

The formula features use the muparser library for mathematical expression parsing.
muparser is released under the MIT open source license.
For more information about the license, usage terms, and the muparser project itself, refer to the official website:
https://beltoforion.de/en/muparser/


## support

1. We invite you to join our LAStools Google Group (http://groups.google.com/group/lastools/).
   If you are looking for information about a specific tool, enter the tool name in the search 
   function and you'll find all discussions related to the respective tool. 
2. Customer Support Page: https://rapidlasso.de/customer-support/.  
3. Download LAStools: https://rapidlasso.de/downloads/.  
4. Changelog: https://rapidlasso.de/changelog/.  


If you want to send us feedback or have questions that are not answered in the resources above, 
please email to info@rapidlasso.de.

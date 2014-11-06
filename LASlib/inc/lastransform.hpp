/*
===============================================================================

  FILE:  lastransform.hpp
  
  CONTENTS:
  
    Transforms LIDAR points with a number of different operations.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2012, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    18 December 2011 -- added '-flip_waveform_direction' to deal with Riegl's data 
    20 March 2011 -- added -translate_raw_xyz after the fullest of full moons
    21 January 2011 -- re-created after matt told me about the optech dashmap bug
  
===============================================================================
*/
#ifndef LAS_TRANSFORM_HPP
#define LAS_TRANSFORM_HPP

#include "lasdefinitions.hpp"

class LASoperation
{
public:
  virtual const char * name() const = 0;
  virtual int get_command(char* string) const = 0;
  virtual void transform(LASpoint* point) const = 0;
  virtual ~LASoperation(){};
};

class LAStransform
{
public:

  bool change_coordinates;

  void usage() const;
  void clean();
  bool parse(int argc, char* argv[]);
  I32 unparse(char* string) const;
  inline bool active() const { return (num_operations != 0); };

  void setPointSource(U16 value);

  void transform(LASpoint* point) const;

  LAStransform();
  ~LAStransform();

private:

  void add_operation(LASoperation* operation);
  U32 num_operations;
  U32 alloc_operations;
  LASoperation** operations;
};

#endif

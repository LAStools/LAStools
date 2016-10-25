/*
===============================================================================

  FILE:  lastransform.hpp
  
  CONTENTS:
  
    Transforms LIDAR points with a number of different operations.

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2015, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     9 May 2016 -- new '-translate_raw_xy_at_random 2 2' for random pertubation
    20 April 2016 -- new '-switch_R_G', '-switch_R_B' and '-set_RGB 32768 16384 0'
    25 January 2016 -- brand-new opportunity to do a '-filtered_transform' 
    18 December 2011 -- added '-flip_waveform_direction' to deal with Riegl's data 
    20 March 2011 -- added -translate_raw_xyz after the fullest of full moons
    21 January 2011 -- re-created after matt told me about the optech dashmap bug
  
===============================================================================
*/
#ifndef LAS_TRANSFORM_HPP
#define LAS_TRANSFORM_HPP

#include "lasdefinitions.hpp"

class LASfilter;

class LASoperation
{
public:
  virtual const CHAR * name() const = 0;
  virtual int get_command(CHAR* string) const = 0;
  virtual void transform(LASpoint* point) = 0;
  virtual void reset(){};
  virtual ~LASoperation(){};
};

class LAStransform
{
public:

  bool change_coordinates;

  void usage() const;
  void clean();
  bool parse(int argc, char* argv[]);
  bool parse(CHAR* string);
  I32 unparse(CHAR* string) const;
  inline bool active() const { return (num_operations != 0); };
  inline bool filtered() const { return is_filtered; };

  void setFilter(LASfilter* filter);

  void setPointSource(U16 value);

  void transform(LASpoint* point);
  void reset();

  LAStransform();
  ~LAStransform();

private:

  void add_operation(LASoperation* operation);
  U32 num_operations;
  U32 alloc_operations;
  LASoperation** operations;
  bool is_filtered;
  LASfilter* filter;
};

#endif

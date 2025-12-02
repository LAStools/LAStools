/*
===============================================================================

	FILE:  lastransform.hpp

	CONTENTS:

		Transforms LIDAR points with a number of different operations.

	PROGRAMMERS:

		info@rapidlasso.de  -  https://rapidlasso.de

	COPYRIGHT:

		(c) 2007-2021, rapidlasso GmbH - fast tools to catch reality

		This is free software; you can redistribute and/or modify it under the
		terms of the GNU Lesser General Licence as published by the Free Software
		Foundation. See the LICENSE.txt file for more information.

		This software is distributed WITHOUT ANY WARRANTY and without even the
		implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	CHANGE HISTORY:

		10 March 2022 -- added TransformMatrix operation
		18 November 2021 -- new '-forceRGB' to use RGB values also in non-RGB point versions
		15 June 2021 -- new '-clamp_RGB_to_8bit' transform useful to avoid 8 bit overflow
		10 May 2019 -- checking for overflows in X, Y, Z 32 bit integers of fixed-point LAS
		 6 March 2018 -- changed '%g' to '%lf' for all sprintf() of F64 values
		28 February 2017 -- now '-set_RGB_of_class' also works for classifications > 31
		 1 February 2017 -- new '-copy_intensity_into_z' for use in lasgrid or lascanopy
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
#include "laszip_decompress_selective_v3.hpp"
#include <cmath>

class LASfilter;
class LASreader;

struct LASTransformMatrix {
	F64 r11;
	F64 r12;
	F64 r13;
	F64 r21;
	F64 r22;
	F64 r23;
	F64 r31;
	F64 r32;
	F64 r33;
	F64 tr1;
	F64 tr2;
	F64 tr3;
};

class LASoperation
{
public:
	virtual const CHAR * name() const = 0;
	virtual I32 get_command(CHAR* string) const = 0;
	virtual U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY; };
	inline I64 get_overflow() const { return overflow; };
	inline void zero_overflow() { overflow = 0; };
  inline void set_header(LASheader& header){ this->header = &header; };
  virtual F64* transform_coords_for_offset_adjustment(F64 x, F64 y, F64 z) = 0;
	virtual void transform(LASpoint* point) = 0;
	virtual void reset() { overflow = 0; };
	inline void set_offset_adjust(BOOL offset_adjust) { this->offset_adjust = offset_adjust; };
	void set_gps_origins(U16 orig_global_encoding, U16 orig_time_offset);
	void set_origins(F64 orig_x_offset, F64 orig_y_offset, F64 orig_z_offset, F64 orig_x_scale_factor, F64 orig_y_scale_factor, F64 orig_z_scale_factor);
  void set_scale_factor(F64 scale_factor_x, F64 scale_factor_y, F64 scale_factor_z);
  void set_adjusted_offset(F64 adjusted_offset_x, F64 adjusted_offset_y, F64 adjusted_offset_z);
  void write_transformed_values_with_adjust_offset(F64 x, F64 y, F64 z, LASpoint* point);
  F64* get_offset_adjust_coord_without_trafo_changes(F64 x, F64 y, F64 z);
  I32 get_Z(LASpoint* point); 
  void set_offset_adjust_coord_without_trafo_changes(LASpoint* point);
  inline LASoperation() {
    overflow = 0;
    orig_x_offset = 0.0;
    orig_y_offset = 0.0;
    orig_z_offset = 0.0;
	orig_time_offset = 0;
	orig_global_encoding = 0;
    adjusted_offset_x = 0.0;
    adjusted_offset_y = 0.0;
    adjusted_offset_z = 0.0;
    orig_x_scale_factor = 0.01;
    orig_y_scale_factor = 0.01;
    orig_z_scale_factor = 0.01;
    scale_factor_x = 0.01;
    scale_factor_y = 0.01;
    scale_factor_z = 0.01;
    offset_adjust = FALSE;
    header = nullptr;
   };
	virtual ~LASoperation() {};
protected:
	I64 overflow;
  F64 orig_x_offset, orig_y_offset, orig_z_offset;
  F64 orig_x_scale_factor, orig_y_scale_factor, orig_z_scale_factor;
  F64 adjusted_offset_x, adjusted_offset_y, adjusted_offset_z;
  F64 scale_factor_x, scale_factor_y, scale_factor_z;
  BOOL offset_adjust;
  U16 orig_time_offset; // LAS 1.5
  U16 orig_global_encoding;
  LASheader* header;
};

// this operation is public cause used out of reader in ptx header
class LASoperationTransformMatrix : public LASoperation
{
public:
	inline const CHAR* name() const { return "transform_matrix"; };
	inline I32 get_command(CHAR* string) const { 
		constexpr size_t buffer_size = 330; //Maximum expected length for 12xF64(double), name() + safety margin
		return snprintf(string, buffer_size, "-%s %lf,%lf,%lf %lf,%lf,%lf %lf,%lf,%lf %lf,%lf,%lf", name(), r11, r12, r13, r21, r22, r23, r31, r32, r33, tr1, tr2, tr3); };
	inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY | LASZIP_DECOMPRESS_SELECTIVE_Z; };
    inline F64* transform_coords_for_offset_adjustment(F64 x, F64 y, F64 z) {
      F64* tranformed_coord = new F64[3]{0.0, 0.0, 0.0};
      tranformed_coord[0] = x * r11 + y * r12 + z * r13 + tr1;
      tranformed_coord[1] = x * r21 + y * r22 + z * r23 + tr2;
      tranformed_coord[2] = x * r31 + y * r32 + z * r33 + tr3;
      return tranformed_coord;
    };
	inline void transform(LASpoint* point) {
    F64 xr = 0.0;
    F64 yr = 0.0;
    F64 zr = 0.0;

    if (offset_adjust) 
		{
      F64 x = point->get_X() * orig_x_scale_factor + orig_x_offset;
      F64 y = point->get_Y() * orig_y_scale_factor + orig_y_offset;
      F64 z = point->get_Z() * orig_z_scale_factor + orig_z_offset;
      xr = x * r11 + y * r12 + z * r13 + tr1;
      yr = x * r21 + y * r22 + z * r23 + tr2;
      zr = x * r31 + y * r32 + z * r33 + tr3;

      write_transformed_values_with_adjust_offset(xr, yr, zr, point);
    }
		else 
		{
      F64 x = point->get_x();
      F64 y = point->get_y();
      F64 z = point->get_z();
      xr = x * r11 + y * r12 + z * r13 + tr1;
      yr = x * r21 + y * r22 + z * r23 + tr2;
      zr = x * r31 + y * r32 + z * r33 + tr3;

      if (!point->set_x(xr)) 
      {
        overflow++;
      }
      if (!point->set_y(yr)) 
      {
        overflow++;
      }
      if (!point->set_z(zr)) 
      {
        overflow++;
      }
    }
	};
	LASoperationTransformMatrix(F64 r11, F64 r12, F64 r13, F64 r21, F64 r22, F64 r23, F64 r31, F64 r32, F64 r33, F64 tr1, F64 tr2, F64 tr3)
	{
		this->r11 = r11; this->r12 = r12; this->r13 = r13;
		this->r21 = r21; this->r22 = r22; this->r23 = r23;
		this->r31 = r31; this->r32 = r32; this->r33 = r33;
		this->tr1 = tr1; this->tr2 = tr2; this->tr3 = tr3;
	};
	LASoperationTransformMatrix(LASTransformMatrix tm)
	{
		this->r11 = tm.r11; this->r12 = tm.r12; this->r13 = tm.r13;
		this->r21 = tm.r21; this->r22 = tm.r22; this->r23 = tm.r23;
		this->r31 = tm.r31; this->r32 = tm.r32; this->r33 = tm.r33;
		this->tr1 = tm.tr1; this->tr2 = tm.tr2; this->tr3 = tm.tr3;
	};
private:
	F64 r11, r12, r13, r21, r22, r23, r31, r32, r33, tr1, tr2, tr3;
};

class LASoperationMultiplyScaledIntensityRangeIntoRGB : public LASoperation
{
 public:
  inline const CHAR* name() const
  {
    return "multiply_scaled_intensity_range_into_RGB";
  };
  inline I32 get_command(CHAR* string) const
  {
    return sprintf(string, "-%s %f ", name(), scale);
  };
  inline U32 get_decompress_selective() const
  {
    return LASZIP_DECOMPRESS_SELECTIVE_INTENSITY | LASZIP_DECOMPRESS_SELECTIVE_RGB;
  };
  inline F64* transform_coords_for_offset_adjustment(F64 x, F64 y, F64 z) 
	{
    LASMessage(LAS_WARNING, "The adjustment of the offset using '-offset_adjust' is not supported for the operation '%s' yet.", name());
    return 0;
  };
  inline void transform(LASpoint* point)
  {
    if ((intensityRange != 0) && (scale != 0))
    {
      U16 val = U16_CLAMP(std::round(scale * 0xFFFF * (point->get_intensity() - intensityMin) / intensityRange));
      point->rgb[0] = val;
      point->rgb[1] = val;
      point->rgb[2] = val;
    }
  };
  LASoperationMultiplyScaledIntensityRangeIntoRGB(F32 scale)
  {
    this->scale = scale;
  };
  I16 intensityMin = 0;
  I16 intensityRange = 0;

 private:
  F32 scale;
};

#define LASTRANSFORM_X_COORDINATE 0x00000001
#define LASTRANSFORM_Y_COORDINATE 0x00000002
#define LASTRANSFORM_Z_COORDINATE 0x00000004
#define LASTRANSFORM_INTENSITY    0x00000008
#define LASTRANSFORM_RGB          0x00020000
#define LASTRANSFORM_NIR          0x02000000

#define LASTRANSFORM_XY_COORDINATE (LASTRANSFORM_X_COORDINATE | LASTRANSFORM_Y_COORDINATE)
#define LASTRANSFORM_XYZ_COORDINATE (LASTRANSFORM_XY_COORDINATE | LASTRANSFORM_Z_COORDINATE)

class LAStransform
{
public:
  bool needPreread;
	U32 transformed_fields;
	F64 registers[16];

	void usage() const;
	void clean();
	BOOL parse(int argc, char* argv[]);
	BOOL parse(CHAR* string);
	I32 unparse(CHAR* string) const;
	inline BOOL active() const { return (num_operations != 0); };
	U32 get_decompress_selective() const;
	inline BOOL filtered() const { return is_filtered; };

	void setFilter(LASfilter* filter);

	void setPointSource(U16 value);
	void unsetPointSource();

	void transform(LASpoint* point);

	void check_for_overflow() const;

	void reset();
  void add_operation(LASoperation* operation);
  void adjust_offset(LASreader* lasreader, F64* scale_factor);
  void addHeaderInfo(LASreader* lasreader);
  template <typename T>
  bool find_operation(T*& op)
  {
    U32 i;
    for (i = 0; i < num_operations; i++)
    {
      if (T* castedObject = dynamic_cast<T*>(operations[i]))
      {
        op = castedObject;
        return true;
      }
    }
    return false;
  }

	LAStransform();
	~LAStransform();

private:

	void delete_operation(const CHAR* name);
	U32 num_operations;
	U32 alloc_operations;
	LASoperation** operations;
	BOOL is_filtered;
	LASfilter* filter;
};

#endif

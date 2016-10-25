/*
===============================================================================

  FILE:  lastransform.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the LICENSE.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "lastransform.hpp"

#include "lasfilter.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

class LASoperationTranslateX : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_x"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), offset); };
  inline void transform(LASpoint* point) {
    point->set_x(point->get_x() + offset);
  };
  LASoperationTranslateX(F64 offset) { this->offset = offset; };
private:
  F64 offset;
};

class LASoperationTranslateY : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_y"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), offset); };
  inline void transform(LASpoint* point) {
    point->set_y(point->get_y() + offset);
  };
  LASoperationTranslateY(F64 offset) { this->offset = offset; };
private:
  F64 offset;
};

class LASoperationTranslateZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), offset); };
  inline void transform(LASpoint* point) {
    point->set_z(point->get_z() + offset);
  };
  LASoperationTranslateZ(F64 offset) { this->offset = offset; };
private:
  F64 offset;
};

class LASoperationTranslateXYZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_xyz"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g %g ", name(), offset[0], offset[1], offset[2]); };
  inline void transform(LASpoint* point) {
    point->set_x(point->get_x() + offset[0]);
    point->set_y(point->get_y() + offset[1]);
    point->set_z(point->get_z() + offset[2]);
  };
  LASoperationTranslateXYZ(F64 x_offset, F64 y_offset, F64 z_offset) { this->offset[0] = x_offset; this->offset[1] = y_offset; this->offset[2] = z_offset; };
private:
  F64 offset[3];
};

class LASoperationScaleX : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_x"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), scale); };
  inline void transform(LASpoint* point) {
    point->set_x(point->get_x() * scale);
  };
  LASoperationScaleX(F64 scale) { this->scale = scale; };
private:
  F64 scale;
};

class LASoperationScaleY : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_y"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), scale); };
  inline void transform(LASpoint* point) {
    point->set_y(point->get_y() * scale);
  };
  LASoperationScaleY(F64 scale) { this->scale = scale; };
private:
  F64 scale;
};

class LASoperationScaleZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), scale); };
  inline void transform(LASpoint* point) {
    point->set_z(point->get_z() * scale);
  };
  LASoperationScaleZ(F64 scale) { this->scale = scale; };
private:
  F64 scale;
};

class LASoperationScaleXYZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_xyz"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g %g ", name(), scale[0], scale[1], scale[2]); };
  inline void transform(LASpoint* point) {
    point->set_x(point->get_x() * scale[0]);
    point->set_y(point->get_y() * scale[1]);
    point->set_z(point->get_z() * scale[2]);
  };
  LASoperationScaleXYZ(F64 x_scale, F64 y_scale, F64 z_scale) { this->scale[0] = x_scale; this->scale[1] = y_scale; this->scale[2] = z_scale; };
private:
  F64 scale[3];
};

class LASoperationTranslateThenScaleX : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_then_scale_x"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g ", name(), offset, scale); };
  inline void transform(LASpoint* point) {
    point->set_x((point->get_x()+offset)*scale);
  };
  LASoperationTranslateThenScaleX(F64 offset, F64 scale) { this->offset = offset; this->scale = scale; };
private:
  F64 offset;
  F64 scale;
};

class LASoperationTranslateThenScaleY : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_then_scale_y"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g ", name(), offset, scale); };
  inline void transform(LASpoint* point) {
    point->set_y((point->get_y()+offset)*scale);
  };
  LASoperationTranslateThenScaleY(F64 offset, F64 scale) { this->offset = offset; this->scale = scale; };
private:
  F64 offset;
  F64 scale;
};

class LASoperationTranslateThenScaleZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_then_scale_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g ", name(), offset, scale); };
  inline void transform(LASpoint* point) {
    point->set_z((point->get_z()+offset)*scale);
  };
  LASoperationTranslateThenScaleZ(F64 offset, F64 scale) { this->offset = offset; this->scale = scale; };
private:
  F64 offset;
  F64 scale;
};

class LASoperationRotateXY : public LASoperation
{
public:
  inline const CHAR* name() const { return "rotate_xy"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g %g ", name(), angle, x_offset, y_offset); };
  inline void transform(LASpoint* point) {
    F64 x = point->get_x() - x_offset;
    F64 y = point->get_y() - y_offset;
    point->set_x(cos_angle*x - sin_angle*y + x_offset);
    point->set_y(cos_angle*y + sin_angle*x + y_offset);
  };
  LASoperationRotateXY(F64 angle, F64 x_offset, F64 y_offset) { this->angle = angle; this->x_offset = x_offset; this->y_offset = y_offset; cos_angle = cos(3.141592653589793238462643383279502884197169/180*angle); sin_angle = sin(3.141592653589793238462643383279502884197169/180*angle); };
private:
  F64 angle;
  F64 x_offset, y_offset;
  F64 cos_angle, sin_angle;
};

class LASoperationRotateXZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "rotate_xz"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g %g ", name(), angle, x_offset, z_offset); };
  inline void transform(LASpoint* point) {
    F64 x = point->get_x() - x_offset;
    F64 z = point->get_z() - z_offset;
    point->set_x(cos_angle*x - sin_angle*z + x_offset);
    point->set_z(cos_angle*z + sin_angle*x + z_offset);
  };
  LASoperationRotateXZ(F64 angle, F64 x_offset, F64 z_offset) { this->angle = angle; this->x_offset = x_offset; this->z_offset = z_offset; cos_angle = cos(3.141592653589793238462643383279502884197169/180*angle); sin_angle = sin(3.141592653589793238462643383279502884197169/180*angle); };
private:
  F64 angle;
  F64 x_offset, z_offset;
  F64 cos_angle, sin_angle;
};

class LASoperationClampZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "clamp_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g ", name(), below, above); };
  inline void transform(LASpoint* point) {
    F64 z = point->get_z();
    if (z < below) point->set_z(below);
    else if (z > above) point->set_z(above);
  };
  LASoperationClampZ(F64 below, F64 above) { this->below = below; this->above = above; };
private:
  F64 below, above;
};

class LASoperationClampZbelow : public LASoperation
{
public:
  inline const CHAR* name() const { return "clamp_z_below"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), below); };
  inline void transform(LASpoint* point) {
    F64 z = point->get_z();
    if (z < below) point->set_z(below);
  };
  LASoperationClampZbelow(F64 below) { this->below = below; };
private:
  F64 below;
};

class LASoperationClampZabove : public LASoperation
{
public:
  inline const CHAR* name() const { return "clamp_z_above"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), above); };
  inline void transform(LASpoint* point) {
    F64 z = point->get_z();
    if (z > above) point->set_z(above);
  };
  LASoperationClampZabove(F64 above) { this->above = above; };
private:
  F64 above;
};

class LASoperationCopyAttributeIntoZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "copy_attribute_into_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), index); };
  inline void transform(LASpoint* point) {
    F64 z = point->get_attribute_as_float(index);
    point->set_z(z);
  };
  LASoperationCopyAttributeIntoZ(I32 index) { this->index = index; };
private:
  I32 index;
};

class LASoperationTranslateRawX : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_raw_x"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), offset); };
  inline void transform(LASpoint* point) {
    point->set_X(point->get_X() + offset);
  };
  LASoperationTranslateRawX(I32 offset) { this->offset = offset; };
private:
  I32 offset;
};

class LASoperationTranslateRawY : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_raw_y"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), offset); };
  inline void transform(LASpoint* point) {
    point->set_Y(point->get_Y() + offset);
  };
  LASoperationTranslateRawY(I32 offset) { this->offset = offset; };
private:
  I32 offset;
};

class LASoperationTranslateRawZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_raw_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), offset); };
  inline void transform(LASpoint* point) {
    point->set_Z(point->get_Z() + offset);
  };
  LASoperationTranslateRawZ(I32 offset) { this->offset = offset; };
private:
  I32 offset;
};

class LASoperationTranslateRawXYZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_raw_xyz"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d %d ", name(), raw_offset[0], raw_offset[1], raw_offset[2]); };
  inline void transform(LASpoint* point) {
    point->set_X(point->get_X() + raw_offset[0]);
    point->set_Y(point->get_Y() + raw_offset[1]);
    point->set_Z(point->get_Z() + raw_offset[2]);
  };
  LASoperationTranslateRawXYZ(I32 raw_x_offset, I32 raw_y_offset, I32 raw_z_offset) { this->raw_offset[0] = raw_x_offset; this->raw_offset[1] = raw_y_offset; this->raw_offset[2] = raw_z_offset; };
private:
  I32 raw_offset[3];
};

class LASoperationTranslateRawXYatRandom : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_raw_xy_at_random"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), max_raw_offset[0], max_raw_offset[1]); };
  inline void transform(LASpoint* point) {
    I32 r;
    srand(seed);
    seed = rand();
    r = ((seed >> 3) % ((2 * max_raw_offset[0]) + 1)) - max_raw_offset[0];
    point->set_X(point->get_X() + r);
    r = ((seed >> 6) % ((2 * max_raw_offset[1]) + 1)) - max_raw_offset[1];
    point->set_Y(point->get_Y() + r);
  };
  void reset() { seed = 0; };
  LASoperationTranslateRawXYatRandom(I32 max_raw_x_offset, I32 max_raw_y_offset) { seed = 0; max_raw_offset[0] = max_raw_x_offset; max_raw_offset[1] = max_raw_y_offset; };
private:
  U32 seed;
  I32 max_raw_offset[2];
};

class LASoperationClampRawZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "clamp_raw_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), below, above); };
  inline void transform(LASpoint* point) {
    if (point->get_Z() < below) point->set_Z(below);
    else if (point->get_Z() > above) point->set_Z(above);
  };
  LASoperationClampRawZ(I32 below, I32 above) { this->below = below; this->above = above; };
private:
  I32 below, above;
};

class LASoperationSetIntensity : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_intensity"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), (I32)intensity); };
  inline void transform(LASpoint* point) {
    point->set_intensity(intensity);
  };
  LASoperationSetIntensity(U16 intensity) { this->intensity = intensity; };
private:
  U16 intensity;
};

class LASoperationScaleIntensity : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_intensity"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), scale); };
  inline void transform(LASpoint* point) {
    F32 intensity = scale*point->intensity;
    point->intensity = U16_CLAMP((I32)intensity);
  };
  LASoperationScaleIntensity(F32 scale) { this->scale = scale; };
private:
  F32 scale;
};

class LASoperationTranslateIntensity : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_intensity"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), offset); };
  inline void transform(LASpoint* point) {
    F32 intensity = offset+point->intensity;
    point->intensity = U16_CLAMP((I32)intensity);
  };
  LASoperationTranslateIntensity(F32 offset) { this->offset = offset; };
private:
  F32 offset;
};

class LASoperationTranslateThenScaleIntensity : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_then_scale_intensity"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g ", name(), offset, scale); };
  inline void transform(LASpoint* point) {
    F32 intensity = (offset+point->intensity)*scale;
    point->intensity = U16_CLAMP((I32)intensity);
  };
  LASoperationTranslateThenScaleIntensity(F32 offset, F32 scale) { this->offset = offset; this->scale = scale; };
private:
  F32 offset;
  F32 scale;
};

class LASoperationClampIntensity : public LASoperation
{
public:
  inline const CHAR* name() const { return "clamp_intensity"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %u %u ", name(), (U32)below, (U32)above); };
  inline void transform(LASpoint* point) {
    if (point->intensity > above ) point->intensity = above;
    else if (point->intensity < below ) point->intensity = below;
  };
  LASoperationClampIntensity(U16 below, U16 above) { this->below = below; this->above = above; };
private:
  U16 below;
  U16 above;
};

class LASoperationClampIntensityBelow : public LASoperation
{
public:
  inline const CHAR* name() const { return "clamp_intensity_below"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %u ", name(), (U32)below); };
  inline void transform(LASpoint* point) {
    if (point->intensity < below ) point->intensity = below;
  };
  LASoperationClampIntensityBelow(U16 below) { this->below = below; };
private:
  U16 below;
};

class LASoperationClampIntensityAbove : public LASoperation
{
public:
  inline const CHAR* name() const { return "clamp_intensity_above"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %u ", name(), (U32)above); };
  inline void transform(LASpoint* point) {
    if (point->intensity > above ) point->intensity = above;
  };
  LASoperationClampIntensityAbove(U16 above) { this->above = above; };
private:
  U16 above;
};

class LASoperationScaleScanAngle : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_scan_angle"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), scale); };
  inline void transform(LASpoint* point) {
    F32 scan_angle_rank = scale*point->scan_angle_rank;
    point->scan_angle_rank = I8_CLAMP(I32_QUANTIZE(scan_angle_rank));
  };
  LASoperationScaleScanAngle(F32 scale) { this->scale = scale; };
private:
  F32 scale;
};

class LASoperationTranslateScanAngle : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_scan_angle"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), offset); };
  inline void transform(LASpoint* point) {
    F32 scan_angle_rank = offset+point->scan_angle_rank;
    point->scan_angle_rank = I8_CLAMP(I32_QUANTIZE(scan_angle_rank));
  };
  LASoperationTranslateScanAngle(F32 offset) { this->offset = offset; };
private:
  F32 offset;
};

class LASoperationTranslateThenScaleScanAngle : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_then_scale_scan_angle"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g ", name(), offset, scale); };
  inline void transform(LASpoint* point) {
    F32 scan_angle_rank = (offset+point->scan_angle_rank)*scale;
    point->scan_angle_rank = I8_CLAMP(I32_QUANTIZE(scan_angle_rank));
  };
  LASoperationTranslateThenScaleScanAngle(F32 offset, F32 scale) { this->offset = offset; this->scale = scale; };
private:
  F32 offset;
  F32 scale;
};

class LASoperationSetClassification : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_classification"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), classification); };
  inline void transform(LASpoint* point) { if (classification > 31) point->extended_classification = classification; else point->classification = classification; };
  LASoperationSetClassification(U8 classification) { this->classification = classification; };
private:
  U8 classification;
};

class LASoperationSetExtendedClassification : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_extended_classification"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), classification); };
  inline void transform(LASpoint* point) { point->extended_classification = classification; };
  LASoperationSetExtendedClassification(U8 classification) { this->classification = classification; };
private:
  U8 classification;
};

class LASoperationChangeClassificationFromTo : public LASoperation
{
public:
  inline const CHAR* name() const { return "change_classification_from_to"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), class_from, class_to); };
  inline void transform(LASpoint* point) {
    if (class_from > 31)
    {
      if (point->extended_classification == class_from)
      {
        if (class_to > 31)
        {
          point->extended_classification = class_to;
          point->classification = 0;
        }
        else
        {
          point->classification = class_to;
          point->extended_classification = 0;
        }
      }
    }
    else if (point->classification == class_from)
    {
      if (class_to > 31)
      {
        point->extended_classification = class_to;
        point->classification = 0;
      }
      else
      {
        point->classification = class_to;
        point->extended_classification = 0;
      }
    }
  };
  LASoperationChangeClassificationFromTo(U8 class_from, U8 class_to) { this->class_from = class_from; this->class_to = class_to; };
private:
  U8 class_from;
  U8 class_to;
};

class LASoperationMoveAncientToExtendedClassification : public LASoperation
{
public:
  inline const CHAR* name() const { return "move_ancient_to_extended_classification"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) {
    if (point->get_withheld_flag() || point->get_keypoint_flag() || point->get_synthetic_flag())
    {
      point->set_extended_classification((point->get_withheld_flag() ? 128 : 0) | (point->get_keypoint_flag() ? 64 : 0) | (point->get_synthetic_flag() ? 32 : 0) | point->get_classification());
      point->set_classification(0);
      point->set_synthetic_flag(0);
      point->set_keypoint_flag(0);
      point->set_withheld_flag(0);
    }
  };
  LASoperationMoveAncientToExtendedClassification() { };
};

class LASoperationChangeExtendedClassificationFromTo : public LASoperation
{
public:
  inline const CHAR* name() const { return "change_extended_classification_from_to"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), class_from, class_to); };
  inline void transform(LASpoint* point) {
    if (point->extended_classification == class_from)
    {
      point->extended_classification = class_to;
    }
  };
  LASoperationChangeExtendedClassificationFromTo(U8 class_from, U8 class_to) { this->class_from = class_from; this->class_to = class_to; };
private:
  U8 class_from;
  U8 class_to;
};

class LASoperationClassifyZbelowAs : public LASoperation
{
public:
  inline const CHAR* name() const { return "classify_z_below_as"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %d ", name(), z_below, class_to); };
  inline void transform(LASpoint* point) {
    if (point->get_z() < z_below)
    {
      if (class_to >= 32)
      {
        point->classification = class_to;
      }
      else
      {
        point->classification = (point->classification & 224) | class_to;
      }
    }
  };
  LASoperationClassifyZbelowAs(F64 z_below, U8 class_to) { this->z_below = z_below; this->class_to = class_to; };
private:
  F64 z_below;
  U8 class_to;
};

class LASoperationClassifyZaboveAs : public LASoperation
{
public:
  inline const CHAR* name() const { return "classify_z_above_as"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %d ", name(), z_above, class_to); };
  inline void transform(LASpoint* point) {
    if (point->get_z() > z_above)
    {
      if (class_to >= 32)
      {
        point->classification = class_to;
      }
      else
      {
        point->classification = (point->classification & 224) | class_to;
      }
    }
  };
  LASoperationClassifyZaboveAs(F64 z_above, U8 class_to) { this->z_above = z_above; this->class_to = class_to; };
private:
  F64 z_above;
  U8 class_to;
};

class LASoperationClassifyZbetweenAs : public LASoperation
{
public:
  inline const CHAR* name() const { return "classify_z_between_as"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g %d ", name(), z_below, z_above, class_to); };
  inline void transform(LASpoint* point) {
    if ((z_below <= point->get_z()) && (point->get_z() <= z_above))
    {
      if (class_to >= 32)
      {
        point->classification = class_to;
      }
      else
      {
        point->classification = (point->classification & 224) | class_to;
      }
    }
  };
  LASoperationClassifyZbetweenAs(F64 z_below, F64 z_above, U8 class_to) { this->z_below = z_below; this->z_above = z_above; this->class_to = class_to; };
private:
  F64 z_below;
  F64 z_above;
  U8 class_to;
};

class LASoperationClassifyIntensityBelowAs : public LASoperation
{
public:
  inline const CHAR* name() const { return "classify_intensity_below_as"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), (I32)intensity_below, (I32)class_to); };
  inline void transform(LASpoint* point) {
    if (point->intensity < intensity_below)
    {
      if (class_to >= 32)
      {
        point->classification = class_to;
      }
      else
      {
        point->classification = (point->classification & 224) | class_to;
      }
    }
  };
  LASoperationClassifyIntensityBelowAs(U16 intensity_below, U8 class_to) { this->intensity_below = intensity_below; this->class_to = class_to; };
private:
  U16 intensity_below;
  U8 class_to;
};

class LASoperationClassifyIntensityAboveAs : public LASoperation
{
public:
  inline const CHAR* name() const { return "classify_intensity_above_as"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), (I32)intensity_above, (I32)class_to); };
  inline void transform(LASpoint* point) {
    if (point->intensity > intensity_above)
    {
      if (class_to >= 32)
      {
        point->classification = class_to;
      }
      else
      {
        point->classification = (point->classification & 224) | class_to;
      }
    }
  };
  LASoperationClassifyIntensityAboveAs(U16 intensity_above, U8 class_to) { this->intensity_above = intensity_above; this->class_to = class_to; };
private:
  U16 intensity_above;
  U8 class_to;
};

class LASoperationSetWithheldFlag : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_withheld_flag"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), flag); };
  inline void transform(LASpoint* point) { point->set_withheld_flag(flag); };
  LASoperationSetWithheldFlag(U8 flag) { this->flag = (flag ? 1 : 0); };
private:
  U8 flag;
};

class LASoperationSetSyntheticFlag : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_synthetic_flag"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), flag); };
  inline void transform(LASpoint* point) { point->set_synthetic_flag(flag); };
  LASoperationSetSyntheticFlag(U8 flag) { this->flag = (flag ? 1 : 0); };
private:
  U8 flag;
};

class LASoperationSetKeypointFlag : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_keypoint_flag"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), flag); };
  inline void transform(LASpoint* point) { point->set_keypoint_flag(flag); };
  LASoperationSetKeypointFlag(U8 flag) { this->flag = (flag ? 1 : 0); };
private:
  U8 flag;
};

class LASoperationSetExtendedOverlapFlag : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_extended_overlap_flag"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), flag); };
  inline void transform(LASpoint* point) { point->set_extended_overlap_flag(flag); };
  LASoperationSetExtendedOverlapFlag(U8 flag) { this->flag = (flag ? 1 : 0); };
private:
  U8 flag;
};

class LASoperationSetExtendedScannerChannel : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_extended_scanner_channel"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), channel); };
  inline void transform(LASpoint* point) { point->set_extended_scanner_channel(channel); };
  LASoperationSetExtendedScannerChannel(U8 channel) { this->channel = (channel >= 3 ? 3 : channel); };
private:
  U8 channel;
};

class LASoperationSetUserData : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_user_data"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), user_data); };
  inline void transform(LASpoint* point) { point->user_data = user_data; };
  LASoperationSetUserData(U8 user_data) { this->user_data = user_data; };
private:
  U8 user_data;
};

class LASoperationChangeUserDataFromTo : public LASoperation
{
public:
  inline const CHAR* name() const { return "change_user_data_from_to"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), user_data_from, user_data_to); };
  inline void transform(LASpoint* point) { if (point->user_data == user_data_from) point->user_data = user_data_to; };
  LASoperationChangeUserDataFromTo(U8 user_data_from, U8 user_data_to) { this->user_data_from = user_data_from; this->user_data_to = user_data_to; };
private:
  U8 user_data_from;
  U8 user_data_to;
};

class LASoperationSetPointSource : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_point_source"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), psid); };
  inline void transform(LASpoint* point) { point->point_source_ID = psid; };
  LASoperationSetPointSource(U16 psid) { this->psid = psid; };
private:
  U16 psid;
};

class LASoperationChangePointSourceFromTo : public LASoperation
{
public:
  inline const CHAR* name() const { return "change_point_source_from_to"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), psid_from, psid_to); };
  inline void transform(LASpoint* point) { if (point->point_source_ID == psid_from) point->point_source_ID = psid_to; };
  LASoperationChangePointSourceFromTo(U16 psid_from, U16 psid_to) { this->psid_from = psid_from; this->psid_to = psid_to; };
private:
  U16 psid_from;
  U16 psid_to;
};

class LASoperationRepairZeroReturns : public LASoperation
{
public:
  inline const CHAR* name() const { return "repair_zero_returns"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { if (point->number_of_returns == 0) point->number_of_returns = 1; if (point->return_number == 0) point->return_number = 1; };
};

class LASoperationSetReturnNumber : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_return_number"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), return_number); };
  inline void transform(LASpoint* point) { point->return_number = return_number; };
  LASoperationSetReturnNumber(U8 return_number) { this->return_number = return_number; };
private:
  U8 return_number;
};

class LASoperationSetExtendedReturnNumber : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_extended_return_number"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), extended_return_number); };
  inline void transform(LASpoint* point) { point->extended_return_number = extended_return_number; };
  LASoperationSetExtendedReturnNumber(U8 extended_return_number) { this->extended_return_number = extended_return_number; };
private:
  U8 extended_return_number;
};

class LASoperationChangeReturnNumberFromTo : public LASoperation
{
public:
  inline const CHAR* name() const { return "change_return_number_from_to"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), return_number_from, return_number_to); };
  inline void transform(LASpoint* point) { if (point->return_number == return_number_from) point->return_number = return_number_to; };
  LASoperationChangeReturnNumberFromTo(U8 return_number_from, U8 return_number_to) { this->return_number_from = return_number_from; this->return_number_to = return_number_to; };
private:
  U8 return_number_from;
  U8 return_number_to;
};

class LASoperationSetNumberOfReturns : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_number_of_returns"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), number_of_returns); };
  inline void transform(LASpoint* point) { point->number_of_returns = number_of_returns; };
  LASoperationSetNumberOfReturns(U8 number_of_returns) { this->number_of_returns = number_of_returns; };
private:
  U8 number_of_returns;
};

class LASoperationSetExtendedNumberOfReturns : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_extended_number_of_returns"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), extended_number_of_returns); };
  inline void transform(LASpoint* point) { point->extended_number_of_returns = extended_number_of_returns; };
  LASoperationSetExtendedNumberOfReturns(U8 extended_number_of_returns) { this->extended_number_of_returns = extended_number_of_returns; };
private:
  U8 extended_number_of_returns;
};

class LASoperationChangeNumberOfReturnsFromTo : public LASoperation
{
public:
  inline const CHAR* name() const { return "change_number_of_returns_from_to"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d ", name(), number_of_returns_from, number_of_returns_to); };
  inline void transform(LASpoint* point) { if (point->number_of_returns == number_of_returns_from) point->number_of_returns = number_of_returns_to; };
  LASoperationChangeNumberOfReturnsFromTo(U8 number_of_returns_from, U8 number_of_returns_to) { this->number_of_returns_from = number_of_returns_from; this->number_of_returns_to = number_of_returns_to; };
private:
  U8 number_of_returns_from;
  U8 number_of_returns_to;
};

class LASoperationSetGpsTime : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_gps_time"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), gps_time); };
  inline void transform(LASpoint* point) { point->gps_time = gps_time; };
  LASoperationSetGpsTime(F64 gps_time) { this->gps_time = gps_time; };
private:
  F64 gps_time;
};

class LASoperationTranslateGpsTime : public LASoperation
{
public:
  inline const CHAR* name() const { return "translate_gps_time"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g ", name(), offset); };
  inline void transform(LASpoint* point) { point->gps_time += offset; };
  LASoperationTranslateGpsTime(F64 offset) { this->offset = offset; };
private:
  F64 offset;
};

class LASoperationConvertAdjustedGpsToWeek : public LASoperation
{
public:
  inline const CHAR* name() const { return "adjusted_to_week"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point)
  {
    I32 week = (I32)(point->gps_time/604800.0 + 1653.4391534391534391534391534392);
    I32 secs = week*604800 - 1000000000;
    point->gps_time -= secs;
  };
};

class LASoperationConvertWeekToAdjustedGps : public LASoperation
{
public:
  inline const CHAR* name() const { return "week_to_adjusted"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d ", name(), week); };
  inline void transform(LASpoint* point) { point->gps_time += delta_secs; }
  LASoperationConvertWeekToAdjustedGps(I32 week) { this->week = week; delta_secs = week; delta_secs *= 604800; delta_secs -= 1000000000; };
private:
  I32 week;
  I64 delta_secs;
};

class LASoperationSetRGB : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_RGB"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d %d ", name(), RGB[0], RGB[1], RGB[2]); };
  inline void transform(LASpoint* point) { point->set_RGB(RGB); };
  LASoperationSetRGB(U16 R, U16 G, U16 B) { RGB[0] = R; RGB[1] = G; RGB[2] = B; };
private:
  U16 RGB[3];
};

class LASoperationSetRGBofClass : public LASoperation
{
public:
  inline const CHAR* name() const { return "set_RGB_of_class"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d %d %d %d ", name(), c, RGB[0], RGB[1], RGB[2]); };
  inline void transform(LASpoint* point) { if (point->get_classification() == c) point->set_RGB(RGB); };
  LASoperationSetRGBofClass(U8 c, U16 R, U16 G, U16 B) { this->c = c; RGB[0] = R; RGB[1] = G; RGB[2] = B; };
private:
  U8 c;
  U16 RGB[3];
};

class LASoperationScaleRGB : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_RGB"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g %g %g ", name(), scale[0], scale[1], scale[2]); };
  inline void transform(LASpoint* point) { point->rgb[0] = U16_CLAMP(scale[0]*point->rgb[0]); point->rgb[1] = U16_CLAMP(scale[1]*point->rgb[1]); point->rgb[2] = U16_CLAMP(scale[2]*point->rgb[2]); };
  LASoperationScaleRGB(F32 scale_R, F32 scale_G, F32 scale_B) { scale[0] = scale_R; scale[1] = scale_G; scale[2] = scale_B; };
private:
  F32 scale[3];
};

class LASoperationScaleRGBdown : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_RGB_down"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { point->rgb[0] = point->rgb[0]/256; point->rgb[1] = point->rgb[1]/256; point->rgb[2] = point->rgb[2]/256; };
};

class LASoperationScaleRGBup : public LASoperation
{
public:
  inline const CHAR* name() const { return "scale_RGB_up"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { point->rgb[0] = point->rgb[0]*256; point->rgb[1] = point->rgb[1]*256; point->rgb[2] = point->rgb[2]*256; };
};

class LASoperationSwitchXY : public LASoperation
{
public:
  inline const CHAR* name() const { return "switch_x_y"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { I32 temp = point->get_X(); point->set_X(point->get_Y()); point->set_Y(temp); };
};

class LASoperationSwitchXZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "switch_x_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { I32 temp = point->get_X(); point->set_X(point->get_Z()); point->set_Z(temp); };
};

class LASoperationSwitchYZ : public LASoperation
{
public:
  inline const CHAR* name() const { return "switch_y_z"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { I32 temp = point->get_Y(); point->set_Y(point->get_Z()); point->set_Z(temp); };
};

class LASoperationSwitchRG : public LASoperation
{
public:
  inline const CHAR* name() const { return "switch_R_G"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { I16 temp = point->get_R(); point->set_R(point->get_G()); point->set_G(temp); };
};

class LASoperationSwitchRB : public LASoperation
{
public:
  inline const CHAR* name() const { return "switch_R_B"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { I16 temp = point->get_R(); point->set_R(point->get_B()); point->set_B(temp); };
};

class LASoperationSwitchGB : public LASoperation
{
public:
  inline const CHAR* name() const { return "switch_G_B"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { I16 temp = point->get_G(); point->set_G(point->get_B()); point->set_B(temp); };
};

class LASoperationCopyRintoNIR : public LASoperation
{
public:
  inline const CHAR* name() const { return "copy_R_into_NIR"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { point->set_NIR(point->get_R()); };
};

class LASoperationCopyGintoNIR : public LASoperation
{
public:
  inline const CHAR* name() const { return "copy_G_into_NIR"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { point->set_NIR(point->get_G()); };
};

class LASoperationCopyBintoNIR : public LASoperation
{
public:
  inline const CHAR* name() const { return "copy_B_into_NIR"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { point->set_NIR(point->get_B()); };
};

class LASoperationFlipWaveformDirection : public LASoperation
{
public:
  inline const CHAR* name() const { return "flip_waveform_direction"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { point->wavepacket.flipDirection(); };
};

class LASoperationCopyUserDataIntoPointSource : public LASoperation
{
public:
  inline const CHAR* name() const { return "copy_user_data_into_point_source"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
  inline void transform(LASpoint* point) { point->point_source_ID = point->user_data; };
};

class LASoperationBinZintoPointSource : public LASoperation
{
public:
  inline const CHAR* name() const { return "bin_Z_into_point_source"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %d", name(), bin_size); };
  inline void transform(LASpoint* point) { point->point_source_ID = U16_CLAMP(point->get_Z()/bin_size); };
  LASoperationBinZintoPointSource(I32 bin_size=1) { this->bin_size = bin_size; };
private:
  I32 bin_size;
};

class LASoperationBinAbsScanAngleIntoPointSource : public LASoperation
{
public:
  inline const CHAR* name() const { return "bin_abs_scan_angle_into_point_source"; };
  inline int get_command(CHAR* string) const { return sprintf(string, "-%s %g", name(), bin_size); };
  inline void transform(LASpoint* point) { point->point_source_ID = U16_CLAMP(point->get_abs_scan_angle()/bin_size); };
  LASoperationBinAbsScanAngleIntoPointSource(F32 bin_size=1.0f) { this->bin_size = bin_size; };
private:
  F32 bin_size;
};

void LAStransform::clean()
{
  U32 i;
  for (i = 0; i < num_operations; i++)
  {
    delete operations[i];
  }
  if (operations) delete [] operations;
  change_coordinates = FALSE;
  alloc_operations = 0;
  num_operations = 0;
  operations = 0;
  if (filter)
  {
    delete filter;
    filter = 0;
  }
}

void LAStransform::usage() const
{
  fprintf(stderr,"Transform coordinates.\n");
  fprintf(stderr,"  -translate_x -2.5\n");
  fprintf(stderr,"  -scale_z 0.3048\n");
  fprintf(stderr,"  -rotate_xy 15.0 620000 4100000 (angle + origin)\n");
  fprintf(stderr,"  -translate_xyz 0.5 0.5 0\n");
  fprintf(stderr,"  -translate_then_scale_y -0.5 1.001\n");
  fprintf(stderr,"  -switch_x_y -switch_x_z -switch_y_z\n");
  fprintf(stderr,"  -clamp_z_below 70.5\n");
  fprintf(stderr,"  -clamp_z 70.5 72.5\n");
  fprintf(stderr,"  -copy_attribute_into_z 0\n");
  fprintf(stderr,"Transform raw xyz integers.\n");
  fprintf(stderr,"  -translate_raw_z 20\n");
  fprintf(stderr,"  -translate_raw_xyz 1 1 0\n");
  fprintf(stderr,"  -translate_raw_xy_at_random 2 2\n");
  fprintf(stderr,"  -clamp_raw_z 500 800\n");
  fprintf(stderr,"Transform intensity.\n");
  fprintf(stderr,"  -set_intensity 0\n");
  fprintf(stderr,"  -scale_intensity 2.5\n");
  fprintf(stderr,"  -translate_intensity 50\n");
  fprintf(stderr,"  -translate_then_scale_intensity 0.5 3.1\n");
  fprintf(stderr,"  -clamp_intensity 0 255\n");
  fprintf(stderr,"  -clamp_intensity_above 255\n");
  fprintf(stderr,"Transform scan_angle.\n");
  fprintf(stderr,"  -scale_scan_angle 1.944445\n");
  fprintf(stderr,"  -translate_scan_angle -5\n");
  fprintf(stderr,"  -translate_then_scale_scan_angle -0.5 2.1\n");
  fprintf(stderr,"Change the return number or return count of points.\n");
  fprintf(stderr,"  -repair_zero_returns\n");
  fprintf(stderr,"  -set_return_number 1\n");
  fprintf(stderr,"  -set_extended_return_number 10\n");
  fprintf(stderr,"  -change_return_number_from_to 2 1\n");
  fprintf(stderr,"  -set_number_of_returns 2\n");
  fprintf(stderr,"  -set_number_of_returns 15\n");
  fprintf(stderr,"  -change_number_of_returns_from_to 0 2\n");
  fprintf(stderr,"Modify the classification.\n");
  fprintf(stderr,"  -set_classification 2\n");
  fprintf(stderr,"  -set_extended_classification 0\n");
  fprintf(stderr,"  -change_classification_from_to 2 4\n");
  fprintf(stderr,"  -classify_z_below_as -5.0 7\n");
  fprintf(stderr,"  -classify_z_above_as 70.0 7\n");
  fprintf(stderr,"  -classify_z_between_as 2.0 5.0 4\n");
  fprintf(stderr,"  -classify_intensity_above_as 200 9\n");
  fprintf(stderr,"  -classify_intensity_below_as 30 11 \n");
  fprintf(stderr,"  -change_extended_classification_from_to 6 46\n");
  fprintf(stderr,"  -move_ancient_to_extended_classification\n");
  fprintf(stderr,"Change the flags.\n");
  fprintf(stderr,"  -set_withheld_flag 0\n");
  fprintf(stderr,"  -set_synthetic_flag 1\n");
  fprintf(stderr,"  -set_keypoint_flag 0\n");
  fprintf(stderr,"  -set_extended_overlap_flag 1\n");
  fprintf(stderr,"Modify the extended scanner channel.\n");
  fprintf(stderr,"  -set_extended_scanner_channel 2\n");
  fprintf(stderr,"Modify the user data.\n");
  fprintf(stderr,"  -set_user_data 0\n");
  fprintf(stderr,"  -change_user_data_from_to 23 26\n");
  fprintf(stderr,"Modify the point source ID.\n");
  fprintf(stderr,"  -set_point_source 500\n");
  fprintf(stderr,"  -change_point_source_from_to 1023 1024\n");
  fprintf(stderr,"  -copy_user_data_into_point_source\n");
  fprintf(stderr,"  -bin_Z_into_point_source 200\n");
  fprintf(stderr,"  -bin_abs_scan_angle_into_point_source 2\n");
  fprintf(stderr,"Transform gps_time.\n");
  fprintf(stderr,"  -set_gps_time 113556962.005715\n");
  fprintf(stderr,"  -translate_gps_time 40.50\n");
  fprintf(stderr,"  -adjusted_to_week\n");
  fprintf(stderr,"  -week_to_adjusted 1671\n");
  fprintf(stderr,"Transform RGB/NIR colors.\n");
  fprintf(stderr,"  -set_RGB 255 0 127\n");
  fprintf(stderr,"  -set_RGB_of_class 9 0 0 255\n");
  fprintf(stderr,"  -scale_RGB 2 4 2\n");
  fprintf(stderr,"  -scale_RGB_down (by 256)\n");
  fprintf(stderr,"  -scale_RGB_up (by 256)\n");
  fprintf(stderr,"  -switch_R_G -switch_R_B -switch_B_G\n");
  fprintf(stderr,"  -copy_R_into_NIR -copy_G_into_NIR -copy_B_into_NIR\n");
}

bool LAStransform::parse(int argc, char* argv[])
{
  int i;

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
    {
      continue;
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      usage();
      return TRUE;
    }
    else if (strncmp(argv[i],"-translate_", 11) == 0)
    {
      if (strcmp(argv[i],"-translate_x") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationTranslateX((F64)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-translate_y") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationTranslateY((F64)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-translate_z") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationTranslateZ((F64)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-translate_xyz") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: offset_x offset_y offset_z\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationTranslateXYZ((F64)atof(argv[i+1]), (F64)atof(argv[i+2]), (F64)atof(argv[i+3])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
      }
      else if (strcmp(argv[i],"-translate_then_scale_x") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: offset scale\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationTranslateThenScaleX((F64)atof(argv[i+1]), (F64)atof(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-translate_then_scale_y") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: offset scale\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationTranslateThenScaleY((F64)atof(argv[i+1]), (F64)atof(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-translate_then_scale_z") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: offset scale\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationTranslateThenScaleZ((F64)atof(argv[i+1]), (F64)atof(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strncmp(argv[i],"-translate_raw_", 14) == 0)
      {
        if (strcmp(argv[i],"-translate_raw_x") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: raw_offset\n", argv[i]);
            return FALSE;
          }
          change_coordinates = TRUE;
          add_operation(new LASoperationTranslateRawX((I32)atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
        }
        else if (strcmp(argv[i],"-translate_raw_y") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: raw_offset\n", argv[i]);
            return FALSE;
          }
          change_coordinates = TRUE;
          add_operation(new LASoperationTranslateRawY((I32)atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
        }
        else if (strcmp(argv[i],"-translate_raw_z") == 0)
        {
          if ((i+1) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 1 argument: raw_offset\n", argv[i]);
            return FALSE;
          }
          change_coordinates = TRUE;
          add_operation(new LASoperationTranslateRawZ((I32)atoi(argv[i+1])));
          *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
        }
        else if (strcmp(argv[i],"-translate_raw_xyz") == 0)
        {
          if ((i+3) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: raw_offset_x raw_offset_y raw_offset_z\n", argv[i]);
            return FALSE;
          }
          change_coordinates = TRUE;
          add_operation(new LASoperationTranslateRawXYZ((I32)atoi(argv[i+1]), (I32)atoi(argv[i+2]), (I32)atoi(argv[i+3])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
        }
        else if (strcmp(argv[i],"-translate_raw_xy_at_random") == 0)
        {
          if ((i+2) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 2 arguments: max_raw_offset_x max_raw_offset_y\n", argv[i]);
            return FALSE;
          }
          change_coordinates = TRUE;
          add_operation(new LASoperationTranslateRawXYatRandom((I32)atoi(argv[i+1]), (I32)atoi(argv[i+2])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
        }
      }
      else if (strcmp(argv[i],"-translate_intensity") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationTranslateIntensity((F32)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-translate_then_scale_intensity") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: offset scale\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationTranslateThenScaleIntensity((F32)atof(argv[i+1]), (F32)atof(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-translate_scan_angle") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationTranslateScanAngle((F32)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-translate_then_scale_scan_angle") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: offset scale\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationTranslateThenScaleScanAngle((F32)atof(argv[i+1]), (F32)atof(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-translate_gps_time") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: offset\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationTranslateGpsTime(atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
    }
    else if (strncmp(argv[i],"-rotate_", 8) == 0)
    {
      if (strcmp(argv[i],"-rotate_xy") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: angle, x, y\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationRotateXY((F64)atof(argv[i+1]), (F64)atof(argv[i+2]), (F64)atof(argv[i+3])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
      }
      else if (strcmp(argv[i],"-rotate_xz") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: angle, x, y\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationRotateXZ((F64)atof(argv[i+1]), (F64)atof(argv[i+2]), (F64)atof(argv[i+3])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
      }
    }
    else if (strncmp(argv[i],"-clamp_", 7) == 0)
    {
      if (strcmp(argv[i],"-clamp_z") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: below above\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationClampZ(atof(argv[i+1]), atof(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-clamp_z_below") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: below\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationClampZbelow(atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-clamp_z_above") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: above\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationClampZabove(atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-clamp_intensity") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: below above\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationClampIntensity(U16_CLAMP(atoi(argv[i+1])), U16_CLAMP(atoi(argv[i+2]))));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-clamp_intensity_below") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: below\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationClampIntensityBelow(U16_CLAMP(atoi(argv[i+1]))));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-clamp_intensity_above") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: above\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationClampIntensityAbove(U16_CLAMP(atoi(argv[i+1]))));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-clamp_raw_z") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: below above\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationClampRawZ((I32)atoi(argv[i+1]), (I32)atoi(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
    }
    else if (strncmp(argv[i],"-copy_", 6) == 0)
    {
      if (strcmp(argv[i],"-copy_attribute_into_z") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: index of attribute\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationCopyAttributeIntoZ(atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-copy_user_data_into_point_source") == 0)
      {
        add_operation(new LASoperationCopyUserDataIntoPointSource());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-copy_R_into_NIR") == 0)
      {
        add_operation(new LASoperationCopyRintoNIR());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-copy_G_into_NIR") == 0)
      {
        add_operation(new LASoperationCopyGintoNIR());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-copy_B_into_NIR") == 0)
      {
        add_operation(new LASoperationCopyBintoNIR());
        *argv[i]='\0'; 
      }
    }
    else if (strncmp(argv[i],"-set_", 5) == 0)
    {
      if (strncmp(argv[i],"-set_classification", 19) == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: classification\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetClassification(U8_CLAMP(atoi(argv[i+1]))));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strncmp(argv[i],"-set_extended_classification", 28) == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: classification\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetExtendedClassification(U8_CLAMP(atoi(argv[i+1]))));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_intensity") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetIntensity(U16_CLAMP(atof(argv[i+1]))));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_withheld_flag") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' need 1 argument: value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetWithheldFlag((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_synthetic_flag") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' need 1 argument: value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetSyntheticFlag((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_keypoint_flag") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' need 1 argument: value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetKeypointFlag((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_extended_overlap_flag") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' need 1 argument: value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetExtendedOverlapFlag((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_extended_scanner_channel") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' need 1 argument: value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetExtendedScannerChannel((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_user_data") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' need 1 argument: value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetUserData((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strncmp(argv[i],"-set_point_source", 17) == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' need 1 argument: psid\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetPointSource((U16)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_return_number") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: return_number\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetReturnNumber((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_extended_return_number") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: extended_return_number\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetExtendedReturnNumber((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_number_of_returns") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: number_of_returns\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetNumberOfReturns((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_extended_number_of_returns") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: extended_number_of_returns\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetExtendedNumberOfReturns((U8)atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-set_gps_time") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationSetGpsTime(atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strncmp(argv[i],"-set_RGB", 8) == 0)
      {
        if (strcmp(argv[i],"-set_RGB") == 0)
        {
          if ((i+3) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 3 arguments: R G B\n", argv[i]);
            return FALSE;
          }
          add_operation(new LASoperationSetRGB((U16)atoi(argv[i+1]), (U16)atoi(argv[i+2]), (U16)atoi(argv[i+3])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
        }
        else if (strcmp(argv[i],"-set_RGB_of_class") == 0)
        {
          if ((i+4) >= argc)
          {
            fprintf(stderr,"ERROR: '%s' needs 4 arguments: class R G B\n", argv[i]);
            return FALSE;
          }
          I32 c = atoi(argv[i+1]);
          if ((c < 0) || (c > 255))
          {
            fprintf(stderr,"ERROR: '%s' needs class between 0 and 255 but got %d\n", argv[i], c);
            return FALSE;
          }
          add_operation(new LASoperationSetRGBofClass((U8)c, (U16)atoi(argv[i+2]), (U16)atoi(argv[i+3]), (U16)atoi(argv[i+4])));
          *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; *argv[i+4]='\0'; i+=4; 
        }
      }
    }
    else if (strncmp(argv[i],"-change_",8) == 0)
    {
      if (strcmp(argv[i],"-change_classification_from_to") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: from_value to_value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationChangeClassificationFromTo(U8_CLAMP(atoi(argv[i+1])), U8_CLAMP(atoi(argv[i+2]))));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-change_extended_classification_from_to") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: from_value to_value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationChangeExtendedClassificationFromTo(U8_CLAMP(atoi(argv[i+1])), U8_CLAMP(atoi(argv[i+2]))));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-change_user_data_from_to") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: from_value to_value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationChangeUserDataFromTo((U8)atoi(argv[i+1]), (U8)atoi(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-change_point_source_from_to") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: from_value to_value\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationChangePointSourceFromTo((U16)atoi(argv[i+1]), (U16)atoi(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-change_return_number_from_to") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: from_return_number to_return_number\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationChangeReturnNumberFromTo((U8)atoi(argv[i+1]), (U8)atoi(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-change_number_of_returns_from_to") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: from_number_of_returns to_number_of_returns\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationChangeNumberOfReturnsFromTo((U8)atoi(argv[i+1]), (U8)atoi(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
    }
    else if (strncmp(argv[i],"-classify_", 10) == 0)
    {
      if (strcmp(argv[i],"-classify_z_below_as") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: z_value classification_code\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationClassifyZbelowAs(atof(argv[i+1]), U8_CLAMP(atoi(argv[i+2]))));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-classify_z_above_as") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: z_value classification_code\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationClassifyZaboveAs(atof(argv[i+1]), U8_CLAMP(atoi(argv[i+2]))));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-classify_z_between_as") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: z_min z_max classification_code\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationClassifyZbetweenAs(atof(argv[i+1]), atof(argv[i+2]), U8_CLAMP(atoi(argv[i+3]))));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
      }
      else if (strcmp(argv[i],"-classify_intensity_below_as") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: intensity_value classification_code\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationClassifyIntensityBelowAs(U16_CLAMP(atoi(argv[i+1])), U8_CLAMP(atoi(argv[i+2]))));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
      else if (strcmp(argv[i],"-classify_intensity_above_as") == 0)
      {
        if ((i+2) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 2 arguments: intensity_value classification_code\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationClassifyIntensityAboveAs(U16_CLAMP(atoi(argv[i+1])), (U8)atoi(argv[i+2])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; i+=2; 
      }
    }
    else if (strncmp(argv[i],"-scale_", 7) == 0)
    {
      if (strcmp(argv[i],"-scale_x") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: scale\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationScaleX((F64)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-scale_y") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: scale\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationScaleY((F64)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-scale_z") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: scale\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationScaleZ((F64)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-scale_xyz") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: scale_x scale_y scale_z\n", argv[i]);
          return FALSE;
        }
        change_coordinates = TRUE;
        add_operation(new LASoperationScaleXYZ((F64)atof(argv[i+1]), (F64)atof(argv[i+2]), (F64)atof(argv[i+3])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
      }
      else if (strcmp(argv[i],"-scale_intensity") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: scale\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationScaleIntensity((F32)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-scale_scan_angle") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: scale\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationScaleScanAngle((F32)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-scale_RGB") == 0 || strcmp(argv[i],"-scale_rgb") == 0)
      {
        if ((i+3) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 3 arguments: scale_R scale_G scale_B\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationScaleRGB((F32)atof(argv[i+1]), (F32)atof(argv[i+2]), (F32)atof(argv[i+3])));
        *argv[i]='\0'; *argv[i+1]='\0'; *argv[i+2]='\0'; *argv[i+3]='\0'; i+=3; 
      }
      else if (strcmp(argv[i],"-scale_RGB_down") == 0 || strcmp(argv[i],"-scale_rgb_down") == 0)
      {
        add_operation(new LASoperationScaleRGBdown());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-scale_RGB_up") == 0 || strcmp(argv[i],"-scale_rgb_up") == 0)
      {
        add_operation(new LASoperationScaleRGBup());
        *argv[i]='\0'; 
      }
    }
    else if (strncmp(argv[i],"-switch_", 8) == 0)
    {
      if (strcmp(argv[i],"-switch_x_y") == 0)
      {
        add_operation(new LASoperationSwitchXY());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-switch_x_z") == 0)
      {
        add_operation(new LASoperationSwitchXZ());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-switch_y_z") == 0)
      {
        add_operation(new LASoperationSwitchYZ());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-switch_R_G") == 0)
      {
        add_operation(new LASoperationSwitchRG());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-switch_R_B") == 0)
      {
        add_operation(new LASoperationSwitchRB());
        *argv[i]='\0'; 
      }
      else if (strcmp(argv[i],"-switch_G_B") == 0)
      {
        add_operation(new LASoperationSwitchGB());
        *argv[i]='\0'; 
      }
    }
    else if (strncmp(argv[i],"-bin_", 5) == 0)
    {
      if (strcmp(argv[i],"-bin_Z_into_point_source") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: bin_size\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationBinZintoPointSource(atoi(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
      else if (strcmp(argv[i],"-bin_abs_scan_angle_into_point_source") == 0)
      {
        if ((i+1) >= argc)
        {
          fprintf(stderr,"ERROR: '%s' needs 1 argument: bin_size\n", argv[i]);
          return FALSE;
        }
        add_operation(new LASoperationBinAbsScanAngleIntoPointSource((F32)atof(argv[i+1])));
        *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
      }
    }
    else if (strcmp(argv[i],"-move_ancient_to_extended_classification") == 0)
    {
      add_operation(new LASoperationMoveAncientToExtendedClassification());
      *argv[i]='\0';
    }
    else if (strcmp(argv[i],"-flip_waveform_direction") == 0)
    {
      add_operation(new LASoperationFlipWaveformDirection());
      *argv[i]='\0'; 
    }
    else if (strcmp(argv[i],"-repair_zero_returns") == 0)
    {
      add_operation(new LASoperationRepairZeroReturns());
      *argv[i]='\0'; 
    }
    else if (strcmp(argv[i],"-adjusted_to_week") == 0)
    {
      add_operation(new LASoperationConvertAdjustedGpsToWeek());
      *argv[i]='\0'; 
    }
    else if (strcmp(argv[i],"-week_to_adjusted") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs 1 argument: week\n", argv[i]);
        return FALSE;
      }
      add_operation(new LASoperationConvertWeekToAdjustedGps(atoi(argv[i+1])));
      *argv[i]='\0'; *argv[i+1]='\0'; i+=1; 
    }
    else if (strcmp(argv[i],"-filtered_transform") == 0)
    {
      is_filtered = TRUE;
      *argv[i]='\0'; 
    }
  }
  return TRUE;
}

bool LAStransform::parse(CHAR* string)
{
  int p = 0;
  int argc = 1;
  char* argv[64];
  int len = strlen(string);

  while (p < len)
  {
    while ((p < len) && (string[p] == ' ')) p++;
    if (p < len)
    {
      argv[argc] = string + p;
      argc++;
      while ((p < len) && (string[p] != ' ')) p++;
      string[p] = '\0';
      p++;
    }
  }

  return parse(argc, argv);
}

I32 LAStransform::unparse(CHAR* string) const
{
  U32 i;
  I32 n = 0;
  if (filter)
  {
    n += filter->unparse(&string[n]);
    n += sprintf(&string[n], "-filtered_transform ");
  }
  for (i = 0; i < num_operations; i++)
  {
    n += operations[i]->get_command(&string[n]);
  }
  return n;
}

void LAStransform::transform(LASpoint* point)
{
  U32 i;
  if (filter)
  {
    if (filter->filter(point))
    {
      return;
    }
  }
  for (i = 0; i < num_operations; i++) operations[i]->transform(point);
}

void LAStransform::reset()
{
  U32 i;
  for (i = 0; i < num_operations; i++)
  {
    operations[i]->reset();
  }
}

LAStransform::LAStransform()
{
  change_coordinates = FALSE;
  alloc_operations = 0;
  num_operations = 0;
  operations = 0;
  is_filtered = FALSE;
  filter = 0;
}

LAStransform::~LAStransform()
{
  if (operations) clean();
}

void LAStransform::add_operation(LASoperation* transform_operation)
{
  if (num_operations == alloc_operations)
  {
    U32 i;
    alloc_operations += 16;
    LASoperation** temp_operations = new LASoperation*[alloc_operations];
    if (operations)
    {
      for (i = 0; i < num_operations; i++)
      {
        temp_operations[i] = operations[i];
      }
      delete [] operations;
    }
    operations = temp_operations;
  }
  operations[num_operations] = transform_operation;
  num_operations++;
}

void LAStransform::setFilter(LASfilter* filter)
{
  if (this->filter) delete this->filter;
  this->filter = filter;
  if (filter == 0) is_filtered = FALSE;
}

void LAStransform::setPointSource(U16 value)
{
  if (operations)
  {
    U32 i;
    for (i = 0; i < num_operations; i++)
    {
      if (strcmp(operations[i]->name(), "set_point_source"))
      {
        delete operations[i];
        operations[i] = new LASoperationSetPointSource(value);
        return;
      }
    }
  }
  add_operation(new LASoperationSetPointSource(value));
}

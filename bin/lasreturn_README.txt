****************************************************************

  lasreturn:

  Reports geometric return statistics for multi-return pulses and
  repairs the 'number of returns' field based on GPS times. Note 
  that input files need (currently) to be sorted based on their
  GPS time stamp.

  For updates check the website or join the LAStools mailing list.

  http://lastools.org/
  http://groups.google.com/group/LAStools/
  http://twitter.com/LAStools/
  http://facebook.com/LAStools/
  http://linkedin.com/groups?gid=4408378

  Martin @LAStools

****************************************************************

example usage:

>> lasreturn -i lidar.laz -repair_number_of_returns -odix _repaired -olaz

the 'number of returns' field of every point is set to the highest
return number that is found for each set of returns with the same
unique GPS time stamp. assumes sorted input (use lassort -gps_time).

>> lasreturn -i lidar.laz -histo return_distance 0.1

computes the distances between all subsequent returns from the same
pulse and prints a histogram with bin size 0.1 meter. assumes sorted
input (use lassort -gps_time).


>> lasreturn -i lidar.laz -histo return_distance 0.1 0.0 4.99

same as before but limits the histogram to the range rfrom 0 to 5

---------------

if you find bugs let me (martin.isenburg@rapidlasso.com) know.
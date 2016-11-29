function plot_geo_pixel(record, value_field)
% PLOT_GEO_PIXEL Plot the geo pixels in the geo pixel record.
%
%    PLOT_GEO_PIXEL(record, value_field) plots the geo pixels in the
%    BEAT-II record.
%

longitude = record.corner_longitude';
latitude = record.corner_latitude';
value = getfield(record, value_field)';

% This is a (not very clean) algorithm to cut swaths in half when they cross
% the -180 longitude meridian.
% The problem with this method is that it doesn't set the latitudes near the
% -180 meridian properly.
longitude_diff = max(longitude) - min(longitude);
index = find(longitude_diff > 180);
num_pixels = length(index);
for i=1:num_pixels
  extra_longitude = longitude(:,index(i));
  for j=1:4
    if longitude(j,index(i)) < 0
      longitude(j,index(i)) = 180;
    end
    if extra_longitude(j) > 0
      extra_longitude(j) = -180;
    end
  end
  longitude = [longitude extra_longitude];
  latitude = [latitude latitude(:,index(i))];
  value = [value value(index(i))];
end

fill(longitude, latitude, value);
shading flat;
xlabel('longitude [ deg ]');
ylabel('latitude [ deg ]');

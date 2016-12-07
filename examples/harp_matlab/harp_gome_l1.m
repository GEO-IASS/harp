function harp_gome_l1(filename)
% SCI_OL__2P Show SCIAMACHY Level-2 Offline data with BEAT-II.
%
%    SCI_OL__2P(directory) shows O3 vcd data from one or more
%    'SCI_OL__2P' product files with BEAT-II.
%

% netherlands = [  -5   15  50  60];
% europe      = [ -20   40  40  70];
% world       = [-180  180 -90  90];
% custom      = [  28   67  69  79];

% frame = world;
% 
% % find all files in the specified directory.
% % files = dir(strcat(directory,'/SCI_OL__2P*.N1'));
% 
% num_files = length(files);
% 
% if num_files == 0 
%   disp('WARNING: no files found!');
% end
% 
% for i=1:num_files
%   files(i).name = strcat(directory,'/',files(i).name);
% end
% filenames = strvcat(files.name);
% filter = sprintf('data=nadir_uv_0,convert_to_DU,latitude_min=%d,latitude_max=%d,longitude_min=%d,longitude_max=%d,scan_direction=forward mixed', frame(3), frame(4), frame(1), frame(2));
% data = beatl2_ingest(filenames,filter);
% 
% figure;
% plot_geo_pixel(data, 'o3_column');
% title('Sciamachy Level-2 Ozone');
% axis(frame);
% caxis([150 500]);

data = harp_ingest(filename);
%longitude = data.latitude;
%longitude_bounds = data.longitude_bounds;
%fraction = data.cloud_fraction;
display(data);
%display(longitude);
%display(fraction);
%display(longitude_bounds(:,2)');


%longitude = data.corner_longitude';
%latitude = data.corner_latitude';
longitude = data.longitude_bounds';
latitude = data.latitude_bounds';
value = getfield(data, 'O3_column_number_density')';

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





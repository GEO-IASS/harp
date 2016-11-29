function gome_l4(directory)
% GOME_L4 Show GOME Level-4 data with BEAT-II.
%
%    GOME_L4(directory) shows assimilated O3 for one or more 'GOME_L4'
%    product files with BEAT-II.
%

files = dir(strcat(directory,'/ga*.gom'));
num_files = length(files);

if num_files == 0
  disp('WARNING: no files found!');
end

for i=1:num_files
  files(i).name = strcat(directory,'/',files(i).name);
end

disp('Press <ctrl>-c to abort this demo');
for i=1:num_files
  data = beatl2_ingest(files(i).name);
  pcolor(data.grid_longitude, data.grid_latitude, shiftdim(data.o3_column_grid,1));
  shading flat;
  title(sprintf('GOME FAST DELIVERY %s', beat_time_to_string(data.time(1))));
  xlabel('longitude [deg]');
  ylabel('latitude [deg]');
  caxis([0 600]);
  colorbar('horz');
  pause(0.01);
end

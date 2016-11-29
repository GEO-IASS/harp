function gosat_fts_l2_co2(directory)
% GOSAT_FTS_L2_CO2 Show GOSAT FTS Level-2 CO2 data with BEAT-II.
%
%    GOSAT_FTS_L2_CO2(directory) shows CO2 vcd data from one or more
%    GOSAT FTS L2 'C01S' product files with BEAT-II.
%

netherlands = [  -5   15  50  60];
europe      = [ -20   40  40  70];
world       = [-180  180 -90  90];
custom      = [  28   67  69  79];

frame = world;

% find all files in the specified directory.
files = dir(strcat(directory,'/GOSATTFTS*C01S*'));

num_files = length(files);

if num_files == 0 
  disp('WARNING: no files found!');
end

for i=1:num_files
  files(i).name = strcat(directory,'/',files(i).name);
end
filenames = strvcat(files.name);
filter = sprintf('latitude_min=%d,latitude_max=%d,longitude_min=%d,longitude_max=%d', frame(3), frame(4), frame(1), frame(2));
data = beatl2_ingest(filenames,filter);

figure;
plot_geo_pixel(data, 'co2_column');
title('GOSAT FTS Level-2 CO2');
axis(frame);
colorbar('horz');
